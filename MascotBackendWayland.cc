#include "MascotBackendWayland.hpp"
#include "ShijimaManager.hpp"
#include <stdexcept>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include <wayland-cursor.h>
#include <syscall.h>
#include <sys/mman.h>
#include <linux/input-event-codes.h>
#include "WaylandShimeji.hpp"
#include "qnamespace.h"
#include "wayland-protocols/wlr-layer-shell.h"
#include "wayland-protocols/tablet-v2.h"
#include "wayland-protocols/xdg-shell.h"
#include "wayland-protocols/fractional-scale-v1.h"
#include "wayland-protocols/cursor-shape-v1.h"
#include "wayland-protocols/viewporter.h"
#include "wayland-protocols/xdg-output.h"
#include "wayland-protocols/wlr-layer-shell.c"
#include "wayland-protocols/tablet-v2.c"
#include "wayland-protocols/xdg-shell.c"
#include "wayland-protocols/fractional-scale-v1.c"
#include "wayland-protocols/cursor-shape-v1.c"
#include "wayland-protocols/viewporter.c"
#include "wayland-protocols/xdg-output.c"
#include <fcntl.h>
#include <wayland-util.h>
#include "os-compatibility.cc"

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

WaylandBuffer MascotBackendWayland::createBuffer(int width, int height) {
    struct wl_shm_pool *pool;
    int stride = width * 4; // 4 bytes per pixel in our ARGB8888 format.
    int size = stride * height;

    int fd = os_create_anonymous_file(size);
    if (fd < 0) {
        fprintf(stderr, "Failed to create a buffer. size: %d\n", size);
        return {};
    }

    uint8_t *data = (uint8_t *)mmap(NULL, size, PROT_READ | PROT_WRITE,
        MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) {
        fprintf(stderr, "mmap failed!\n");
        close(fd);
        return {};
    }

    pool = wl_shm_create_pool(m_shm, fd, size);
    ::wl_buffer *buffer = wl_shm_pool_create_buffer(pool, 0,
        width, height, stride, WL_SHM_FORMAT_ARGB8888);
    wl_shm_pool_destroy(pool);

    return { width, height, buffer, data, fd };
}

WaylandBuffer::WaylandBuffer(): m_valid(false) {}

WaylandBuffer::WaylandBuffer(int width, int height, ::wl_buffer *buffer,
    uint8_t *data, int fd): m_width(width), m_height(height),
    m_valid(true), m_buffer(buffer), m_data(data), m_fd(fd) {}

void WaylandBuffer::destroy() {
    if (m_valid) {
        wl_buffer_destroy(m_buffer);
        munmap(m_data, size());
        close(m_fd);
        m_valid = false;
    }
}

WaylandBuffer &WaylandBuffer::operator=(WaylandBuffer &&rhs) {
    if (this != &rhs) {
        destroy();
        m_buffer = rhs.m_buffer;
        m_data = rhs.m_data;
        m_fd = rhs.m_fd;
        m_width = rhs.m_width;
        m_height = rhs.m_height;
        m_valid = true;
        rhs.m_valid = false;
    }
    return *this;
}

WaylandBuffer::WaylandBuffer(WaylandBuffer &&rhs) {
    *this = std::move(rhs);
}

WaylandBuffer::~WaylandBuffer() {
    destroy();
}

void MascotBackendWayland_register_global(void *data,
    struct wl_registry *wl_registry,
    uint32_t name,
    const char *interface,
    uint32_t version)
{
    MascotBackendWayland *backend = static_cast<MascotBackendWayland *>(data);
    printf("interface: '%s', version: %u, name: %u\n", interface, version, name);
    if (strcmp(interface, wl_compositor_interface.name) == 0 &&
        version >= 4)
    {
		backend->m_compositor = (::wl_compositor *)wl_registry_bind(wl_registry, name,
			&wl_compositor_interface, 6);
	}
    else if (strcmp(interface, wl_shm_interface.name) == 0) {
		backend->m_shm = (::wl_shm *)wl_registry_bind(wl_registry, name,
			&wl_shm_interface, 1);
	}
    else if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
		backend->m_layer_shell = (::zwlr_layer_shell_v1 *)wl_registry_bind(wl_registry,
            name, &zwlr_layer_shell_v1_interface,
            version < 4 ? version : 4);
	}
    else if (strcmp(interface, wl_output_interface.name) == 0) {
        ::wl_output *output = (::wl_output *)wl_registry_bind(wl_registry, name,
            &wl_output_interface, 1);
        backend->m_outputs[output] = new WaylandOutput { output };
	}
    else if (strcmp(interface, wl_subcompositor_interface.name) == 0) {
		backend->m_subcompositor = (::wl_subcompositor *)wl_registry_bind(wl_registry, name,
			&wl_subcompositor_interface, 1);
	}
    else if (strcmp(interface, wl_seat_interface.name) == 0) {
		backend->m_seat = (::wl_seat *)wl_registry_bind(wl_registry, name,
			&wl_seat_interface, 1);
	}
    else if (strcmp(interface, wp_fractional_scale_manager_v1_interface.name) == 0) {
        backend->m_fractionalScaleManager = 
            (::wp_fractional_scale_manager_v1 *)wl_registry_bind(wl_registry, name,
            &wp_fractional_scale_manager_v1_interface, 1);
    }
}

