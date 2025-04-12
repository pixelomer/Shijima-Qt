#pragma once
#include <memory>
#include <functional>

class ShijimaManager;
class ActiveMascot;
class MascotData;
namespace shijima {
namespace mascot {
    class manager;
    class environment;
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
        ActiveMascot *parent, int mascotId, bool resetPosition) = 0;
    virtual ActiveMascot *migrate(ActiveMascot &old) = 0;
    virtual void preTick() = 0;
    virtual void postTick() = 0;
    virtual void updateEnvironments(
        std::function<void(shijima::mascot::environment &)> cb) = 0;
};
