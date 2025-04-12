#pragma once

#include <shijima/mascot/environment.hpp>
#include <memory>
#include <wayland-client.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include "wayland-protocols/fractional-scale-v1.h"
#include "wayland-protocols/wlr-layer-shell-unstable-v1.h"
#include "WaylandBuffer.hpp"
#include <list>

class WaylandClient;
class WaylandOutput;
class MascotBackendWayland;

class WaylandEnvironment {
public:
    WaylandEnvironment(MascotBackendWayland *backend, WaylandOutput *output);
    std::shared_ptr<shijima::mascot::environment> env() { return m_env; }
    WaylandOutput *output() { return m_output; }
    MascotBackendWayland *backend() { return m_backend; }
    ::wl_surface *surface() { return m_surface; }
    bool valid() { return m_valid; }
    void invalidate() { m_valid = false; }
    void pointerEnter();
    void pointerLeave();
    void pointerMove(double x, double y);
    void pointerButton(uint32_t button, uint32_t state);
    void pointerButton(WaylandClient *client, uint32_t button, uint32_t state);
    void preTick();
    void postTick();
    void tick();
    void addClient(WaylandClient *client);
    void removeClient(WaylandClient *client);
    void invalidateRegion() { m_regionValid = false; }
    int32_t scaleFactor() { return m_scaleFactor; }
    void requestNullRegion() { m_nullRegionRequested = true; }
    ~WaylandEnvironment();
    void destroy();
private:
    void updateRegion();
    void releaseDragTarget();
    void initEnvironment();
    void finalizeEnvironment();
    friend void WaylandEnvironment_preferred_scale(void *data,
        struct wp_fractional_scale_v1 *wp_fractional_scale_v1,
        uint32_t scale);
    friend void WaylandEnvironment_layer_surface_configure(void *data,
        struct zwlr_layer_surface_v1 *zwlr_layer_surface_v1,
        uint32_t serial,
        uint32_t width,
        uint32_t height);
    friend void WaylandEnvironment_layer_surface_closed(void *data,
        struct zwlr_layer_surface_v1 *zwlr_layer_surface_v1);
    std::shared_ptr<shijima::mascot::environment> m_env;
    WaylandOutput *m_output;
    MascotBackendWayland *m_backend;
    ::wl_surface *m_surface;
    ::wp_fractional_scale_v1 *m_fractionalScale;
    ::zwlr_layer_surface_v1 *m_layerSurface;
    ::wl_region *m_layerRegion;
    WaylandBuffer m_layerBuffer;
    bool m_valid;
    double m_scaleFactor;
    bool m_regionValid;
    bool m_nullRegionRequested;
    std::list<WaylandClient *> m_clients;
    WaylandClient *m_activeDragTarget;
};