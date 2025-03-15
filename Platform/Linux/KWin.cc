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

#include "KWin.hpp"
#include <QDBusMessage>
#include "DBus.hpp"
#include "Platform-Linux.hpp"

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
    callDBus(msg);
}

void stopScript(int id) {
    std::string path = "/Scripting/Script" + std::to_string(id);
    auto msg = QDBusMessage::createMethodCall("org.kde.KWin",
        QString::fromStdString(path), "org.kde.kwin.Script", "stop");
    callDBus(msg);
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
        Platform::windowMasksEnabled = false;
        return true;
    }
    catch (DBusCallError &err) {
        return false;
    }
}

}
}