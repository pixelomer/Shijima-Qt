#include "WaylandClient.hpp"
#include "WaylandEnvironment.hpp"
#include "MascotBackendWayland.hpp"
#include "../../ShijimaManager.hpp"
#include "WaylandClient.hpp"
#include "wayland-protocols/fractional-scale-v1.h"
#include "wayland-protocols/wlr-layer-shell-unstable-v1.h"
#include <linux/input-event-codes.h>
#include <wayland-client-protocol.h>

void WaylandEnvironment_preferred_scale(void *data,
    struct wp_fractional_scale_v1 *wp_fractional_scale_v1,
    uint32_t scale)
{
    (void)wp_fractional_scale_v1;
    auto env = (WaylandEnvironment *)data;
    env->m_scaleFactor = scale / 120.0;
}

void WaylandEnvironment_layer_surface_configure(void *data,
    struct zwlr_layer_surface_v1 *zwlr_layer_surface_v1,
    uint32_t serial,
    uint32_t width,
    uint32_t height)
{
    (void)data;
    zwlr_layer_surface_v1_ack_configure(zwlr_layer_surface_v1, serial);
}

void WaylandEnvironment_layer_surface_closed(void *data,
    struct zwlr_layer_surface_v1 *zwlr_layer_surface_v1)
{
    (void)data; (void)zwlr_layer_surface_v1;
}

WaylandEnvironment::WaylandEnvironment(MascotBackendWayland *backend,
    WaylandOutput *output): m_env(std::make_shared<shijima::mascot::environment>()),
    m_output(output), m_backend(backend), m_valid(true), m_activeDragTarget(nullptr)
{
    static const ::wp_fractional_scale_v1_listener fractional_scale_listener = {
        WaylandEnvironment_preferred_scale
    };

    m_surface = wl_compositor_create_surface(m_backend->compositor());
    wl_surface_commit(m_surface);
    m_fractionalScale = wp_fractional_scale_manager_v1_get_fractional_scale(
        m_backend->fractionalScaleManager(), m_surface);
    wp_fractional_scale_v1_add_listener(m_fractionalScale, 
        &fractional_scale_listener, this);
    wl_display_roundtrip(m_backend->display());
    m_layerSurface = zwlr_layer_shell_v1_get_layer_surface(
        m_backend->layerShell(), m_surface,
        m_output->output(), ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY,
        "ShijimaQt");
    zwlr_layer_surface_v1_set_size(m_layerSurface,
        output->logicalWidth(), output->logicalHeight());
    zwlr_layer_surface_v1_set_anchor(m_layerSurface,
        ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP |
        ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT);
    zwlr_layer_surface_v1_set_exclusive_zone(m_layerSurface,
        -1);
    zwlr_layer_surface_v1_set_margin(m_layerSurface,
        0, 0, 0, 0);
    
    static ::zwlr_layer_surface_v1_listener layer_surface_listener = {
        WaylandEnvironment_layer_surface_configure,
        WaylandEnvironment_layer_surface_closed
    };
    
    zwlr_layer_surface_v1_set_keyboard_interactivity(
        m_layerSurface,
        ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_NONE);
    zwlr_layer_surface_v1_add_listener(m_layerSurface,
        &layer_surface_listener, m_surface);
    wl_surface_commit(m_surface);
    m_layerRegion = wl_compositor_create_region(m_backend->compositor());
    wl_surface_set_input_region(m_surface, m_layerRegion);
    wl_display_roundtrip(m_backend->display());

    m_layerBuffer = m_backend->createBuffer(output->logicalWidth(),
        output->logicalHeight());

    wl_surface_attach(m_surface, m_layerBuffer, 0, 0);
    wl_surface_commit(m_surface);
}

void WaylandEnvironment::initEnvironment() {
    auto &env = *m_env;
    env.screen = { 0, (double)m_output->logicalWidth() * m_scaleFactor,
        (double)m_output->logicalHeight() * m_scaleFactor, 0 };
    env.screen /= m_scaleFactor;
    env.work_area = env.screen;
    env.floor = { env.screen.bottom, env.screen.left, env.screen.right };
    env.ceiling = { env.screen.top, env.screen.left, env.screen.right };
    auto manager = m_backend->manager();
    manager->applyActiveIE(*m_env);
    env.subtick_count = manager->subtickCount();
    env.set_scale(1.0 / std::sqrt(manager->userScale()));
}