void MascotBackendWayland_deregister_global(void *data,
    struct wl_registry *wl_registry,
    uint32_t name)
{
    MascotBackendWayland *backend = static_cast<MascotBackendWayland *>(data);
    printf("removed: %u\n", name);
}

void MascotBackendWayland_layer_surface_configure(void *data,
    struct zwlr_layer_surface_v1 *zwlr_layer_surface_v1,
    uint32_t serial,
    uint32_t width,
    uint32_t height)
{
    printf("layer-surface width=%u height=%u\n",
        width, height);
    zwlr_layer_surface_v1_ack_configure(zwlr_layer_surface_v1, serial);
}

void MascotBackendWayland_layer_surface_closed(void *data,
    struct zwlr_layer_surface_v1 *zwlr_layer_surface_v1)
{
    printf("layer-surface closed\n");
}

void MascotBackendWayland_pointer_enter(void *data,
    struct wl_pointer *wl_pointer,
    uint32_t serial,
    struct wl_surface *surface,
    wl_fixed_t surface_x,
    wl_fixed_t surface_y)
{
    auto wayland = (MascotBackendWayland *)data;
    wl_pointer_set_cursor(wayland->m_pointer, serial,
        wayland->m_pointerSurface,
        wayland->m_leftCursorImage->hotspot_x,
        wayland->m_leftCursorImage->hotspot_y);
}

void MascotBackendWayland_pointer_leave(void *data,
    struct wl_pointer *wl_pointer,
    uint32_t serial,
    struct wl_surface *surface)
{
    auto wayland = (MascotBackendWayland *)data;
    if (wayland->m_activeMouseListener != nullptr) {
        wayland->m_activeMouseListener->mouseUp(Qt::MouseButton::LeftButton);
        wayland->m_leftMouseDown = false;
        wayland->m_activeMouseListener = nullptr;
        wayland->m_regionValid = false;
    }
}

void MascotBackendWayland_pointer_motion(void *data,
    struct wl_pointer *wl_pointer,
    uint32_t time,
    wl_fixed_t surface_x,
    wl_fixed_t surface_y)
{
    double x = wl_fixed_to_double(surface_x);
    double y = wl_fixed_to_double(surface_y);
    auto wayland = (MascotBackendWayland *)data;
    wayland->m_cursorPosition = { x, y };
    wayland->m_env->cursor.move({ x, y });
    if (wayland->m_activeMouseListener != nullptr) {
        wayland->m_activeMouseListener->mouseMove(wayland->m_cursorPosition);
    }
    else if (!wayland->m_clients.empty()) {
        auto client = *wayland->m_clients.begin();
        client->mouseMove(wayland->m_cursorPosition);
    }
}

void MascotBackendWayland_pointer_button(void *data,
    struct wl_pointer *wl_pointer,
    uint32_t serial,
    uint32_t time,
    uint32_t button,
    uint32_t state)
{
    auto wayland = (MascotBackendWayland *)data;
    WaylandClient *client = nullptr;
    bool down = (state == WL_POINTER_BUTTON_STATE_PRESSED);
    if (down) {
        for (auto option : wayland->m_clients) {
            if (option->pointInside(wayland->m_cursorPosition)) {
                client = option;
                break;
            }
        }
        if (client == nullptr) {
            std::cerr << "warning: found no client at this point?" << std::endl;
            return;
        }
    }
    Qt::MouseButton qtButton;
    if (button == BTN_RIGHT) {
        qtButton = Qt::MouseButton::RightButton;
    }
    else {
        qtButton = Qt::MouseButton::LeftButton;
    }
    if (qtButton == Qt::MouseButton::LeftButton) {
        if (down) {
            wl_region_add(wayland->m_layerRegion, 0, 0, 
                wayland->m_activeOutput->width(),
                wayland->m_activeOutput->height());
            wayland->m_leftMouseDown = true;
            wl_surface_set_input_region(wayland->m_surface,
                wayland->m_layerRegion);
            wl_surface_commit(wayland->m_surface);
            wayland->m_regionValid = false;
        }
        else {
            wayland->m_regionValid = false;
            wayland->m_leftMouseDown = false;
        }
    }
    if (down) {
        client->mouseDown(qtButton);
        if (qtButton == Qt::MouseButton::LeftButton) {
            wayland->m_activeMouseListener = client;
            wayland->m_regionValid = false;
        }
    }
    else {
        if (wayland->m_activeMouseListener != nullptr) {
            wayland->m_activeMouseListener->mouseUp(qtButton);
            wayland->m_regionValid = false;
            wayland->m_activeMouseListener = nullptr;
        }
    }
}

