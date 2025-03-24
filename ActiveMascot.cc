// 
// Shijima-Qt - Cross-platform shimeji simulation app for desktop
// Copyright (C) 2025 pixelomer
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// 

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
#include "Platform/Platform.hpp"
#include "ShimejiInspectorDialog.hpp"
#include "AssetLoader.hpp"
#include "ShijimaContextMenu.hpp"
#include "ShijimaManager.hpp"
#include <shimejifinder/utils.hpp>
#include "ActiveMascot.hpp"

using namespace shijima;

ActiveMascot::ActiveMascot(MascotData *mascotData,
    std::unique_ptr<shijima::mascot::manager> mascot,
    int mascotId):
    m_data(mascotData), m_inspector(nullptr),
    m_mascotId(mascotId)
{
    m_containerRect.setRect(0, 0, 128, 128);
    m_mascot = std::move(mascot);
    
    QDir dir { m_data->imgRoot() };
    if (dir.exists() && dir.cdUp() && dir.cd("sound")) {
        m_sounds.searchPaths.push_back(dir.path());
    }
}


ActiveMascot::ActiveMascot(ActiveMascot &old):
    ActiveMascot(old.mascotData(),
    std::move(old.m_mascot), old.m_mascotId) {}

void ActiveMascot::showInspector() {
    if (m_inspector == nullptr) {
        m_inspector = new ShimejiInspectorDialog { this };
    }
    m_inspector->show();
}

bool ActiveMascot::inspectorVisible() {
    return m_inspector != nullptr && m_inspector->isVisible();
}

Asset const& ActiveMascot::getActiveAsset() {
    auto &name = m_mascot->state->active_frame.get_name(
        m_mascot->state->looking_right);
    auto lowerName = shimejifinder::to_lower(name);
    auto imagePath = QDir::cleanPath(m_data->imgRoot()
        + QDir::separator() + QString::fromStdString(lowerName));
    return AssetLoader::defaultLoader()->loadAsset(imagePath);
}

bool ActiveMascot::isMirroredRender() const {
    return m_mascot->state->active_frame.right_name.empty() &&
        m_mascot->state->looking_right;
}

bool ActiveMascot::updateOffsets() {
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

    if (windowWidth != m_containerRect.width()) {
        needsRepaint = true;
    }
    if (windowHeight != m_containerRect.height()) {
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
    int winX = (int)m_mascot->state->anchor.x - m_anchorInWindow.x()
        - (int)env()->screen.left;
    int winY = (int)m_mascot->state->anchor.y - m_anchorInWindow.y()
        - (int)env()->screen.top;
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
    winX += (int)env()->screen.left;
    winY += (int)env()->screen.top;

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
    
    m_containerRect.setRect(winX, winY, windowWidth, windowHeight);

    return needsRepaint;
}

bool ActiveMascot::pointInside(QPoint const& point) {
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

bool ActiveMascot::tick() {
    if (m_markedForDeletion) {
        return false;
    }
    if (paused()) {
        return false;
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

    return offsetsChanged || forceRepaint;
}

void ActiveMascot::contextMenuClosed(QCloseEvent *event) {
    m_contextMenuVisible = false;
}

void ActiveMascot::showContextMenu(QPoint const& pos) {
    m_contextMenuVisible = true;
    ShijimaContextMenu *menu = new ShijimaContextMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->popup(pos);
}

bool ActiveMascot::mascotClosed() {
    return true;
}

void ActiveMascot::show() {
}

ActiveMascot::~ActiveMascot() {
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

void ActiveMascot::setDragTarget(ActiveMascot *target) {
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


bool ActiveMascot::mousePressEvent(Qt::MouseButton button, QPoint global) {
    if (m_dragTarget != nullptr) {
        m_dragTarget->m_mascot->state->dragging = false;
    }
    setDragTarget(this);
    if (button == Qt::MouseButton::LeftButton) {
        m_dragTarget->m_mascot->state->dragging = true;
    }
    else if (button == Qt::MouseButton::RightButton) {
        m_dragTarget->showContextMenu(global);
        setDragTarget(nullptr);
    }
    return true;
}

bool ActiveMascot::mousePressEvent(Qt::MouseButton button, QPoint local,
    QPoint global)
{
    if (m_dragTarget != nullptr) {
        m_dragTarget->m_mascot->state->dragging = false;
    }
    if (pointInside(local)) {
        setDragTarget(this);
    }
    else {
        ActiveMascot *target = ShijimaManager::defaultManager()->hitTest(global);
        setDragTarget(target);
        if (target == nullptr) {
            return false;
        }
    }
    if (button == Qt::MouseButton::LeftButton) {
        m_dragTarget->m_mascot->state->dragging = true;
    }
    else if (button == Qt::MouseButton::RightButton) {
        m_dragTarget->showContextMenu(global);
        setDragTarget(nullptr);
    }
    return true;
}

void ActiveMascot::mouseReleaseEvent(Qt::MouseButton button) {
    if (m_dragTarget == nullptr) {
        return;
    }
    if (button == Qt::MouseButton::LeftButton) {
        m_dragTarget->m_mascot->state->dragging = false;
        setDragTarget(nullptr);
    }
}
