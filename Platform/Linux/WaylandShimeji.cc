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
#include <wayland-client-protocol.h>
#include <QPainter>

WaylandShimeji::WaylandShimeji(MascotData *mascotData,
    std::unique_ptr<shijima::mascot::manager> mascot,
    int mascotId, std::shared_ptr<WaylandEnvironment> environment): 
    ActiveMascot(mascotData, std::move(mascot), mascotId)
{
    setEnvironment(environment);
    init();
}

WaylandShimeji::WaylandShimeji(ActiveMascot &old,
    std::shared_ptr<WaylandEnvironment> environment):
    ActiveMascot(old)
{
    setEnvironment(environment);
    init();
}

void WaylandShimeji::init() {
    m_closed = false;
    m_region = wl_compositor_create_region(m_env->backend()->compositor());
    initSurface();   
}

void WaylandShimeji::initSurface() {
    m_surface = wl_compositor_create_surface(m_env->backend()->compositor());
    wl_surface_set_input_region(m_surface, m_region);
    wl_surface_commit(m_surface);
    wl_display_roundtrip(m_env->backend()->display());
    m_subsurface = wl_subcompositor_get_subsurface(
        m_env->backend()->subcompositor(),
        m_surface, m_env->surface());
    wl_subsurface_set_desync(m_subsurface);
    resetSurface();
}

void WaylandShimeji::resetSurface() {
    auto container = this->container();
    m_buffer = m_env->backend()->createBuffer(container.width(),
        container.height());
    wl_surface_attach(m_surface, m_buffer, 0, 0);
    wl_subsurface_set_position(m_subsurface, container.x(),
        container.y());
    wl_surface_commit(m_surface);
    wl_surface_commit(m_env->surface());
    wl_display_roundtrip(m_env->backend()->display());
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
    wl_surface_commit(m_env->surface());
}

bool WaylandShimeji::tick() {
    if (!m_env->valid()) {
        mascot().state->dragging = false;
        m_env->backend()->reassignEnvironment(this);
    }
    if (markedForDeletion()) {
        m_closed = true;
        return false;
    }
    auto container = this->container();
    auto asset = getActiveAsset();
    bool changed = ActiveMascot::tick();
    if (ActiveMascot::contextMenuVisible()) {
        m_env->requestNullRegion();
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
        m_env->invalidateRegion();
        return true;
    //}
    //return false;
}

WaylandShimeji::~WaylandShimeji() {
    m_env->removeClient(this);
    wl_subsurface_destroy(m_subsurface);
    wl_surface_destroy(m_surface);
    wl_region_destroy(m_region);
}

bool WaylandShimeji::mascotClosed() {
    return m_closed;
}

void WaylandShimeji::mouseDown(Qt::MouseButton button) {
    QPoint global = { ((int)mascot().state->env->cursor.x + m_env->output()->logicalX()) * m_env->scaleFactor(),
        ((int)mascot().state->env->cursor.y + m_env->output()->logicalY()) * m_env->scaleFactor() };
    ActiveMascot::mousePressEvent(button, global);
}

void WaylandShimeji::mouseUp(Qt::MouseButton button) {
    ActiveMascot::mouseReleaseEvent(button);
}

void WaylandShimeji::mouseMove() {
    auto cursor = mascot().state->env->cursor;
    if (mascot().state->dragging &&
        (cursor.x < 0 || cursor.x >= m_env->env()->screen.right ||
        cursor.y < 0 || cursor.y >= m_env->env()->screen.bottom))
    {
        // move to a different monitor if necessary
        if (m_env->backend()->reassignEnvironment(this)) {
            wl_subsurface_destroy(m_subsurface);
            wl_surface_destroy(m_surface);
            initSurface();
        }
    }
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

void WaylandShimeji::setEnvironment(std::shared_ptr<WaylandEnvironment> env) {
    if (m_env != nullptr) {
        m_env->removeClient(this);
    }
    m_env = env;
    env->addClient(this);
    setEnv(m_env->env());
}

bool WaylandShimeji::pointInside(QPointF point) {
    return m_parentRegion.contains(point.toPoint());
}
