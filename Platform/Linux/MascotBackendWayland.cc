#include "MascotBackendWayland.hpp"
#include "../../ShijimaManager.hpp"
#include <stdexcept>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include <wayland-cursor.h>
#include <syscall.h>
#include <sys/mman.h>
#include <linux/input-event-codes.h>
#include "WaylandEnvironment.hpp"
#include "WaylandShimeji.hpp"
#include "wayland-protocols/wlr-layer-shell-unstable-v1.h"
#include "wayland-protocols/fractional-scale-v1.h"
#include "os-compatibility.hpp"
#include "wayland-protocols/xdg-output-unstable-v1.h"
#include <iostream>

WaylandBuffer MascotBackendWayland::createBuffer(int width, int height) {
    struct wl_shm_pool *pool;
    int stride = width * 4;
    int size = stride * height;

    int fd = os_create_anonymous_file(size);
    if (fd < 0) {
        std::cerr << "Failed to create a buffer. size: " << size << std::endl;
        return {};
    }

    uint8_t *data = (uint8_t *)mmap(NULL, size, PROT_READ | PROT_WRITE,
        MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) {
        std::cerr << "mmap failed!" << std::endl;
        close(fd);
        return {};
    }

    pool = wl_shm_create_pool(m_shm, fd, size);
    ::wl_buffer *buffer = wl_shm_pool_create_buffer(pool, 0,
        width, height, stride, WL_SHM_FORMAT_ARGB8888);
    wl_shm_pool_destroy(pool);

    return { width, height, buffer, data, fd };
}

void MascotBackendWayland_register_global(void *data,
    struct wl_registry *wl_registry,
    uint32_t name,
    const char *interface,
    uint32_t version)
{
    MascotBackendWayland *backend = static_cast<MascotBackendWayland *>(data);
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
        backend->m_outputsByName[name] = output;
        auto outputWrapper = new WaylandOutput { output };
        if (backend->m_xdgOutputManager != NULL) {
            outputWrapper->setXdgOutput(zxdg_output_manager_v1_get_xdg_output(
                backend->m_xdgOutputManager, output));
            backend->m_env[outputWrapper] = std::make_shared<WaylandEnvironment>(
                backend, outputWrapper);
        }
        backend->m_outputs[output] = outputWrapper;
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
    else if (strcmp(interface, zxdg_output_manager_v1_interface.name) == 0) {
        backend->m_xdgOutputManager =
            (::zxdg_output_manager_v1 *)wl_registry_bind(wl_registry, name,
            &zxdg_output_manager_v1_interface, 1);
    }
}

void MascotBackendWayland_deregister_global(void *data,
    struct wl_registry *wl_registry,
    uint32_t name)
{
    (void)wl_registry;
    MascotBackendWayland *backend = static_cast<MascotBackendWayland *>(data);
    if (backend->m_outputsByName.count(name) != 0) {
        auto output = backend->m_outputsByName[name];
        backend->m_outputsByName.erase(name);
        auto outputWrapper = backend->m_outputs[output];
        delete outputWrapper;
        auto env = backend->m_env[outputWrapper];
        env->invalidate();
        backend->m_env.erase(outputWrapper);
    }
    //printf("removed: %u\n", name);
}

void MascotBackendWayland_pointer_enter(void *data,
    struct wl_pointer *wl_pointer,
    uint32_t serial,
    struct wl_surface *surface,
    wl_fixed_t surface_x,
    wl_fixed_t surface_y)
{
    (void)wl_pointer; (void)surface_x; (void)surface_y;
    auto wayland = (MascotBackendWayland *)data;
    if (wayland->m_pointedEnvironment != nullptr) {
        wayland->m_pointedEnvironment->pointerLeave();
    }
    wayland->m_pointedEnvironment = nullptr;
    for (auto &pair : wayland->m_env) {
        if (pair.second->surface() == surface) {
            wayland->m_pointedEnvironment = pair.second;
            break;
        }
    }
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
    (void)wl_pointer; (void)serial;
    auto wayland = (MascotBackendWayland *)data;
    if (wayland->m_pointedEnvironment != nullptr &&
        wayland->m_pointedEnvironment->surface() == surface)
    {
        wayland->m_pointedEnvironment->pointerLeave();
        wayland->m_pointedEnvironment = nullptr;
    }
}

