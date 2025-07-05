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
#include <shijima/mascot/manager.hpp>
#include <shijima/mascot/environment.hpp>
#include <wayland-client-protocol.h>
#include "../../MascotData.hpp"
#include "../../ActiveMascot.hpp"
#include "WaylandEnvironment.hpp"
#include "WaylandClient.hpp"

class WaylandShimeji : public ActiveMascot, public WaylandClient
{
public:
    friend class ShijimaContextMenu;
    explicit WaylandShimeji(MascotData *mascotData,
        std::unique_ptr<shijima::mascot::manager> mascot,
        int mascotId, std::shared_ptr<WaylandEnvironment> environment);
    explicit WaylandShimeji(ActiveMascot &old,
        std::shared_ptr<WaylandEnvironment> environment);
    virtual bool tick() override;
    virtual ~WaylandShimeji();
    virtual bool mascotClosed() override;
    virtual void updateRegion(::wl_region *region) override;
    virtual void mouseDown(Qt::MouseButton button) override;
    virtual void mouseUp(Qt::MouseButton button) override;
    virtual void mouseMove() override;
    virtual bool pointInside(QPointF point) override;
    void setEnvironment(std::shared_ptr<WaylandEnvironment> env);
    std::shared_ptr<WaylandEnvironment> waylandEnv() { return m_env; }
private:
    void init();
    void initSurface();
    void resetSurface();
    void redraw();
    bool m_closed;
    ::wl_subsurface *m_subsurface;
    ::wl_surface *m_surface;
    std::shared_ptr<WaylandEnvironment> m_env;
    QImage m_image;
    WaylandBuffer m_buffer;
    ::wl_region *m_region = NULL;
    QRegion m_parentRegion;
    shijima::mascot::environment::dvec2 m_cursor;
};