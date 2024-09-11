#include "ShijimaWidget.hpp"
#include <QWidget>
#include <QPainter>
#include <QFile>
#include <QDir>
#include <QScreen>
#include <QMouseEvent>
#include <QMenu>
#include <QGuiApplication>
#include <QTextStream>
#include <shijima/shijima.hpp>
#include "AssetLoader.hpp"
#include "ShijimaContextMenu.hpp"

using namespace shijima;

ShijimaWidget::ShijimaWidget(std::string const& mascotName,
    std::string const& imgRoot,
    std::unique_ptr<shijima::mascot::manager> mascot,
    QWidget *parent) : QWidget(parent)
{
    m_mascotName = mascotName;
    m_windowHeight = 128;
    m_windowWidth = 128;
    m_imgRoot = imgRoot;
    m_mascot = std::move(mascot);

    QString qImgRoot = QString::fromStdString(imgRoot);
    m_sounds.searchPaths.push_back(qImgRoot);
    QDir dir { qImgRoot };
    if (dir.exists() && dir.cdUp() && dir.cdUp() && dir.cd("sound")) {
        m_sounds.searchPaths.push_back(dir.path());
    }
    dir = { qImgRoot };
    if (dir.exists() && dir.cdUp() && dir.cd("sound")) {
        m_sounds.searchPaths.push_back(dir.path());
    }
    dir = { qImgRoot };
    if (dir.exists() && dir.cd("sound")) {
        m_sounds.searchPaths.push_back(dir.path());
    }
    qInfo() << m_sounds.searchPaths;
    
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_MacShowFocusRect, false);
    setFixedSize(m_windowWidth, m_windowHeight);
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint
        | Qt::WindowDoesNotAcceptFocus | Qt::NoDropShadowWindowHint);
}

Asset const& ShijimaWidget::getActiveAsset() {
    auto &frame = m_mascot->state->active_frame;
    auto imagePath = QDir::cleanPath(QString::fromStdString(m_imgRoot)
        + QDir::separator() + QString(frame.name.c_str()));
    return AssetLoader::defaultLoader()->loadAsset(imagePath);
}

void ShijimaWidget::paintEvent(QPaintEvent *event) {
    if (!m_visible) {
        return;
    }
    QPainter painter(this);
    auto &asset = getActiveAsset();
    auto &image = asset.image(m_mascot->state->looking_right);
    painter.drawImage(m_drawOrigin, image);
}

bool ShijimaWidget::updateOffsets() {
    bool needsRepaint = false;
    auto &frame = m_mascot->state->active_frame;
    auto &asset = getActiveAsset();
    
    // Does the image go outside of the minimum boundary? If so,
    // extend the window boundary
    int windowWidth = asset.originalSize().width();
    int windowHeight = asset.originalSize().height();
    int screenWidth = m_mascot->state->env->screen.width();
    int screenHeight = m_mascot->state->env->screen.height();
    if (windowWidth != m_windowWidth) {
        m_windowWidth = windowWidth;
        setFixedWidth(m_windowWidth);
        needsRepaint = true;
    }
    if (windowHeight != m_windowHeight) {
        m_windowHeight = windowHeight;
        setFixedHeight(m_windowHeight);
        needsRepaint = true;
    }

    // Determine the frame anchor within the window
    if (m_mascot->state->looking_right) {
        m_anchorInWindow = {
            asset.originalSize().width() - (int)frame.anchor.x,
            (int)frame.anchor.y };
    }
    else {
        m_anchorInWindow = { (int)frame.anchor.x, (int)frame.anchor.y };
    }

    // Detemine draw offsets and window positions
    QPoint drawOffset;
    m_visible = true;
    int winX = (int)m_mascot->state->anchor.x - m_anchorInWindow.x();
    int winY = (int)m_mascot->state->anchor.y - m_anchorInWindow.y();
    if (winX < 0) {
        drawOffset.setX(winX);
        winX = 0;
    }
    else if (winX + windowWidth > screenWidth) {
        drawOffset.setX(winX - screenWidth + windowWidth);
        winX = screenWidth - windowWidth;
    }
    if (winY < 0) {
        drawOffset.setY(winY);
        winY = 0;
    }
    else if (winY + windowHeight > screenHeight) {
        drawOffset.setY(winY - screenHeight + windowHeight);
        winY = screenHeight - windowHeight;
    }
    QPoint drawOrigin = drawOffset;
    if (m_mascot->state->looking_right) {
        drawOrigin += QPoint {
            asset.originalSize().width() - asset.offset().topRight().x(),
            asset.offset().topLeft().y() };
    }
    else {
        drawOrigin += asset.offset().topLeft();
    }
    if (drawOrigin != m_drawOrigin) {
        needsRepaint = true;
        m_drawOrigin = drawOrigin;
    }
    move(winX, winY);

    return needsRepaint;
}

void ShijimaWidget::tick() {
    if (m_markedForDeletion) {
        close();
        return;
    }
    if (paused()) {
        return;
    }

    // Tick
    auto prev_frame = m_mascot->state->active_frame;
    m_mascot->tick();
    auto &new_frame = m_mascot->state->active_frame;
    auto &new_sound = m_mascot->state->active_sound;
    bool forceRepaint = prev_frame.name != new_frame.name;
    bool offsetsChanged = updateOffsets();
    if (offsetsChanged || forceRepaint) {
        repaint();
        update();
    }
    if (m_mascot->state->active_sound_changed) {
        m_sounds.stop();
        if (!new_sound.empty()) {
            m_sounds.play(QString::fromStdString(new_sound));
        }
    }
    else if (!m_sounds.playing()) {
        m_mascot->state->active_sound.clear();
    }
}

void ShijimaWidget::contextMenuClosed(QCloseEvent *event) {
    m_contextMenuVisible = false;
}

void ShijimaWidget::showContextMenu(QPoint const& pos) {
    m_contextMenuVisible = true;
    ShijimaContextMenu *menu = new ShijimaContextMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->popup(pos);
}

void ShijimaWidget::mousePressEvent(QMouseEvent *event) {
    // Ignore presses to transparent areas
    {
        if (!m_visible) {
            event->ignore();
            return;
        }
        auto &asset = getActiveAsset();
        auto image = asset.image(m_mascot->state->looking_right);
        auto pos = event->pos();
        auto imagePos = pos - m_drawOrigin;
        if (imagePos.x() < 0 || imagePos.y() < 0 ||
            imagePos.x() > image.width() || imagePos.y() > image.height())
        {
            event->ignore();
            return;
        }
        auto color = image.pixelColor(imagePos);
        if (color.alpha() == 0) {
            event->ignore();
            return;
        }
    }

    if (event->button() == Qt::MouseButton::LeftButton) {
        m_mascot->state->dragging = true;
    }
    else if (event->button() == Qt::MouseButton::RightButton) {
        auto pos = event->pos();
        showContextMenu(mapToGlobal(pos));
    }
}

void ShijimaWidget::closeAction() {
    close();
}

void ShijimaWidget::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::MouseButton::LeftButton) {
        m_mascot->state->dragging = false;
    }
}