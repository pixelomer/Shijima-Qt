#pragma once

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

#include <QWidget>
#include <memory>
#include <QRegion>
#include "Asset.hpp"
#include "SoundEffectManager.hpp"
#include <shijima/mascot/manager.hpp>
#include <shijima/mascot/environment.hpp>
#include "PlatformWidget.hpp"
#include "MascotData.hpp"

class QPushButton;
class QPaintEvent;
class QMouseEvent;
class QCloseEvent;
class ShijimaContextMenu;
class ShimejiInspectorDialog;

class ActiveMascot {
public:
    friend class ShijimaContextMenu;
    explicit ActiveMascot(MascotData *mascotData,
        std::unique_ptr<shijima::mascot::manager> mascot,
        int mascotId);
    explicit ActiveMascot(ActiveMascot &old);
    virtual bool tick();
    bool pointInside(QPoint const& point);
    int mascotId() { return m_mascotId; }
    void showInspector();
    void markForDeletion() { m_markedForDeletion = true; }
    bool inspectorVisible();
    bool paused() const { return m_paused || m_contextMenuVisible; }
    shijima::mascot::manager &mascot() {
        return *m_mascot;
    }
    void setEnv(std::shared_ptr<shijima::mascot::environment> env) {
        m_mascot->state->env = env;
    }
    std::shared_ptr<shijima::mascot::environment> env() {
        return m_mascot->state->env; 
    }
    MascotData *mascotData() {
        return m_data;
    }
    QString const& mascotName() {
        return m_data->name();
    }
    virtual ~ActiveMascot();
    int y() { return m_containerRect.y(); }
    int x() { return m_containerRect.x(); }
    virtual bool mascotClosed();
    virtual void show();
    QRect container() { return m_containerRect; }
    QPoint drawOrigin() { return m_drawOrigin; }
protected:
    bool visible() { return m_visible; }
    double drawScale() { return m_drawScale; }
    bool mousePressEvent(Qt::MouseButton button, QPoint local, QPoint global);
    bool mousePressEvent(Qt::MouseButton button, QPoint global);
    void mouseReleaseEvent(Qt::MouseButton button);
    void setDragTarget(ActiveMascot *target);
    bool isMirroredRender() const;
    void closeAction();
    void contextMenuClosed(QCloseEvent *);
    void showContextMenu(QPoint const&);
    virtual bool updateOffsets();
    bool markedForDeletion() { return m_markedForDeletion; }
    Asset const& getActiveAsset();
    bool contextMenuVisible() const { return m_contextMenuVisible; }
private:
    MascotData *m_data;
    ShimejiInspectorDialog *m_inspector;
    SoundEffectManager m_sounds;
    ActiveMascot *m_dragTarget = nullptr;
    ActiveMascot **m_dragTargetPt = nullptr;
    std::unique_ptr<shijima::mascot::manager> m_mascot;
    QRect m_imageRect;
    QPoint m_anchorInWindow;
    double m_drawScale = 1.0;
    QPoint m_drawOrigin;
    QRect m_containerRect;
    bool m_visible;
    bool m_contextMenuVisible = false;
    bool m_paused = false;
    bool m_markedForDeletion = false;
    int m_mascotId;
};
