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

#include <QString>
#include <QDBusVirtualObject>
#include <QSocketNotifier>
#include "WindowObserverBackend.hpp"
#include "../ActiveWindow.hpp"

namespace Platform {

class PrivateActiveWindowObserver : public QDBusVirtualObject {
private:
    ActiveWindow m_activeWindow;
    ActiveWindow m_previousActiveWindow;
    std::unique_ptr<WindowObserverBackend> m_backend;
    QSocketNotifier *m_signalNotifier;
    void updateActiveWindow(QString const& uid, int pid, double x, double y,
        double width, double height);
public:
    static const QString m_dbusServiceName;
    static const QString m_dbusInterfaceName;
    static const QString m_dbusMethodName;
    PrivateActiveWindowObserver(QObject *obj);
    QString introspect(QString const& path) const override;
    bool handleMessage(const QDBusMessage &message,
        const QDBusConnection &connection) override;
    bool alive() { return (m_backend == nullptr) || m_backend->alive(); }
    ActiveWindow getActiveWindow() { return m_activeWindow; }
};

}
