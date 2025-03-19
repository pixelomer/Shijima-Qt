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

#include "PrivateActiveWindowObserver.hpp"
#include "KWin.hpp"
#include "GNOME.hpp"
#include "KWin.hpp"
#include "KDEWindowObserverBackend.hpp"
#include "GNOMEWindowObserverBackend.hpp"
#include "Platform-Linux.hpp"
#include <QProcessEnvironment>
#include <QDBusConnection>
#include <QTextStream>
#include <QGuiApplication>
#include <QFile>
#include <iostream>
#include <unistd.h>

namespace Platform {

const QString PrivateActiveWindowObserver::m_dbusInterfaceName = "com.pixelomer.ShijimaQt";
const QString PrivateActiveWindowObserver::m_dbusServiceName = "com.pixelomer.ShijimaQt";
const QString PrivateActiveWindowObserver::m_dbusMethodName = "updateActiveWindow";

PrivateActiveWindowObserver::PrivateActiveWindowObserver(QObject *obj)
    : QDBusVirtualObject(obj) 
{
    m_signalNotifier = new QSocketNotifier(Platform::terminateClientFd,
        QSocketNotifier::Read, this);
    connect(m_signalNotifier, &QSocketNotifier::activated, []{
        QGuiApplication::exit(0);
    });
    bool disableWindowTracking = QProcessEnvironment::systemEnvironment()
        .value("SHIJIMA_NO_WINDOW_TRACKING") == "1";
    if (disableWindowTracking) {
        std::cout << "Detected SHIJIMA_NO_WINDOW_TRACKING=1, window tracking " <<
            "is disabled" << std::endl;
        KWin::running(); // KDE still needs to be detected to disable window masks
        m_backend = nullptr;
    }
    else if (KWin::running()) {
        std::cout << "Detected KDE" << std::endl;
        m_backend = std::make_unique<KDEWindowObserverBackend>();
    }
    else if (GNOME::running()) {
        std::cout << "Detected GNOME" << std::endl;
        m_backend = std::make_unique<GNOMEWindowObserverBackend>();
    }
    else {
        std::cout << "Could not find a supported desktop environment, " <<
            "window tracking will not work" << std::endl;
        m_backend = nullptr;
    }
    if (m_backend != nullptr) {
        auto bus = QDBusConnection::sessionBus();
        if (!bus.isConnected()) {
            throw std::runtime_error("could not connect to DBus");
        }
        bool ret = bus.registerVirtualObject("/", this);
        if (!ret) {
            throw std::runtime_error("could not register object");
        }
        ret = bus.registerService(m_dbusServiceName);
        if (!ret) {
            throw std::runtime_error("could not register DBus service");
        }
    }
}

QString PrivateActiveWindowObserver::introspect(QString const&) const {
    const QString interfaceXML =
        "<interface name=\"" + m_dbusInterfaceName + "\">"
        "  <method name=\"" + m_dbusMethodName + "\">"
        "    <arg name=\"uid\" type=\"s\" direction=\"in\"/>"
        "    <arg name=\"pid\" type=\"i\" direction=\"in\"/>"
        "    <arg name=\"x\" type=\"d\" direction=\"in\"/>"
        "    <arg name=\"y\" type=\"d\" direction=\"in\"/>"
        "    <arg name=\"width\" type=\"d\" direction=\"in\"/>"
        "    <arg name=\"height\" type=\"d\" direction=\"in\"/>"
        "  </method>"
        "</interface>";
    return interfaceXML;
}

bool PrivateActiveWindowObserver::handleMessage(const QDBusMessage &message,
    const QDBusConnection &connection)
{
    if (message.type() != QDBusMessage::MethodCallMessage) {
        return false;
    }
    if (message.path() != "/") {
        return false;
    }
    if (message.interface() != m_dbusInterfaceName) {
        return false;
    }
    if (message.member() != m_dbusMethodName) {
        return false;
    }
    auto args = message.arguments();

    #define fail_invalid_args(msg) do { \
        QString _msg = msg; \
        auto reply = message.createErrorReply(QDBusError::InvalidArgs, \
            (_msg)); \
        connection.send(reply); \
        return true; \
    } \
    while (0)

    if (args.size() != 6) {
        fail_invalid_args("Expected 6 arguments");
    }
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    // type() is used here because it also works with Qt5
    if (args[0].type() != QVariant::String) {
        fail_invalid_args("Expected args[0] to be an String");
    }
    if (args[1].type() != QVariant::Int) {
        fail_invalid_args("Expected args[1] to be an Int");
    }
#pragma GCC diagnostic pop
    for (int i=2; i<=5; ++i) {
        if (args[i].canConvert<double>()) {
            continue;
        }
        fail_invalid_args(QString::fromStdString("Expected args[" + \
            std::to_string(i) + "] to be a Double"));
    }
    QString uid = args[0].toString();
    int pid = args[1].toInt();
    double x = args[2].toDouble();
    double y = args[3].toDouble();
    double width = args[4].toDouble();
    double height = args[5].toDouble();
    updateActiveWindow(uid, pid, x, y, width, height);

    auto reply = message.createReply();
    connection.send(reply);
    return true;
}

void PrivateActiveWindowObserver::updateActiveWindow(QString const& uid, int pid,
    double x, double y, double width, double height)
{
    /* std::cerr << "uid=" << uid.toStdString() << ", "
        << "pid=" << pid << ", "
        << "x=" << x << ", "
        << "y=" << y << ", "
        << "width=" << width << ", "
        << "height=" << height << ", " << std::endl; */
    if (getpid() == pid) {
        if (!m_activeWindow.available && m_previousActiveWindow.available) {
            m_activeWindow = m_previousActiveWindow;
        }
        return;
    }
    if (width < 0 && m_activeWindow.available && !m_previousActiveWindow.available) {
        m_previousActiveWindow = m_activeWindow;
        m_activeWindow = {};
    }
    else {
        m_activeWindow = { uid, pid, x, y, width, height };
        m_previousActiveWindow = {};
    }
}


}