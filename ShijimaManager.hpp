#pragma once

#include <QMainWindow>
#include <shijima/mascot/manager.hpp>
#include <vector>
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
    int m_timer;
    std::shared_ptr<shijima::mascot::environment> m_env;
    std::vector<ShijimaWidget *> m_mascots;
};