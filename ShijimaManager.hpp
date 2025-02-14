#pragma once

#include <QMainWindow>
#include <QString>
#include <shijima/mascot/manager.hpp>
#include <shijima/mascot/factory.hpp>
#include <vector>
#include <QMap>
#include <QListWidgetItem>
#include <QListWidget>
#include <QSettings>
#include "PlatformWidget.hpp"
#include "MascotData.hpp"
#include <set>
#include "Platform/ActiveWindowObserver.hpp"
#include "ShijimaWidget.hpp"

class QVBoxLayout;
class QWidget;

class ShijimaManager : public PlatformWidget<QMainWindow>
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
    void setManagerVisible(bool visible);
    void importOnShow(QString const& path);
    ShijimaWidget *hitTest(QPoint const& screenPos);
protected:
    void timerEvent(QTimerEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
private:
    explicit ShijimaManager(QWidget *parent = nullptr);
    static std::string imgRootForTemplatePath(std::string const& path);
    void loadDefaultMascot();
    void loadData(MascotData const& data);
    void spawnClicked();
    void reloadMascot(QString const& name);
    void askClose();
    void itemDoubleClicked(QListWidgetItem *qItem);
    void reloadMascots(std::set<std::string> const& mascots);
    void loadAllMascots();
    void refreshListWidget();
    void buildToolbar();
    void importAction();
    void deleteAction();
    void quitAction();
    std::set<std::string> import(QString const& path) noexcept;
    void importWithDialog(QList<QString> const& paths);
    void tick();
    QSettings m_settings;
    Platform::ActiveWindow m_previousWindow;
    Platform::ActiveWindow m_currentWindow;
    Platform::ActiveWindowObserver m_windowObserver;
    int m_mascotTimer = -1;
    bool m_allowClose = false;
    bool m_firstShow = true;
    bool m_wasVisible = false;
    double m_userScale = 1.0;
    int m_windowObserverTimer = -1;
    QMap<QString, MascotData> m_loadedMascots;
    QSet<QString> m_listItemsToRefresh;
    std::shared_ptr<shijima::mascot::environment> m_env;
    shijima::mascot::factory m_factory;
    QString m_importOnShowPath;
    std::vector<ShijimaWidget *> m_mascots;
    QString m_mascotsPath;
    QListWidget m_listWidget;
};