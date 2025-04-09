#pragma once

// 
// Shijima-Qt - Cross-platform shimeji simulation app for desktop
// Copyright (C) 2025 pixelomer
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// 

#include <QMainWindow>
#include <QString>
#include <shijima/mascot/manager.hpp>
#include <shijima/mascot/factory.hpp>
#include <QMap>
#include <QListWidgetItem>
#include <QListWidget>
#include <QSettings>
#include <QScreen>
#include "ActiveMascot.hpp"
#include "PlatformWidget.hpp"
#include "MascotData.hpp"
#include <set>
#include <list>
#include <mutex>
#include "Platform/ActiveWindowObserver.hpp"
#include "ShijimaWidget.hpp"
#include "ShijimaHttpApi.hpp"
#include <condition_variable>

class QVBoxLayout;
class QWidget;
class MascotBackend;

class ShijimaManager : public PlatformWidget<QMainWindow>
{
public:
    static ShijimaManager *defaultManager();
    static void finalize();
    QString const& mascotsPath();
    ActiveMascot *spawn(std::string const& name);
    void killAll();
    void killAll(QString const& name);
    void killAllButOne(ActiveMascot *widget);
    void killAllButOne(QString const& name);
    void setManagerVisible(bool visible);
    void importOnShow(QString const& path);
    bool changeBackend(std::function<MascotBackend *()> backendProvider);
    QMap<QString, MascotData *> const& loadedMascots();
    QMap<int, MascotData *> const& loadedMascotsById();
    std::list<ActiveMascot *> const& mascots();
    std::map<int, ActiveMascot *> const& mascotsById();
    ActiveMascot *hitTest(QPoint const& screenPos);
    void onTickSync(std::function<void(ShijimaManager *)> callback);
    ~ShijimaManager();
    Platform::ActiveWindow const& previousActiveWindow();
    Platform::ActiveWindow const& currentActiveWindow();
    void applyActiveIE(shijima::mascot::environment &env);
    double userScale();
    int subtickCount();
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
    std::unique_lock<std::mutex> acquireLock();
    void loadDefaultMascot();
    void loadData(MascotData *data);
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
    void updateSandboxBackground();
    QWidget *mascotParent();
    void quitAction();
    std::set<std::string> import(QString const& path) noexcept;
    void importWithDialog(QList<QString> const& paths);
    void tick();
    QColor m_sandboxBackground;
    QWidget *m_sandboxWidget;
    QSettings m_settings;
    Platform::ActiveWindow m_previousWindow;
    Platform::ActiveWindow m_currentWindow;
    Platform::ActiveWindowObserver m_windowObserver;
    int m_mascotTimer = -1;
    bool m_allowClose = false;
    bool m_firstShow = true;
    bool m_wasVisible = false;
    int m_idCounter;
    double m_userScale = 1.0;
    int m_windowObserverTimer = -1;
    QMap<QString, MascotData *> m_loadedMascots;
    QMap<int, MascotData *> m_loadedMascotsById;
    QSet<QString> m_listItemsToRefresh;
    shijima::mascot::factory m_factory;
    QString m_importOnShowPath;
    std::list<ActiveMascot *> m_mascots;
    std::map<int, ActiveMascot *> m_mascotsById;
    QString m_mascotsPath;
    QListWidget m_listWidget;
    ShijimaHttpApi m_httpApi;
    bool m_hasTickCallbacks;
    std::mutex m_mutex;
    std::condition_variable m_tickCallbackCompletion;
    std::list<std::function<void(ShijimaManager *)>> m_tickCallbacks;
    MascotBackend *m_backend;
};
