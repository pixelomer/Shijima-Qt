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

#include "WaylandShimeji.hpp"
#include "../../ActiveMascot.hpp"
#include "MascotBackendWayland.hpp"
#include <QPixmap>
#include <wayland-cursor.h>
#include <cstdint>
#include <wayland-client-protocol.h>
#include <QPainter>

WaylandShimeji::WaylandShimeji(MascotData *mascotData,
    std::unique_ptr<shijima::mascot::manager> mascot,
    int mascotId, MascotBackendWayland *wayland): 
    ActiveMascot(mascotData, std::move(mascot), mascotId),
    m_wayland(wayland)
{
    init();
}

WaylandShimeji::WaylandShimeji(ActiveMascot &old, MascotBackendWayland *wayland):
    ActiveMascot(old), m_wayland(wayland)
{
    init();
}

void WaylandShimeji::init() {
    m_wayland->addClient(this);
    m_closed = false;
    m_region = wl_compositor_create_region(m_wayland->compositor());
    m_surface = wl_compositor_create_surface(m_wayland->compositor());
    wl_surface_set_input_region(m_surface, m_region);
    wl_surface_commit(m_surface);
    wl_display_roundtrip(m_wayland->display());
    m_subsurface = wl_subcompositor_get_subsurface(m_wayland->subcompositor(),
        m_surface, m_wayland->overlaySurface());
    wl_subsurface_set_desync(m_subsurface);
    resetSurface();
    m_cursorSurface = wl_compositor_create_surface(m_wayland->compositor());
    showInspector();
}

void WaylandShimeji::resetSurface() {
    auto container = this->container();
    m_buffer = m_wayland->createBuffer(container.width(),
        container.height());
    wl_surface_attach(m_surface, m_buffer, 0, 0);
    wl_subsurface_set_position(m_subsurface, container.x(),
        container.y());
    wl_surface_commit(m_surface);
    wl_surface_commit(m_wayland->overlaySurface());
    wl_display_roundtrip(m_wayland->display());
}

void WaylandShimeji::redraw() {
    auto &asset = getActiveAsset();
    auto &image = asset.image(isMirroredRender());
    auto scaledSize = image.size() / drawScale();
    if (m_image.size() != QSize { m_buffer.width(), m_buffer.height() }) {
        m_image = QImage { m_buffer.width(), m_buffer.height(),
            QImage::Format_ARGB32 };
        assert((size_t)m_image.sizeInBytes() == m_buffer.size());
    }
    m_image.fill(Qt::transparent);
    QPainter painter { &m_image };
    painter.drawImage(QRect { drawOrigin(), scaledSize }, image);
    memcpy(m_buffer.data(), m_image.bits(), m_buffer.size());
    wl_surface_attach(m_surface, m_buffer, 0, 0);
    wl_surface_damage_buffer(m_surface, 0, 0, m_buffer.width(),
        m_buffer.height());
    wl_surface_commit(m_surface);
    wl_surface_commit(m_wayland->overlaySurface());
}

bool WaylandShimeji::tick() {
    if (markedForDeletion()) {
        m_closed = true;
        return false;
    }
    auto container = this->container();
    auto asset = getActiveAsset();
    bool changed = ActiveMascot::tick();
    if (ActiveMascot::contextMenuVisible()) {
        m_wayland->requestNullRegion();
    }
    //if (changed) {
        auto newContainer = this->container();
        if (container.size() != newContainer.size()) {
            resetSurface();
        }
        if (container.topLeft() != newContainer.topLeft()) {
            wl_subsurface_set_position(m_subsurface, newContainer.x(),
                newContainer.y());
            wl_subsurface_set_desync(m_subsurface);
        }
        redraw();
        m_wayland->invalidateRegion();
        return true;
    //}
    //return false;
}

WaylandShimeji::~WaylandShimeji() {
    m_wayland->removeClient(this);
    wl_subsurface_destroy(m_subsurface);
    wl_surface_destroy(m_surface);
    wl_region_destroy(m_region);
}

bool WaylandShimeji::mascotClosed() {
    return m_closed;
}

void WaylandShimeji::mouseMove(QPointF pos) {
    /*mascot().state->env->cursor.move({ (double)pos.x(),
        (double)pos.y() });*/
}

void WaylandShimeji::mouseDown(Qt::MouseButton button) {
    QPoint global = { (int)mascot().state->env->cursor.x,
        (int)mascot().state->env->cursor.y };
    ActiveMascot::mousePressEvent(button, global);
}

void WaylandShimeji::mouseUp(Qt::MouseButton button) {
    ActiveMascot::mouseReleaseEvent(button);
}

void WaylandShimeji::updateRegion(::wl_region *region) {
    // extremely resource-intensive
    /*
        auto &asset = getActiveAsset();
        auto &image = asset.image(isMirroredRender());
        auto scaledSize = image.size() / drawScale();
        m_parentRegion = QRegion { QBitmap::fromPixmap(asset.mask(
            isMirroredRender()).scaled(scaledSize)) };
        m_parentRegion.translate(container().topLeft() + drawOrigin());
        for (auto rect : m_parentRegion) {
            wl_region_add(region, rect.x(), rect.y(),
                rect.width(), rect.height());
        }
    */
    auto &asset = getActiveAsset();
    auto scale = 1.0 / drawScale();
    m_parentRegion = QRect { container().topLeft() + drawOrigin(),
        asset.offset().size() * scale };
    for (auto rect : m_parentRegion) {
        wl_region_add(region, rect.x(), rect.y(),
            rect.width(), rect.height());
    }
}

bool WaylandShimeji::pointInside(QPointF point) {
    return m_parentRegion.contains(point.toPoint());
}