void MascotBackendWayland_pointer_motion(void *data,
    struct wl_pointer *wl_pointer,
    uint32_t time,
    wl_fixed_t surface_x,
    wl_fixed_t surface_y)
{
    (void)wl_pointer; (void)time;
    auto wayland = (MascotBackendWayland *)data;
    auto &env = wayland->m_pointedEnvironment;
    if (env != nullptr) {
        double x = wl_fixed_to_double(surface_x) + wayland->m_cursorOffset.x;
        double y = wl_fixed_to_double(surface_y) + wayland->m_cursorOffset.y;
        env->pointerMove(x, y);
    }
}

void MascotBackendWayland_pointer_axis(void *data,
    struct wl_pointer *wl_pointer,
    uint32_t time,
    uint32_t axis,
    wl_fixed_t value)
{
    // Stub
    (void)data; (void)wl_pointer; (void)time; (void)axis; (void)value;
}

void MascotBackendWayland_pointer_frame(void *data,
    struct wl_pointer *wl_pointer)
{
    // Stub
    (void)data; (void)wl_pointer;
}

void MascotBackendWayland_pointer_axis_source(void *data,
    struct wl_pointer *wl_pointer,
    uint32_t axis_source)
{
    // Stub
    (void)data; (void)wl_pointer; (void)axis_source;
}

void MascotBackendWayland_pointer_axis_stop(void *data,
    struct wl_pointer *wl_pointer,
    uint32_t time,
    uint32_t axis)
{
    // Stub
    (void)data; (void)wl_pointer; (void)time; (void)axis;
}

void MascotBackendWayland_pointer_axis_discrete(void *data,
    struct wl_pointer *wl_pointer,
    uint32_t axis,
    int32_t discrete)
{
    // Stub
    (void)data; (void)wl_pointer; (void)axis; (void)discrete;
}

void MascotBackendWayland_pointer_axis_value120(void *data,
    struct wl_pointer *wl_pointer,
    uint32_t axis,
    int32_t value120)
{
    // Stub
    (void)data; (void)wl_pointer; (void)axis; (void)value120;
}

void MascotBackendWayland_pointer_axis_relative_direction(void *data,
    struct wl_pointer *wl_pointer,
    uint32_t axis,
    uint32_t direction)
{
    // Stub
    (void)data; (void)wl_pointer; (void)axis; (void)direction;
}

std::shared_ptr<WaylandEnvironment> MascotBackendWayland::environmentAt(QPoint point) {
    for (auto &pair : m_env) {
        auto output = pair.first;
        auto &env = pair.second;
        if (output->logicalX() <= point.x() &&
            (output->logicalX() + env->env()->screen.right) > point.x() &&
            output->logicalY() <= point.y() &&
            (output->logicalY() + env->env()->screen.bottom) > point.y())
        {
            return env;
        }
    }
    return {};
}

std::shared_ptr<WaylandEnvironment> MascotBackendWayland::spawnEnvironment() {
    if (m_env.size() == 0) {
        return {};
    }
    auto iter = m_env.begin();
    auto &pair = *iter;
    return pair.second;
}

void MascotBackendWayland_pointer_button(void *data,
    struct wl_pointer *wl_pointer,
    uint32_t serial,
    uint32_t time,
    uint32_t button,
    uint32_t state)
{
    (void)wl_pointer; (void)serial; (void)time;
    auto wayland = (MascotBackendWayland *)data;
    if (wayland->m_pointedEnvironment != nullptr) {
        wayland->m_cursorOffset = { 0, 0 };
        wayland->m_pointedEnvironment->pointerButton(button, state);
    }
}

MascotBackendWayland::MascotBackendWayland(ShijimaManager *manager):
    MascotBackend(manager),
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
        else if (m_xdgOutputManager == NULL) {
            error = "xdg_output_manager not available";
        }
        if (error != NULL) {
            wl_registry_destroy(m_registry);
            throw std::runtime_error(error);
        }
    }

    // attach xdg outputs and build environments
    for (auto &pair : m_outputs) {
        auto output = pair.first;
        auto outputWrapper = pair.second;
        if (outputWrapper->xdgOutput() == NULL) {
            outputWrapper->setXdgOutput(zxdg_output_manager_v1_get_xdg_output(
                m_xdgOutputManager, output));
            m_env[outputWrapper] = std::make_shared<WaylandEnvironment>(
                this, outputWrapper);
        }
    }

    m_cursorTheme = wl_cursor_theme_load(NULL, 24, m_shm);
    m_leftCursor = wl_cursor_theme_get_cursor(m_cursorTheme, "left_ptr");
    m_leftCursorImage = m_leftCursor->images[0];
    m_leftCursorBuffer = wl_cursor_image_get_buffer(m_leftCursorImage);

    static const ::wl_pointer_listener pointer_listener = {
        MascotBackendWayland_pointer_enter,
        MascotBackendWayland_pointer_leave,
        MascotBackendWayland_pointer_motion,
        MascotBackendWayland_pointer_button,
        MascotBackendWayland_pointer_axis,
        #ifdef WL_POINTER_FRAME_SINCE_VERSION
        MascotBackendWayland_pointer_frame,
        MascotBackendWayland_pointer_axis_source,
        MascotBackendWayland_pointer_axis_stop,
        MascotBackendWayland_pointer_axis_discrete,
        #endif
        #ifdef WL_POINTER_AXIS_VALUE120_SINCE_VERSION
        MascotBackendWayland_pointer_axis_value120,
        #endif
        #ifdef WL_POINTER_AXIS_RELATIVE_DIRECTION_SINCE_VERSION
        MascotBackendWayland_pointer_axis_relative_direction,
        #endif
    };
    
    m_pointer = wl_seat_get_pointer(m_seat);
    wl_pointer_add_listener(m_pointer, &pointer_listener, this);
    m_pointerSurface = wl_compositor_create_surface(m_compositor);
    wl_surface_attach(m_pointerSurface, m_leftCursorBuffer, 0, 0);
    wl_surface_commit(m_pointerSurface);
}

