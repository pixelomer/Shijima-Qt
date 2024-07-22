#include "KWin.hpp"
#include <QDBusConnection>
#include <iostream>
#include <QDBusMessage>

namespace Platform {
namespace KWin {

QVariantList callDBus(QDBusMessage const& message, const char *format) {
    auto method = message.member();
    auto bus = QDBusConnection::sessionBus();
    auto res = bus.call(message);
    if (res.type() != QDBusMessage::ReplyMessage) {
        auto err = res.errorMessage();
        throw DBusCallError(method, err);
    }
    auto list = res.arguments();
    if (format != NULL) {
        auto size = list.size();
        for (int i=0; format[i] != 0; ++i) {
            if (i >= size) {
                throw DBusReturnError(method);
            }
            char c = format[i];
            QVariant::Type type;
            switch (c) {
                case 'i': type = QVariant::Int; break;
                case 'b': type = QVariant::Bool; break;
                default:
                    throw std::invalid_argument("invalid format");
            }
            if (list[i].type() != type) {
                throw DBusReturnError(method);
            }
        }
    }
    return list;
}

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
        throw DBusCallError("loadScript",
            QString::fromStdString("Returned " + std::to_string(id)));
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

}
}