#pragma once

#include <cstdint>
#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include <wayland-cursor.h>

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