void MascotBackendWayland::preTick() {
    wl_display_dispatch_pending(m_display);
    for (auto &pair : m_env) {
        pair.second->preTick();
    }
}

void MascotBackendWayland::postTick() {
    for (auto &pair : m_env) {
        pair.second->postTick();
    }
    wl_display_roundtrip(m_display);
}

void MascotBackendWayland::updateEnvironments(
    std::function<void(shijima::mascot::environment &)> cb)
{
    for (auto &pair : m_env) {
        cb(*pair.second->env());
    }
}

MascotBackendWayland::~MascotBackendWayland() {
    m_outputsByName.clear();
    for (auto &pair : m_outputs) {
        delete pair.second;
    }
    m_outputs.clear();
    for (auto &pair : m_env) {
        pair.second->invalidate();
    }
    m_env.clear();
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
    ActiveMascot *parent, int mascotId, bool resetPosition)
{
    std::shared_ptr<WaylandEnvironment> env;
    if (parent != nullptr) {
        auto waylandParent = dynamic_cast<WaylandShimeji *>(parent);
        env = waylandParent->waylandEnv();
    }
    else {
        env = spawnEnvironment();
    }
    mascot->state->env = env->env();
    if (resetPosition) {
        env->preTick();
        mascot->reset_position();
        env->postTick();
    }
    auto shimeji = new WaylandShimeji(mascotData,
        std::move(mascot), mascotId, env);
    return shimeji;
}

ActiveMascot *MascotBackendWayland::migrate(ActiveMascot &old) {
    auto env = spawnEnvironment();
    env->preTick();
    old.mascot().state->env = env->env();
    old.mascot().reset_position();
    env->postTick();
    auto shimeji = new WaylandShimeji(old, env);
    return shimeji;
}

bool MascotBackendWayland::reassignEnvironment(WaylandShimeji *shimeji) {
    auto &mascot = shimeji->mascot();
    if (mascot.state->dragging) {
        // move to monitor with cursor
        auto cursor = mascot.state->env->cursor;
        auto oldEnv = shimeji->waylandEnv();
        cursor.x += oldEnv->output()->logicalX();
        cursor.y += oldEnv->output()->logicalY();
        auto env = environmentAt({ (int)cursor.x, (int)cursor.y });
        if (env != nullptr && env != oldEnv) {
            // make this environment this mascot's new owner
            cursor.x -= env->output()->logicalX();
            cursor.y -= env->output()->logicalY();
            env->env()->cursor = cursor;
            shimeji->setEnvironment(env);

            // this new environment needs to start receiving events now
            if (m_pointedEnvironment != nullptr) {
                m_pointedEnvironment->pointerLeave();
            }
            m_pointedEnvironment = env;
            m_pointedEnvironment->pointerEnter();
            m_pointedEnvironment->pointerButton(shimeji,
                BTN_LEFT, WL_POINTER_BUTTON_STATE_PRESSED);
            
            // the old surface will continue receiving move events
            // apply an offset to these events
            m_cursorOffset.y += oldEnv->output()->logicalY() - env->output()->logicalY();
            m_cursorOffset.x += oldEnv->output()->logicalX() - env->output()->logicalX();
                
            return true;
        }
        return false;
    }
    else {
        // move to default monitor
        auto env = spawnEnvironment();
        shimeji->setEnvironment(env);
        return true;
    }
}
