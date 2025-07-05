#include "MascotBackend.hpp"

MascotBackend::MascotBackend(ShijimaManager *manager): m_manager(manager) {}
MascotBackend::~MascotBackend() {}
ShijimaManager *MascotBackend::manager() {
    return m_manager;
}
