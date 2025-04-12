#pragma once
#include "../../MascotBackend.hpp"
#include "wayland-protocols/wlr-layer-shell-unstable-v1.h"
#include "wayland-protocols/fractional-scale-v1.h"
#include "WaylandBuffer.hpp"
#include "WaylandOutput.hpp"
#include "WaylandClient.hpp"
#include "WaylandEnvironment.hpp"

class WaylandShimeji;

class MascotBackendWayland : public MascotBackend {
public:
    MascotBackendWayland(ShijimaManager *manager);
    virtual ~MascotBackendWayland() override;
    virtual ActiveMascot *spawn(MascotData *mascotData,
        std::unique_ptr<shijima::mascot::manager> mascot,
        ActiveMascot *parent, int mascotId, bool resetPosition) override;
    virtual ActiveMascot *migrate(ActiveMascot &old) override;
    virtual void preTick() override;
    virtual void postTick() override;
    virtual void updateEnvironments(
        std::function<void(shijima::mascot::environment &)> cb) override;
    WaylandBuffer createBuffer(int width, int height);
    ::wl_compositor *compositor() { return m_compositor; }
    ::wl_display *display() { return m_display; }
    ::wl_subcompositor *subcompositor() { return m_subcompositor; }
    ::zwlr_layer_shell_v1 *layerShell() { return m_layer_shell; }
    ::wl_seat *seat() { return m_seat; }
    ::wp_fractional_scale_manager_v1 *fractionalScaleManager() {
        return m_fractionalScaleManager;
    }
    std::shared_ptr<WaylandEnvironment> environmentAt(QPoint point);
    std::shared_ptr<WaylandEnvironment> spawnEnvironment();
    bool reassignEnvironment(WaylandShimeji *shimeji);
private:
    friend void MascotBackendWayland_register_global(void *data,
        struct wl_registry *wl_registry,
        uint32_t name,
        const char *interface,
        uint32_t version);
    friend void MascotBackendWayland_deregister_global(void *data,
        struct wl_registry *wl_registry,
        uint32_t name);
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
    ::wl_pointer *m_pointer = NULL;
    ::wl_surface *m_pointerSurface = NULL;
    ::wp_fractional_scale_manager_v1 *m_fractionalScaleManager = NULL;
    std::shared_ptr<WaylandEnvironment> m_pointedEnvironment = NULL;
    WaylandBuffer m_layerBuffer;
    bool m_leftMouseDown = false;
    bool m_regionValid = false;
    std::map<::wl_output *, WaylandOutput *> m_outputs;
    std::map<WaylandOutput *, std::shared_ptr<WaylandEnvironment>> m_env;
    double m_scaleFactor;
    bool m_nullRegionRequested;
    shijima::math::vec2 m_cursorOffset;
};
