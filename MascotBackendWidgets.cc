#include "MascotBackendWidgets.hpp"
#include "ShijimaWidget.hpp"
#include "ShijimaManager.hpp"

MascotBackendWidgets::MascotBackendWidgets(ShijimaManager *manager):
    MascotBackend(manager) {}
MascotBackendWidgets::~MascotBackendWidgets() {}

ActiveMascot *MascotBackendWidgets::spawn(MascotData *mascotData,
    std::unique_ptr<shijima::mascot::manager> mascot,
    int mascotId)
{
    return new ShijimaWidget(mascotData, std::move(mascot),
        mascotId, manager());
}

ActiveMascot *MascotBackendWidgets::migrate(ActiveMascot &old) {
    return new ShijimaWidget(old, manager());
}

void MascotBackendWidgets::tick() {
    
}

bool MascotBackendWidgets::multiMonitorAware() {
    return true;
}
