#include "PrivateActiveWindowObserver.hpp"
#include "KWin.hpp"
#include "GNOME.hpp"
#include "KWin.hpp"
#include "KDEWindowObserverBackend.hpp"
#include "GNOMEWindowObserverBackend.hpp"
#include <QDBusConnection>
#include <QTextStream>
#include <iostream>
#include <QFile>
#include <unistd.h>

namespace Platform {

const QString PrivateActiveWindowObserver::m_dbusInterfaceName = "com.pixelomer.ShijimaQT";
const QString PrivateActiveWindowObserver::m_dbusServiceName = "com.pixelomer.ShijimaQT";
const QString PrivateActiveWindowObserver::m_dbusMethodName = "updateActiveWindow";

PrivateActiveWindowObserver::PrivateActiveWindowObserver(QObject *obj)
    : QDBusVirtualObject(obj) 
{
    if (KWin::running()) {
        std::cout << "Detected KDE" << std::endl;
        m_backend = std::make_unique<KDEWindowObserverBackend>();
    }
    else if (GNOME::running()) {
        std::cout << "Detected GNOME" << std::endl;
        m_backend = std::make_unique<GNOMEWindowObserverBackend>();
    }
    else {
        throw std::runtime_error("No window observer backend available");
    }
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

QString PrivateActiveWindowObserver::introspect(QString const& path) const {
    const QString interfaceXML =
        "<interface name=\"" + m_dbusInterfaceName + "\">"
        "  <method name=\"" + m_dbusMethodName + "\">"
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
    if (args.size() != 5) {
        auto reply = message.createErrorReply(QDBusError::InvalidArgs,
            "Expected 5 arguments");
        connection.send(reply);
        return true;
    }
    if (args[0].type() != QVariant::Int) {
        auto reply = message.createErrorReply(QDBusError::InvalidArgs,
            "Expected args[0] to be an Int");
        connection.send(reply);
        return true;
    }
    for (int i=1; i<=4; ++i) {
        if (args[i].canConvert<double>()) {
            continue;
        }
        auto reply = message.createErrorReply(QDBusError::InvalidArgs,
            QString::fromStdString("Expected args[" + std::to_string(i)
                + "] to be a Double"));
        connection.send(reply);
        return true;
    }
    int pid = args[0].toInt();
    double x = args[1].toDouble();
    double y = args[2].toDouble();
    double width = args[3].toDouble();
    double height = args[4].toDouble();
    updateActiveWindow(pid, x, y, width, height);

    auto reply = message.createReply();
    connection.send(reply);
    return true;
}

void PrivateActiveWindowObserver::updateActiveWindow(int pid, double x,
    double y, double width, double height)
{
    //std::cerr << pid << ", " << x << ", " << y << std::endl;
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
        m_activeWindow = { pid, x, y, width, height };
        m_previousActiveWindow = {};
    }
}


}