void MascotBackendWayland_preferred_scale(void *data,
    struct wp_fractional_scale_v1 *wp_fractional_scale_v1,
    uint32_t scale)
{
    auto wayland = (MascotBackendWayland *)data;
    wayland->m_scaleFactor = scale / 120.0;
}

MascotBackendWayland::MascotBackendWayland(ShijimaManager *manager):
    MascotBackend(manager),
    m_env(std::make_shared<shijima::mascot::environment>()),
    m_scaleFactor(1), m_nullRegionRequested(false)
{
    // connect to compositor
    m_display = wl_display_connect(NULL);
    if (m_display == NULL) {
        throw std::runtime_error("wl_display_connect() failed");
    }
    m_registry = wl_display_get_registry(m_display);
    static const ::wl_registry_listener registry_listener = {
        MascotBackendWayland_register_global,
        MascotBackendWayland_deregister_global
    };
    wl_registry_add_listener(m_registry,
        &registry_listener, this);

    // find displays, interfaces
    wl_display_roundtrip(m_display);

    // get display data
    wl_display_roundtrip(m_display);

    // check interfaces
    {
        const char *error = NULL;
        if (m_compositor == NULL) {
            error = "wl_compositor not available";
        }
        else if (m_shm == NULL) {
            error = "wl_shm not available";
        }
        else if (m_layer_shell == NULL) {
            error = "zwlr_layer_shell not available";
        }
        else if (m_subcompositor == NULL) {
            error = "wl_subcompositor not available";
        }
        else if (m_seat == NULL) {
            error = "wl_seat not available";
        }
        else if (m_fractionalScaleManager == NULL) {
            error = "wp_fractional_scale_manager_v1 not available";
        }
        if (error != NULL) {
            wl_registry_destroy(m_registry);
            throw std::runtime_error(error);
        }
    }

    static const ::wp_fractional_scale_v1_listener fractional_scale_listener = {
        MascotBackendWayland_preferred_scale
    };

    // create surface
    m_surface = wl_compositor_create_surface(m_compositor);
    wl_surface_commit(m_surface);
    m_fractionalScale = wp_fractional_scale_manager_v1_get_fractional_scale(
        m_fractionalScaleManager, m_surface);
    wp_fractional_scale_v1_add_listener(m_fractionalScale, 
        &fractional_scale_listener, this);
    wl_display_roundtrip(m_display);
    WaylandOutput *firstOutput;
    for (auto &entry : m_outputs) {
        firstOutput = entry.second;
        break;
    }
    m_activeOutput = firstOutput;
    m_layer_surface = zwlr_layer_shell_v1_get_layer_surface(
        m_layer_shell, m_surface, firstOutput->output(),
        ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY, "ShijimaQt");
    zwlr_layer_surface_v1_set_size(m_layer_surface,
        firstOutput->width(), firstOutput->height());
    zwlr_layer_surface_v1_set_anchor(m_layer_surface,
        ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP |
        ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT);
    zwlr_layer_surface_v1_set_exclusive_zone(m_layer_surface,
        -1);
    zwlr_layer_surface_v1_set_margin(m_layer_surface,
        0, 0, 0, 0);
    
    static ::zwlr_layer_surface_v1_listener layer_surface_listener = {
        MascotBackendWayland_layer_surface_configure,
        MascotBackendWayland_layer_surface_closed
    };
    
    zwlr_layer_surface_v1_set_keyboard_interactivity(
        m_layer_surface,
        ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_NONE);
    zwlr_layer_surface_v1_add_listener(m_layer_surface,
        &layer_surface_listener, m_surface);
    wl_surface_commit(m_surface);
    m_layerRegion = wl_compositor_create_region(m_compositor);
    wl_surface_set_input_region(m_surface, m_layerRegion);
    wl_display_roundtrip(m_display);

    m_layerBuffer = createBuffer(
        firstOutput->width() * m_scaleFactor,
        firstOutput->height() * m_scaleFactor);
    for (size_t i=0; i<m_layerBuffer.size(); i+=4) {
        m_layerBuffer[i] = 0x10;
        m_layerBuffer[i+1] = 0x10;
        m_layerBuffer[i+2] = 0x10;
        m_layerBuffer[i+3] = 0x10;
    }

    wl_surface_attach(m_surface, m_layerBuffer, 0, 0);
    wl_surface_commit(m_surface);

    m_cursorTheme = wl_cursor_theme_load(NULL, 24, m_shm);
    m_leftCursor = wl_cursor_theme_get_cursor(m_cursorTheme, "left_ptr");
    m_leftCursorImage = m_leftCursor->images[0];
    m_leftCursorBuffer = wl_cursor_image_get_buffer(m_leftCursorImage);

    static const ::wl_pointer_listener pointer_listener = {
        MascotBackendWayland_pointer_enter,
        MascotBackendWayland_pointer_leave,
        MascotBackendWayland_pointer_motion,
        MascotBackendWayland_pointer_button,
        NULL, NULL, NULL,
        NULL, NULL, NULL,
        NULL
    };
    m_pointer = wl_seat_get_pointer(m_seat);
    wl_pointer_add_listener(m_pointer, &pointer_listener, this);
    m_pointerSurface = wl_compositor_create_surface(m_compositor);
    wl_surface_attach(m_pointerSurface, m_leftCursorBuffer, 0, 0);
    wl_surface_commit(m_pointerSurface);
}

