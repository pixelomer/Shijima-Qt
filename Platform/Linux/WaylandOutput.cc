#include "WaylandOutput.hpp"
#include "wayland-protocols/xdg-output-unstable-v1.h"

std::ostream &operator<<(std::ostream &lhs, WaylandOutput &rhs) {
    lhs << "<WaylandOutput "
        << "wl_output=" << rhs.output() << " "
        << "name=\"" << rhs.name() << "\" "
        << "description=\"" << rhs.description() << "\" "
        << "make=\"" << rhs.make() << "\" "
        << "model=\"" << rhs.model() << "\" "
        << "logical_width=" << rhs.logicalWidth() << " "
        << "logical_height=" << rhs.logicalHeight() << " "
        << "logical_x=" << rhs.logicalX() << " "
        << "logical_y=" << rhs.logicalY() << " "
        << "factor=" << rhs.factor() << " "
        << "refresh=" << rhs.refresh() / 1000.0
        << ">";
    return lhs;
}

void MascotBackendWayland_Output_geometry(void *data,
    struct wl_output *wl_output,
    int32_t x,
    int32_t y,
    int32_t physical_width,
    int32_t physical_height,
    int32_t subpixel,
    const char *make,
    const char *model,
    int32_t transform)
{
    (void)transform;
    auto *output = (WaylandOutput *)data;
    //output->ready = false;
    output->m_x = x;
    output->m_y = y;
    output->m_subpixel = subpixel;
    output->m_make = make;
    output->m_model = model;
    printf("output %p: x=%d, y=%d, pw=%d, ph=%d, subpixel=%d, make=%s\n",
        (void *)wl_output, x, y, physical_width, physical_height, subpixel, make);
}

void MascotBackendWayland_Output_mode(void *data,
    struct wl_output *wl_output,
    uint32_t flags,
    int32_t width,
    int32_t height,
    int32_t refresh)
{
    (void)flags;
    auto *output = (WaylandOutput *)data;
    //output->ready = false;
    output->m_width = width;
    output->m_height = height;
    output->m_refresh = refresh;
    printf("output %p: w=%d, h=%d, r=%d\n",
        (void *)wl_output, width, height, refresh);
}

void MascotBackendWayland_Output_done(void *data,
    struct wl_output *wl_output)
{
    (void)data; (void)wl_output;
    /*
    auto *output = (WaylandOutput *)data;
    output->ready = true;
    printf("output %p: done\n", (void *)wl_output);
    */
}

void MascotBackendWayland_Output_scale(void *data,
    struct wl_output *wl_output,
    int32_t factor)
{
    // This should NOT be used to scale content
    // See wl_surface.preferred_surface_scale instead
    
    auto *output = (WaylandOutput *)data;
    //output->ready = false;
    output->m_factor = factor;
    printf("output %p: f=%d\n", (void *)wl_output, factor);
}

void MascotBackendWayland_Output_name(void *data,
    struct wl_output *wl_output,
    const char *name)
{
    auto *output = (WaylandOutput *)data;
    //output->ready = false;
    output->m_name = name;
    printf("output %p: n=%s\n", (void *)wl_output, name);
}

void MascotBackendWayland_Output_description(void *data,
    struct wl_output *wl_output,
    const char *description)
{
    auto *output = (WaylandOutput *)data;
    //output->ready = false;
    output->m_name = description;
    printf("output %p: d=%s\n", (void *)wl_output, description);
}

void MascotBackendWayland_Output_logical_position(void *data,
    struct zxdg_output_v1 *zxdg_output_v1,
    int32_t x,
    int32_t y)
{
    (void)zxdg_output_v1;
    auto *output = (WaylandOutput *)data;
    output->m_logicalX = x;
    output->m_logicalY = y;
}

void MascotBackendWayland_Output_logical_size(void *data,
    struct zxdg_output_v1 *zxdg_output_v1,
    int32_t width,
    int32_t height)
{
    (void)zxdg_output_v1;
    auto *output = (WaylandOutput *)data;
    output->m_logicalWidth = width;
    output->m_logicalHeight = height;
}

void MascotBackendWayland_Output_xdg_done(void *data,
    struct zxdg_output_v1 *zxdg_output_v1)
{
    // Deprecated
    (void)data; (void)zxdg_output_v1;
}

void MascotBackendWayland_Output_xdg_name(void *data,
    struct zxdg_output_v1 *zxdg_output_v1,
    const char *name)
{
    // Deprecated
    (void)data; (void)zxdg_output_v1; (void)name;
}

void MascotBackendWayland_Output_xdg_description(void *data,
    struct zxdg_output_v1 *zxdg_output_v1,
    const char *description)
{
    // Deprecated
    (void)data; (void)zxdg_output_v1; (void)description;
}

void WaylandOutput::setXdgOutput(::zxdg_output_v1 *output) {
    m_xdgOutput = output;
    static const ::zxdg_output_v1_listener xdgOutputListener = {
        MascotBackendWayland_Output_logical_position,
        MascotBackendWayland_Output_logical_size,
        MascotBackendWayland_Output_xdg_done,
        MascotBackendWayland_Output_xdg_name,
        MascotBackendWayland_Output_xdg_description
    };
    zxdg_output_v1_add_listener(output, &xdgOutputListener, this);
}

WaylandOutput::WaylandOutput(::wl_output *output): m_output(output) {
    static const ::wl_output_listener outputListener = {
        MascotBackendWayland_Output_geometry,
        MascotBackendWayland_Output_mode,
        MascotBackendWayland_Output_done,
        MascotBackendWayland_Output_scale,
        MascotBackendWayland_Output_name,
        MascotBackendWayland_Output_description
    };
    wl_output_add_listener(output, &outputListener, this);
}

WaylandOutput::~WaylandOutput() {
    //wl_output_release(m_output);
}