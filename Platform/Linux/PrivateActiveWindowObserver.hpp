#pragma once
#include <QString>
#include <QDBusVirtualObject>
#include "../ActiveWindow.hpp"

namespace Platform {

class PrivateActiveWindowObserver : public QDBusVirtualObject {
private:
    static const QString m_kwinScriptPath;
    static const QString m_kwinScriptName;
    static const QString m_dbusServiceName;
    static const QString m_dbusInterfaceName;
    static const QString m_dbusMethodName;
    int m_kwinScriptID = -1;
    ActiveWindow m_activeWindow;
    ActiveWindow m_previousActiveWindow;
    void updateActiveWindow(int pid, double x, double y, double width,
        double height);
    void createKWinScript();
public:
    PrivateActiveWindowObserver(QObject *obj);
    ~PrivateActiveWindowObserver();
    QString introspect(QString const& path) const override;
    bool handleMessage(const QDBusMessage &message,
        const QDBusConnection &connection) override;
    void startKWinScript();
    bool isKWinScriptLoaded();
    void stopKWinScript();
    ActiveWindow getActiveWindow() { return m_activeWindow; }
};

}