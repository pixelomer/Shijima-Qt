#pragma once
#include <QString>
#include <QDBusVirtualObject>
#include "WindowObserverBackend.hpp"
#include "../ActiveWindow.hpp"

namespace Platform {

class PrivateActiveWindowObserver : public QDBusVirtualObject {
private:
    ActiveWindow m_activeWindow;
    ActiveWindow m_previousActiveWindow;
    std::unique_ptr<WindowObserverBackend> m_backend;
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
    bool alive() { return m_backend->alive(); }
    ActiveWindow getActiveWindow() { return m_activeWindow; }
};

}