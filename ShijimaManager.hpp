#pragma once

#include <QMainWindow>
#include <QString>
#include <shijima/mascot/manager.hpp>
#include <shijima/mascot/factory.hpp>
#include <vector>
#include <QMap>
#include "MascotData.hpp"
#include <set>
#include "Platform/ActiveWindowObserver.hpp"
#include "ShijimaWidget.hpp"

class QVBoxLayout;
class QWidget;

class ShijimaManager : public QMainWindow
{
public:
    static ShijimaManager *defaultManager();
    static void finalize();
    void updateEnvironment();
    QString const& mascotsPath();
    void spawn(std::string const& name);
    void killAll();
    void killAll(QString const& name);
    void killAllButOne(ShijimaWidget *widget);
    void killAllButOne(QString const& name);
    void importOnShow(QString const& path);
    ShijimaWidget *hitTest(QPoint const& screenPos);
protected:
    void timerEvent(QTimerEvent *event) override;
    void showEvent(QShowEvent *event) override;
private:
    explicit ShijimaManager(QWidget *parent = nullptr);
    void spawnClicked();
    void reloadMascot(QString const& name);
    void reloadMascots(std::set<std::string> const& mascots);
    void loadAllMascots();
    std::set<std::string> import(QString const& path) noexcept;
    void importWithDialog(QString const& path);
    void tick();
    Platform::ActiveWindow m_previousWindow;
    Platform::ActiveWindow m_currentWindow;
    Platform::ActiveWindowObserver m_windowObserver;
    int m_mascotTimer = -1;
    int m_windowObserverTimer = -1;
    QMap<QString, MascotData> m_loadedMascots;
    std::shared_ptr<shijima::mascot::environment> m_env;
    shijima::mascot::factory m_factory;
    QString m_importOnShowPath;
    std::vector<ShijimaWidget *> m_mascots;
    QString m_mascotsPath;
};