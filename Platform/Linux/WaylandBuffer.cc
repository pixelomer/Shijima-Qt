#include "WaylandBuffer.hpp"
#include <fcntl.h>
#include <wayland-util.h>
#include <sys/mman.h>
#include <unistd.h>

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
