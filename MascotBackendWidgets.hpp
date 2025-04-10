#pragma once
#include "MascotBackend.hpp"
#include <QObject>
#include <QMap>

class QScreen;

class MascotBackendWidgets : public MascotBackend, public QObject {
public:
    MascotBackendWidgets(ShijimaManager *manager);
    virtual ~MascotBackendWidgets() override;
    virtual ActiveMascot *spawn(MascotData *mascotData,
        std::unique_ptr<shijima::mascot::manager> mascot,
        int mascotId, bool resetPosition) override;
    virtual ActiveMascot *migrate(ActiveMascot &old) override;
    virtual void preTick() override;
    virtual void postTick() override;
    virtual void updateEnvironments(
        std::function<void(shijima::mascot::environment &)> cb) override;
    QMap<QScreen *, std::shared_ptr<shijima::mascot::environment>> const& env();
    QMap<shijima::mascot::environment *, QScreen *> const& reverseEnv();
    QScreen *spawnScreen();
    std::shared_ptr<shijima::mascot::environment> spawnEnv();
private:
    void screenAdded(QScreen *);
    void screenRemoved(QScreen *);
    void updateEnvironment(QScreen *screen);

    QMap<QScreen *, std::shared_ptr<shijima::mascot::environment>> m_env;
    QMap<shijima::mascot::environment *, QScreen *> m_reverseEnv;
};
