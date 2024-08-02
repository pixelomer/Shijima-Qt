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

#define kShijimaWidth 128
#define kShijimaHeight kShijimaWidth

using namespace shijima;

ShijimaWidget::ShijimaWidget(std::string const& mascotName,
    std::string const& imgRoot,
    std::unique_ptr<shijima::mascot::manager> mascot,
    QWidget *parent) : QWidget(parent)
{
    m_mascotName = mascotName;
    m_imgRoot = imgRoot;
    m_mascot = std::move(mascot);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_MacShowFocusRect, false);
    setFixedSize(kShijimaWidth, kShijimaHeight);
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint
        | Qt::WindowDoesNotAcceptFocus | Qt::NoDropShadowWindowHint);
}

ShijimaWidget::ActiveImage const& ShijimaWidget::getActiveImage() {
    if (m_activeImage.available) {
        return m_activeImage;
    }
    auto &frame = m_mascot->state->active_frame;
    auto imagePath = QDir::cleanPath(QString::fromStdString(m_imgRoot)
        + QDir::separator() + QString(frame.name.c_str()));
    QImage const& image = AssetLoader::defaultLoader()
        ->loadImage(imagePath, m_mascot->state->looking_right);
    QPoint dest = { 0, 0 };
    QRect source = image.rect();
    if (m_offsetX < 0) {
        // Mascot X is less than 0
        source.setX(-m_offsetX);
    }
    else if (m_offsetX > 0) {
        // Mascot X is greater than screen width
        source.setWidth(source.width() - m_offsetX);
        dest.setX(m_offsetX);
    }
    if (m_offsetY < 0) {
        // Mascot Y is less than 0
        source.setY(-m_offsetY);
    }
    else if (m_offsetY > 0) {
        // Mascot Y is greater than screen height
        source.setHeight(source.height() - m_offsetY);
    }

    return m_activeImage = { &image, dest, source };
}

void ShijimaWidget::paintEvent(QPaintEvent *event) {
    if (!m_visible) {
        return;
    }
    QPainter painter(this);
    auto &active = getActiveImage();
    painter.drawImage(active.drawOrigin, *active.image, active.sourceRect);
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

    // Update draw offsets depending on the new position
    //raise();
    auto envWidth = env()->screen.width();
    //auto envHeight = m_env->screen.height();
    auto &frame = m_mascot->state->active_frame;
    bool needsRepaint = prev_frame.name != frame.name;
    int winX = (int)m_mascot->state->anchor.x;
    if (m_mascot->state->looking_right) {
        winX -= kShijimaWidth - (int)frame.anchor.x;
    }
    else {
        winX -= (int)frame.anchor.x;
    }
    int winY = (int)m_mascot->state->anchor.y - (int)frame.anchor.y;
    int offX=0, offY=0;
    if (winX < 0) {
        offX = winX;
        winX = 0;
        needsRepaint = true;
    }
    else if ((winX + kShijimaWidth) > envWidth) {
        offX = winX + kShijimaWidth - envWidth;
        winX = envWidth - kShijimaWidth;
        needsRepaint = true;
    }
    if (winY < 0) {
        offY = winY;
        winY = 0;
    }
    if (offX != m_offsetX || offY != m_offsetY) {
        needsRepaint = true;
    }
    m_offsetX = offX;
    m_offsetY = offY;
    m_visible = !(m_offsetX <= -kShijimaWidth || m_offsetX >= kShijimaWidth ||
        m_offsetY <= -kShijimaHeight || m_offsetY >= kShijimaHeight);
    if (m_visible) {
        move(winX, winY);
    }

    // Repaint if needed
    if (needsRepaint) {
        m_activeImage = {};
        repaint();
        update();
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
        auto &active = getActiveImage();
        auto image = active.image;
        auto source = active.sourceRect;
        auto pos = event->pos();
        auto imagePos = pos + source.topLeft() - active.drawOrigin;
        if (imagePos.x() < 0 || imagePos.y() < 0 ||
            imagePos.x() > image->width() || imagePos.y() > image->height())
        {
            event->ignore();
            return;
        }
        auto color = image->pixelColor(imagePos);
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