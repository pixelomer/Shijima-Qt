#pragma once
#include "MascotBackend.hpp"

class MascotBackendWidgets : public MascotBackend {
public:
    MascotBackendWidgets(ShijimaManager *manager);
    virtual ~MascotBackendWidgets() override;
    virtual ActiveMascot *spawn(MascotData *mascotData,
        std::unique_ptr<shijima::mascot::manager> mascot,
        int mascotId) override;
    virtual ActiveMascot *migrate(ActiveMascot &old) override;
    virtual void tick() override;
    virtual bool multiMonitorAware() override;
};
