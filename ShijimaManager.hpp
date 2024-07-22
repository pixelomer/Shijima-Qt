#pragma once

#include <QMainWindow>
#include <shijima/mascot/manager.hpp>
#include <vector>
#include "Platform/Platform.hpp"
#include "ShijimaWidget.hpp"

class QVBoxLayout;
class QWidget;

class ShijimaManager : public QMainWindow
{
public:
    static ShijimaManager *defaultManager();
    void updateEnvironment();
protected:
    void timerEvent(QTimerEvent *event) override;
private:
    explicit ShijimaManager(QWidget *parent = nullptr);
    void spawnClicked();
    void tick();
    Platform::ActiveWindow m_activeWindow;
    Platform::ActiveWindowObserver m_windowObserver;
    int m_mascotTimer = -1;
    int m_windowObserverTimer = -1;
    std::shared_ptr<shijima::mascot::environment> m_env;
    std::vector<ShijimaWidget *> m_mascots;
};