void WaylandEnvironment::finalizeEnvironment() {
    m_env->cursor.dx = m_env->cursor.dy = 0;
    m_env->reset_scale();
}

void WaylandEnvironment::pointerMove(double x, double y) {
    m_env->cursor.move({ x, y });
    if (m_activeDragTarget != nullptr) {
        m_activeDragTarget->mouseMove();
    }
}

void WaylandEnvironment::releaseDragTarget() {
    if (m_activeDragTarget != nullptr) {
        m_activeDragTarget->mouseUp(Qt::MouseButton::LeftButton);
        m_activeDragTarget = nullptr;
        m_regionValid = false;
    }
}

void WaylandEnvironment::pointerEnter() {
}

void WaylandEnvironment::pointerLeave() {
    releaseDragTarget();
}

void WaylandEnvironment::pointerButton(WaylandClient *client, uint32_t button,
    uint32_t state)
{
    releaseDragTarget();
    bool down = (state == WL_POINTER_BUTTON_STATE_PRESSED);
    Qt::MouseButton qtButton;
    if (button == BTN_RIGHT) {
        qtButton = Qt::MouseButton::RightButton;
    }
    else {
        qtButton = Qt::MouseButton::LeftButton;
    }
    if (down) {
        if (qtButton == Qt::MouseButton::LeftButton) {
            m_activeDragTarget = client;
            m_regionValid = false;
            updateRegion();
        }
        client->mouseDown(qtButton);
    }
    else {
        client->mouseUp(qtButton);
    }
}

void WaylandEnvironment::pointerButton(uint32_t button, uint32_t state) {
    releaseDragTarget();
    bool down = (state == WL_POINTER_BUTTON_STATE_PRESSED);
    WaylandClient *client = nullptr;
    for (auto option : m_clients) {
        if (option->pointInside({ m_env->cursor.x, m_env->cursor.y })) {
            client = option;
            break;
        }
    }
    if (client == nullptr) {
        if (down) {
            std::cerr << "warning: found no client at this point?" << std::endl;
        }
        return;
    }
    pointerButton(client, button, state);
}

void WaylandEnvironment::preTick() {
    initEnvironment();
}

void WaylandEnvironment::updateRegion() {
    wl_region_subtract(m_layerRegion, 0, 0, INT32_MAX, INT32_MAX);
    if (m_nullRegionRequested) {
        m_regionValid = false;
        m_nullRegionRequested = false;
    }
    else if (!m_regionValid) {
        if (m_activeDragTarget != nullptr) {
            wl_region_add(m_layerRegion, 0, 0,
                m_output->logicalWidth() * m_scaleFactor,
                m_output->logicalHeight() * m_scaleFactor);
        }
        else {
            for (auto client : m_clients) {
                client->updateRegion(m_layerRegion);
            }
        }
        m_regionValid = true;
    }
    wl_surface_set_input_region(m_surface, m_layerRegion);
    wl_surface_commit(m_surface);
}

void WaylandEnvironment::postTick() {
    finalizeEnvironment();
    updateRegion();
}

void WaylandEnvironment::addClient(WaylandClient *client) {
    m_clients.push_front(client);
}

void WaylandEnvironment::removeClient(WaylandClient *client) {
    if (client == m_activeDragTarget) {
        releaseDragTarget();
    }
    m_clients.remove(client);
}

void WaylandEnvironment::destroy() {
    wl_region_destroy(m_layerRegion);
    zwlr_layer_surface_v1_destroy(m_layerSurface);
    wp_fractional_scale_v1_destroy(m_fractionalScale);
    wl_surface_destroy(m_surface);
    m_layerBuffer.destroy();
    m_valid = false;
}

WaylandEnvironment::~WaylandEnvironment() {
    destroy();
}
