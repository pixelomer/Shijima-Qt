#include "ShijimaWidget.hpp"
#include <QWidget>
#include <QPainter>
#include <QFile>
#include <QDir>
#include <QScreen>
#include <QMouseEvent>
#include <QMenu>
#include <QWindow>
#include <QDebug>
#include <QGuiApplication>
#include <QTextStream>
#include <shijima/shijima.hpp>
#include "ShimejiInspectorDialog.hpp"
#include "AssetLoader.hpp"
#include "ShijimaContextMenu.hpp"
#include "ShijimaManager.hpp"

using namespace shijima;

ShijimaWidget::ShijimaWidget(std::string const& mascotName,
    std::string const& imgRoot,
    std::unique_ptr<shijima::mascot::manager> mascot,
    QWidget *parent):
#if defined(__APPLE__)
    QWidget(nullptr),
#else
    QWidget(parent),
#endif
    m_inspector(nullptr)
{
    m_mascotName = mascotName;
    m_windowHeight = 128;
    m_windowWidth = 128;
    m_imgRoot = imgRoot;
    m_mascot = std::move(mascot);

    QString qImgRoot = QString::fromStdString(imgRoot);
    QDir dir { qImgRoot };
    if (dir.exists() && dir.cdUp() && dir.cd("sound")) {
        m_sounds.searchPaths.push_back(dir.path());
    }
    
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_MacShowFocusRect, false);
    setFixedSize(m_windowWidth, m_windowHeight);
    Qt::WindowFlags flags = Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint
        | Qt::WindowDoesNotAcceptFocus | Qt::NoDropShadowWindowHint
        | Qt::WindowOverridesSystemGestures;
    #if defined(__APPLE__)
    flags |= Qt::Window;
    #else
    flags |= Qt::Tool;
    #endif
    setWindowFlags(flags);
}

void ShijimaWidget::showInspector() {
    if (m_inspector == nullptr) {
        m_inspector = new ShimejiInspectorDialog { this };
    }
    m_inspector->show();
}

Asset const& ShijimaWidget::getActiveAsset() {
    auto &name = m_mascot->state->active_frame.get_name(m_mascot->state->looking_right);
    auto imagePath = QDir::cleanPath(QString::fromStdString(m_imgRoot)
        + QDir::separator() + QString(name.c_str()));
    return AssetLoader::defaultLoader()->loadAsset(imagePath);
}

bool ShijimaWidget::isMirroredRender() const {
    return m_mascot->state->active_frame.right_name.empty() &&
        m_mascot->state->looking_right;
}

void ShijimaWidget::paintEvent(QPaintEvent *event) {
    if (!m_visible) {
        return;
    }
    QPainter painter(this);
    auto &asset = getActiveAsset();
    auto &image = asset.image(isMirroredRender());
    painter.drawImage(QRect { m_drawOrigin, image.size() / m_drawScale }, image);
}

bool ShijimaWidget::updateOffsets() {
    bool needsRepaint = false;
    auto &frame = m_mascot->state->active_frame;
    auto &asset = getActiveAsset();
    
    // Does the image go outside of the minimum boundary? If so,
    // extend the window boundary
    int originalWidth = asset.originalSize().width();
    int originalHeight = asset.originalSize().height();
    double scale = m_mascot->state->env->get_scale();
    int screenWidth = (int)(m_mascot->state->env->screen.width()
        / scale);
    int screenHeight = (int)(m_mascot->state->env->screen.height()
        / scale);
    int windowWidth = (int)(originalWidth / scale);
    int windowHeight = (int)(originalHeight / scale);

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
    if (isMirroredRender()) {
        m_anchorInWindow = {
            (int)((originalWidth - frame.anchor.x) / scale),
            (int)(frame.anchor.y / scale) };
    }
    else {
        m_anchorInWindow = { (int)(frame.anchor.x / scale),
            (int)(frame.anchor.y / scale) };
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

    if (isMirroredRender()) {
        drawOffset += QPoint {
            (int)((originalWidth - asset.offset().topRight().x()) / scale),
            (int)(asset.offset().topLeft().y() / scale) };
    }
    else {
        drawOffset += asset.offset().topLeft() / scale;
    }
    if (drawOffset != m_drawOrigin) {
        needsRepaint = true;
        m_drawOrigin = drawOffset;
    }
    if (scale != m_drawScale) {
        needsRepaint = true;
        m_drawScale = scale;
    }
    move(winX, winY);

    return needsRepaint;
}

bool ShijimaWidget::pointInside(QPoint const& point) {
    if (!m_visible) {
        return false;
    }
    auto &asset = getActiveAsset();
    auto image = asset.image(isMirroredRender());
    int drawnWidth = (int)(image.width() / m_drawScale);
    int drawnHeight = (int)(image.height() / m_drawScale);
    auto imagePos = point - m_drawOrigin;
    if (imagePos.x() < 0 || imagePos.y() < 0 ||
        imagePos.x() > drawnWidth || imagePos.y() > drawnHeight)
    {
        return false;
    }
    //FIXME: is this position correct?
    auto color = image.pixelColor(imagePos * m_drawScale);
    if (color.alpha() == 0) {
        return false;
    }
    return true;
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
    if (m_mascot->state->dead) {
        forceRepaint = true;
        new_frame.name = "";
        new_sound = "";
        m_mascot->state->active_sound_changed = true;
        markForDeletion();
    }
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

    // Update inspector
    if (m_inspector != nullptr && m_inspector->isVisible()) {
        m_inspector->tick();
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

ShijimaWidget::~ShijimaWidget() {
    if (m_dragTargetPt != nullptr) {
        *m_dragTargetPt = nullptr;
        m_dragTargetPt = nullptr;
    }
    if (m_inspector != nullptr) {
        m_inspector->close();
        delete m_inspector;
    }
    setDragTarget(nullptr);
}

void ShijimaWidget::setDragTarget(ShijimaWidget *target) {
    if (m_dragTarget != nullptr) {
        m_dragTarget->m_dragTargetPt = nullptr;
    }
    if (target != nullptr) {
        if (target->m_dragTargetPt != nullptr) {
            throw std::runtime_error("target widget being dragged by multiple widgets");
        }
        m_dragTarget = target;
        m_dragTarget->m_dragTargetPt = &m_dragTarget;
    }
    else {
        m_dragTarget = nullptr;
    }
}

void ShijimaWidget::mousePressEvent(QMouseEvent *event) {
    auto pos = event->pos();
    auto screenPos = mapToGlobal(pos);
    if (m_dragTarget != nullptr) {
        m_dragTarget->m_mascot->state->dragging = false;
    }
    if (pointInside(pos)) {
        setDragTarget(this);
    }
    else {
        ShijimaWidget *target = ShijimaManager::defaultManager()->hitTest(screenPos);
        setDragTarget(target);
        if (target == nullptr) {
            event->ignore();
            return;
        }
    }
    if (event->button() == Qt::MouseButton::LeftButton) {
        m_dragTarget->m_mascot->state->dragging = true;
    }
    else if (event->button() == Qt::MouseButton::RightButton) {
        m_dragTarget->showContextMenu(screenPos);
        setDragTarget(nullptr);
    }
}

void ShijimaWidget::closeAction() {
    close();
}

void ShijimaWidget::mouseReleaseEvent(QMouseEvent *event) {
    if (m_dragTarget == nullptr) {
        return;
    }
    if (event->button() == Qt::MouseButton::LeftButton) {
        m_dragTarget->m_mascot->state->dragging = false;
        setDragTarget(nullptr);
    }
}