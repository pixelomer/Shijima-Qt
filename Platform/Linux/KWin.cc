#include "KWin.hpp"
#include <QDBusMessage>
#include "DBus.hpp"

using namespace Platform::DBus;

namespace Platform {
namespace KWin {

bool isScriptLoaded(QString const& pluginName) {
    auto msg = QDBusMessage::createMethodCall("org.kde.KWin",
        "/Scripting", "org.kde.kwin.Scripting", "isScriptLoaded");
    msg << pluginName;
    auto res = callDBus(msg, "b");
    return res[0].toBool();
}

int loadScript(QString const& path, QString const& pluginName) {
    auto msg = QDBusMessage::createMethodCall("org.kde.KWin",
        "/Scripting", "org.kde.kwin.Scripting", "loadScript");
    msg << path;
    msg << pluginName;
    auto res = callDBus(msg, "i");
    int id = res[0].toInt();
    if (id < 0) {
        throw DBusCallError(msg, QString::fromStdString("Returned " +
            std::to_string(id)));
    }
    return id;
}

void runScript(int id) {
    std::string path = "/Scripting/Script" + std::to_string(id);
    auto msg = QDBusMessage::createMethodCall("org.kde.KWin",
        QString::fromStdString(path), "org.kde.kwin.Script", "run");
    callDBus(msg, NULL);
}

void stopScript(int id) {
    std::string path = "/Scripting/Script" + std::to_string(id);
    auto msg = QDBusMessage::createMethodCall("org.kde.KWin",
        QString::fromStdString(path), "org.kde.kwin.Script", "stop");
    callDBus(msg, NULL);
}

bool unloadScript(QString const& pluginName) {
    auto msg = QDBusMessage::createMethodCall("org.kde.KWin",
        "/Scripting", "org.kde.kwin.Scripting", "unloadScript");
    msg << pluginName;
    auto res = callDBus(msg, "b");
    return res[0].toBool();
}

bool running() {
    try {
        isScriptLoaded("");
        return true;
    }
    catch (DBusCallError &err) {
        return false;
    }
}

}
}