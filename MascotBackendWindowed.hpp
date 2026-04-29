#pragma once

#include "MascotBackend.hpp"
#include <shijima/mascot/manager.hpp>
#include <memory>
#include <functional>
#include "ActiveMascot.hpp"
#include <QObject>

class QWidget;
class ShijimaManager;

class MascotBackendWindowed : public QObject, public MascotBackend {
public:
    MascotBackendWindowed(ShijimaManager *manager);
    virtual ~MascotBackendWindowed() override;
    virtual ActiveMascot *spawn(MascotData *mascotData,
        std::unique_ptr<shijima::mascot::manager> mascot,
        ActiveMascot *parent, int mascotId, bool resetPosition) override;
    virtual ActiveMascot *migrate(ActiveMascot &old) override;
    virtual void preTick() override;
    virtual void postTick() override;
    virtual void updateEnvironments(
        std::function<void(shijima::mascot::environment &)> cb) override;
    virtual bool eventFilter(QObject *object, QEvent *event) override;
private:
    QWidget *m_sandbox;
    std::shared_ptr<shijima::mascot::environment> m_env;
};
