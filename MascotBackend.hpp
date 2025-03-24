#pragma once
#include <memory>

class ShijimaManager;
class ActiveMascot;
class MascotData;
namespace shijima {
namespace mascot {
    class manager;
}
}

class MascotBackend {
private:
    ShijimaManager *m_manager;
public:
    ShijimaManager *manager();
    MascotBackend(ShijimaManager *manager);
    virtual ~MascotBackend();
    virtual ActiveMascot *spawn(MascotData *mascotData,
        std::unique_ptr<shijima::mascot::manager> mascot,
        int mascotId) = 0;
    virtual ActiveMascot *migrate(ActiveMascot &old) = 0;
    virtual void tick();
    virtual bool multiMonitorAware() = 0;
};
