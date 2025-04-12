#pragma once
#include "../../MascotBackend.hpp"
#include "wayland-protocols/wlr-layer-shell-unstable-v1.h"
#include "wayland-protocols/fractional-scale-v1.h"
#include "WaylandBuffer.hpp"
#include "WaylandOutput.hpp"
#include "WaylandClient.hpp"

class MascotBackendWayland : public MascotBackend {
public:
    MascotBackendWayland(ShijimaManager *manager);
    virtual ~MascotBackendWayland() override;
    virtual ActiveMascot *spawn(MascotData *mascotData,
        std::unique_ptr<shijima::mascot::manager> mascot,
        int mascotId, bool resetPosition) override;
    virtual ActiveMascot *migrate(ActiveMascot &old) override;
    virtual void preTick() override;
    virtual void postTick() override;
    virtual void updateEnvironments(
        std::function<void(shijima::mascot::environment &)> cb) override;
    WaylandBuffer createBuffer(int width, int height);
    ::wl_compositor *compositor() { return m_compositor; }
    ::wl_display *display() { return m_display; }
    ::wl_subcompositor *subcompositor() { return m_subcompositor; }
    ::wl_surface *overlaySurface() { return m_surface; }
    void addClient(WaylandClient *client);
    void removeClient(WaylandClient *client);
    void invalidateRegion() { m_regionValid = false; }
    bool leftMouseDown() { return m_leftMouseDown; }
    int32_t scaleFactor() { return m_scaleFactor; }
    void requestNullRegion() { m_nullRegionRequested = true; }
private:
    friend void MascotBackendWayland_register_global(void *data,
        struct wl_registry *wl_registry,
        uint32_t name,
        const char *interface,
        uint32_t version);
    friend void MascotBackendWayland_deregister_global(void *data,
        struct wl_registry *wl_registry,
        uint32_t name);
    friend void MascotBackendWayland_layer_surface_configure(void *data,
        struct zwlr_layer_surface_v1 *zwlr_layer_surface_v1,
        uint32_t serial,
        uint32_t width,
        uint32_t height);
    friend void MascotBackendWayland_layer_surface_closed(void *data,
        struct zwlr_layer_surface_v1 *zwlr_layer_surface_v1);
    friend void MascotBackendWayland_pointer_enter(void *data,
        struct wl_pointer *wl_pointer,
        uint32_t serial,
        struct wl_surface *surface,
        wl_fixed_t surface_x,
        wl_fixed_t surface_y);
    friend void MascotBackendWayland_pointer_leave(void *data,
        struct wl_pointer *wl_pointer,
        uint32_t serial,
        struct wl_surface *surface);
    friend void MascotBackendWayland_pointer_motion(void *data,
        struct wl_pointer *wl_pointer,
        uint32_t time,
        wl_fixed_t surface_x,
        wl_fixed_t surface_y);
    friend void MascotBackendWayland_pointer_button(void *data,
        struct wl_pointer *wl_pointer,
        uint32_t serial,
        uint32_t time,
        uint32_t button,
        uint32_t state);
    friend void MascotBackendWayland_preferred_scale(void *data,
        struct wp_fractional_scale_v1 *wp_fractional_scale_v1,
        uint32_t scale);
    void initEnvironment();
    void finalizeEnvironment();
    ::wl_display *m_display = NULL;
    ::wl_registry *m_registry = NULL;
    ::wl_shm *m_shm = NULL;
    ::wl_compositor *m_compositor = NULL;
    ::wl_subcompositor *m_subcompositor = NULL;
    ::zwlr_layer_shell_v1 *m_layer_shell = NULL;
    ::wl_seat *m_seat = NULL;
    ::wl_buffer *m_leftCursorBuffer = NULL;
    ::wl_cursor *m_leftCursor = NULL;
    ::wl_cursor_image *m_leftCursorImage = NULL;
    ::wl_cursor_theme *m_cursorTheme = NULL;
    ::wl_surface *m_surface = NULL;
    ::zwlr_layer_surface_v1 *m_layer_surface = NULL;
    ::wl_pointer *m_pointer = NULL;
    ::wl_surface *m_pointerSurface = NULL;
    ::wl_region *m_layerRegion = NULL;
    ::wp_fractional_scale_manager_v1 *m_fractionalScaleManager = NULL;
    ::wp_fractional_scale_v1 *m_fractionalScale = NULL;
    WaylandOutput *m_activeOutput = NULL;
    WaylandBuffer m_layerBuffer;
    WaylandClient *m_activeMouseListener = NULL;
    bool m_leftMouseDown = false;
    bool m_regionValid = false;
    QPointF m_cursorPosition;
    std::map<::wl_output *, WaylandOutput *> m_outputs;
    std::shared_ptr<shijima::mascot::environment> m_env;
    std::list<WaylandClient *> m_clients;
    double m_scaleFactor;
    bool m_nullRegionRequested;
};
