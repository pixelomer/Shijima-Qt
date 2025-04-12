#include "WaylandOutput.hpp"

std::ostream &operator<<(std::ostream &lhs, WaylandOutput &rhs) {
    lhs << "<WaylandOutput "
        << "wl_output=" << rhs.output() << " "
        << "name=\"" << rhs.name() << "\" "
        << "description=\"" << rhs.description() << "\" "
        << "make=\"" << rhs.make() << "\" "
        << "model=\"" << rhs.model() << "\" "
        << "width=" << rhs.width() << " "
        << "height=" << rhs.height() << " "
        << "x=" << rhs.x() << " "
        << "y=" << rhs.y() << " "
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

WaylandOutput::WaylandOutput(::wl_output *output): m_output(output) {
    static const ::wl_output_listener listener = {
        MascotBackendWayland_Output_geometry,
        MascotBackendWayland_Output_mode,
        MascotBackendWayland_Output_done,
        MascotBackendWayland_Output_scale,
        MascotBackendWayland_Output_name,
        MascotBackendWayland_Output_description
    };
    wl_output_add_listener(output, &listener, this);
}

WaylandOutput::~WaylandOutput() {
    //wl_output_release(m_output);
}