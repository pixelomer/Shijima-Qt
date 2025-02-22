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

#include "GNOME.hpp"
#include "DBus.hpp"
#include <QDBusMessage>
#include <QVariantList>
#include <QDBusArgument>
#include <QProcess>
#include <QStringList>
#include "ExtensionFile.hpp"
#include <QDir>

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
    QString program;
    QStringList args;
    if (ExtensionFile::flatpak()) {
        program = "flatpak-spawn";
        args = { "--host", "gnome-extensions", "install",
            "--force", path };
    }
    else {
        program = "gnome-extensions";
        args = { "install", "--force", path };
    }
    QProcess process;
    process.setProcessChannelMode(QProcess::ProcessChannelMode::ForwardedChannels);
    process.start(program, args);
    process.waitForFinished(-1);
    int exitCode = process.exitCode();
    if (exitCode != 0) {
        throw std::runtime_error(std::string("gnome-extensions install failed. ") +
            "See output for details. (" + std::to_string(exitCode) + ")");
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