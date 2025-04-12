#pragma once

#include <cstdint>
#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include <wayland-cursor.h>
#include <string>
#include <ostream>

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

std::ostream &operator<<(std::ostream &lhs, WaylandOutput &rhs);