void MascotBackendWayland::preTick() {
    initEnvironment();
}

void MascotBackendWayland::initEnvironment() {
    auto &env = *m_env;
    env.screen = { (double)m_activeOutput->top(), (double)m_activeOutput->right(),
        (double)m_activeOutput->bottom(), (double)m_activeOutput->left() };
    env.screen /= m_scaleFactor;
    env.work_area = env.screen;
    env.floor = { env.screen.bottom, env.screen.left, env.screen.right };
    env.ceiling = { env.screen.top, env.screen.left, env.screen.right };
    manager()->applyActiveIE(*m_env);
    env.subtick_count = manager()->subtickCount();
    env.set_scale(1.0 / std::sqrt(manager()->userScale()));
}

void MascotBackendWayland::finalizeEnvironment() {
    m_env->cursor.dx = m_env->cursor.dy = 0;
    m_env->reset_scale();
}


void MascotBackendWayland::postTick() {
    if (m_nullRegionRequested) {
        wl_region_subtract(m_layerRegion, 0, 0, INT32_MAX, INT32_MAX);
        wl_surface_set_input_region(m_surface, m_layerRegion);
        wl_surface_commit(m_surface);
        m_regionValid = false;
        m_nullRegionRequested = false;
    }
    else if (!m_regionValid) {
        wl_region_subtract(m_layerRegion, 0, 0, INT32_MAX, INT32_MAX);
        if (m_leftMouseDown) {
            wl_region_add(m_layerRegion, 0, 0, m_activeOutput->width(),
                m_activeOutput->height());
        }
        else {
            for (auto client : m_clients) {
                client->updateRegion(m_layerRegion);
            }
        }
        m_regionValid = true;
        wl_surface_set_input_region(m_surface, m_layerRegion);
        wl_surface_commit(m_surface);
    }
    finalizeEnvironment();
    wl_display_roundtrip(m_display);
    wl_display_dispatch_pending(m_display);
}

void MascotBackendWayland::updateEnvironments(
    std::function<void(shijima::mascot::environment &)> cb)
{
    cb(*m_env);
}

MascotBackendWayland::~MascotBackendWayland() {
    wl_surface_destroy(m_surface);
    wl_surface_destroy(m_pointerSurface);
    //wl_buffer_destroy(m_leftCursorBuffer);
    wl_cursor_theme_destroy(m_cursorTheme);
    m_layerBuffer.destroy();
    wl_registry_destroy(m_registry);
    wl_display_roundtrip(m_display);
    wl_display_disconnect(m_display);
}

ActiveMascot *MascotBackendWayland::spawn(MascotData *mascotData,
    std::unique_ptr<shijima::mascot::manager> mascot,
    int mascotId)
{
    initEnvironment();
    mascot->state->env = m_env;
    mascot->reset_position();
    finalizeEnvironment();
    auto shimeji = new WaylandShimeji(mascotData, std::move(mascot), mascotId,
        this);
    return shimeji;
}

ActiveMascot *MascotBackendWayland::migrate(ActiveMascot &old) {
    initEnvironment();
    old.mascot().state->env = m_env;
    old.mascot().reset_position();
    finalizeEnvironment();
    auto shimeji = new WaylandShimeji(old, this);
    return shimeji;
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
    auto *output = (WaylandOutput *)data;
    //output->ready = true;
    printf("output %p: done\n", (void *)wl_output);
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

void MascotBackendWayland::addClient(WaylandClient *client) {
    m_clients.push_front(client);
}

void MascotBackendWayland::removeClient(WaylandClient *client) {
    m_clients.remove(client);
}
