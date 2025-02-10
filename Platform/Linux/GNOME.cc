#include "GNOME.hpp"
#include "DBus.hpp"
#include <QDBusMessage>
#include <QVariantList>
#include <QDBusArgument>

using namespace Platform::DBus;

namespace Platform {
namespace GNOME {

QMap<QString, QVariant> getExtensionInfo(QString const& uuid) {
    auto msg = QDBusMessage::createMethodCall("org.gnome.Shell",
        "/org/gnome/Shell", "org.gnome.Shell.Extensions", 
        "GetExtensionInfo");
    msg << uuid;
    auto res = callDBus(msg, "a{sv}");
    QDBusArgument arg = res[0].value<QDBusArgument>();
    QMap<QString, QVariant> map;
    arg >> map;
    return map;
}

bool isExtensionInstalled(QString const& uuid) {
    auto map = getExtensionInfo(uuid);
    return map.size() > 0;
}

bool isExtensionEnabled(QString const& uuid) {
    auto map = getExtensionInfo(uuid);
    return map.contains("enabled") &&
        map["enabled"].canConvert<bool>() &&
        map["enabled"].toBool();
}

void installExtension(QString const& path) {
    auto stdPath = path.toStdString();

    for (char c : stdPath) {
        if (c >= 'A' && c <= 'Z') continue;
        if (c >= 'a' && c <= 'z') continue;
        if (c >= '0' && c <= '9') continue;
        if (c == '-' || c == '.' || c == '_' || c == '/') continue;
        throw std::invalid_argument("Potentially unsafe path (" + std::string(&c, 1) + "): " + path.toStdString());
    }

    // No need to overcomplicate the code. system() will
    // do the job just fine.
    auto cmd = "gnome-extensions install --force " + stdPath;
    int ret = std::system(cmd.c_str());
    if (ret == -1) {
        throw std::system_error({ errno, std::generic_category() }, strerror(errno));
    }
    else if (ret != 0) {
        throw std::runtime_error("gnome-extensions install failed: " +
            std::to_string(ret));
    }
}

void enableExtension(QString const& uuid) {
    auto msg = QDBusMessage::createMethodCall("org.gnome.Shell",
        "/org/gnome/Shell", "org.gnome.Shell.Extensions", 
        "EnableExtension");
    msg << uuid;
    auto res = callDBus(msg, "b");
    if (!res[0].toBool()) {
        throw DBusCallError(msg, "Returned false");
    }
}

void disableExtension(QString const& uuid) {
    auto msg = QDBusMessage::createMethodCall("org.gnome.Shell",
        "/org/gnome/Shell", "org.gnome.Shell.Extensions", 
        "DisableExtension");
    msg << uuid;
    auto res = callDBus(msg, "b");
    if (!res[0].toBool()) {
        throw DBusCallError(msg, "Returned false");
    }
}

bool userExtensionsEnabled() {
    auto res = getDBusProperty("org.gnome.Shell", "/org/gnome/Shell",
        "org.gnome.Shell.Extensions", "UserExtensionsEnabled");
    return res.canConvert<bool>() && res.toBool();
}

void setUserExtensionsEnabled(bool enabled) {
    setDBusProperty("org.gnome.Shell", "/org/gnome/Shell",
        "org.gnome.Shell.Extensions", "UserExtensionsEnabled",
        QVariant(enabled));
}

bool running() {
    try {
        isExtensionInstalled("");
        return true;
    }
    catch (DBusCallError &err) {
        return false;
    }
}

}
}