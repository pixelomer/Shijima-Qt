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
#include "ActiveMascot.hpp"

class QPushButton;
class QPaintEvent;
class QMouseEvent;
class QCloseEvent;
class ShijimaContextMenu;
class ShimejiInspectorDialog;
class MascotBackendWidgets;

class ShijimaWidget : public PlatformWidget<QWidget>, public ActiveMascot
{
public:
    friend class ShijimaContextMenu;
    explicit ShijimaWidget(MascotBackendWidgets *backend,
        MascotData *mascotData,
        std::unique_ptr<shijima::mascot::manager> mascot,
        int mascotId, QWidget *parent = nullptr);
    explicit ShijimaWidget(MascotBackendWidgets *backend,
        ActiveMascot &old, QWidget *parent = nullptr);
    virtual bool tick() override;
    virtual ~ShijimaWidget();
    virtual bool mascotClosed() override;
    virtual bool updateOffsets() override;
    virtual void show() override;
protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
private:
    void widgetSetup();
    MascotBackendWidgets *m_backend;
#ifdef __linux__
    QRegion m_windowMask;
#endif
};