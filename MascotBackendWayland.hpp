#pragma once
#include "MascotBackend.hpp"
#include "wayland-protocols/wlr-layer-shell.h"
#include "wayland-protocols/fractional-scale-v1.h"
#include <cstdint>
#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include <wayland-cursor.h>
#include <map>
#include <string>
#include <QPoint>
#include <list>

class WaylandClient {
public:
    virtual void updateRegion(::wl_region *region) = 0;
    virtual void mouseMove(QPointF pos) = 0;
    virtual void mouseDown(Qt::MouseButton button) = 0;
    virtual void mouseUp(Qt::MouseButton button) = 0;
    virtual bool pointInside(QPointF pos) = 0;
};

class WaylandBuffer {
public:
    int width() { return m_width; }
    int height() { return m_height; }
    size_t size() { return m_width * m_height * 4; }
    bool valid() { return m_valid; }
    ::wl_buffer *buffer() { return m_buffer; }
    uint8_t *data() { return m_data; }
    operator ::wl_buffer*() { return m_buffer; }
    uint8_t &operator[](size_t idx) { return m_data[idx]; }
    WaylandBuffer();
    WaylandBuffer(int width, int height, ::wl_buffer *buffer,
        uint8_t *data, int fd);
    WaylandBuffer(WaylandBuffer const&) = delete;
    WaylandBuffer(WaylandBuffer&&);
    WaylandBuffer &operator=(WaylandBuffer&&);
    void destroy();
    ~WaylandBuffer();
private:
    int m_width;
    int m_height;
    bool m_valid;
    ::wl_buffer *m_buffer;
    uint8_t *m_data;
    int m_fd;
};

class WaylandOutput {
public:
    WaylandOutput();
    WaylandOutput(::wl_output *);
    WaylandOutput(WaylandOutput const&) = delete;
    ~WaylandOutput();
    ::wl_output *output() { return m_output; }
    int32_t x() { return m_x; }
    int32_t y() { return m_y; }
    int32_t width() { return m_width; }
    int32_t height() { return m_height; }
    int32_t top() { return y(); }
    int32_t right() { return x() + width(); }
    int32_t bottom() { return y() + height(); }
    int32_t left() { return x(); }
    int32_t subpixel() { return m_subpixel; }
    int32_t refresh() { return m_refresh; }
    int32_t factor() { return m_factor; }
    std::string const& make() { return m_make; }
    std::string const& model() { return m_model; }
    std::string const& name() { return m_name; }
    std::string const& description() { return m_description; }
private:
    friend std::ostream &operator<<(std::ostream &lhs, WaylandOutput &rhs);
    friend void MascotBackendWayland_Output_geometry(void *data,
        struct wl_output *wl_output,
        int32_t x,
        int32_t y,
        int32_t physical_width,
        int32_t physical_height,
        int32_t subpixel,
        const char *make,
        const char *model,
        int32_t transform);
    friend void MascotBackendWayland_Output_mode(void *data,
        struct wl_output *wl_output,
        uint32_t flags,
        int32_t width,
        int32_t height,
        int32_t refresh);
    friend void MascotBackendWayland_Output_done(void *data,
        struct wl_output *wl_output);
    friend void MascotBackendWayland_Output_scale(void *data,
        struct wl_output *wl_output,
        int32_t factor);
    friend void MascotBackendWayland_Output_name(void *data,
        struct wl_output *wl_output,
        const char *name);
    friend void MascotBackendWayland_Output_description(void *data,
        struct wl_output *wl_output,
        const char *description);
    ::wl_output *m_output = NULL;
    int32_t m_x = 0, m_y = 0, m_width = 0, m_height = 0, m_subpixel = 0,
        m_refresh = 0, m_factor = 0;
    std::string m_make, m_model, m_name, m_description;
};

class MascotBackendWayland : public MascotBackend {
public:
    MascotBackendWayland(ShijimaManager *manager);
    virtual ~MascotBackendWayland() override;
    virtual ActiveMascot *spawn(MascotData *mascotData,
        std::unique_ptr<shijima::mascot::manager> mascot,
        int mascotId) override;
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
