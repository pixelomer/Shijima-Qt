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

class ShijimaWidget : public PlatformWidget<QWidget>
{
public:
    friend class ShijimaContextMenu;
    explicit ShijimaWidget(MascotData *mascotData,
        std::unique_ptr<shijima::mascot::manager> mascot,
        int mascotId, bool windowedMode, QWidget *parent = nullptr);
    explicit ShijimaWidget(ShijimaWidget &old, bool windowedMode,
        QWidget *parent = nullptr);
    void tick();
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
    ~ShijimaWidget();
protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
private:
    void setDragTarget(ShijimaWidget *target);
    bool isMirroredRender() const;
    void closeAction();
    void contextMenuClosed(QCloseEvent *);
    void showContextMenu(QPoint const&);
    bool updateOffsets();
#ifdef __linux__
    QRegion m_windowMask;
#endif
    bool m_windowedMode;
    MascotData *m_data;
    ShimejiInspectorDialog *m_inspector;
    SoundEffectManager m_sounds;
    Asset const& getActiveAsset();
    ShijimaWidget *m_dragTarget = nullptr;
    ShijimaWidget **m_dragTargetPt = nullptr;
    std::unique_ptr<shijima::mascot::manager> m_mascot;
    QRect m_imageRect;
    QPoint m_anchorInWindow;
    double m_drawScale = 1.0;
    QPoint m_drawOrigin;
    int m_windowHeight;
    int m_windowWidth;
    bool m_visible;
    bool m_contextMenuVisible = false;
    bool m_paused = false;
    bool m_markedForDeletion = false;
    int m_mascotId;
};