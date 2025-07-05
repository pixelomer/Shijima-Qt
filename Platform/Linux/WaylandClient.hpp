#pragma once

#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include <wayland-cursor.h>
#include <QMouseEvent>
#include <QPointF>

class WaylandClient {
public:
    virtual void updateRegion(::wl_region *region) = 0;
    virtual void mouseDown(Qt::MouseButton button) = 0;
    virtual void mouseUp(Qt::MouseButton button) = 0;
    virtual void mouseMove() = 0;
    virtual bool pointInside(QPointF pos) = 0;
};
