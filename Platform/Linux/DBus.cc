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

#include "DBus.hpp"
#include <QDBusConnection>
#include <QDBusVariant>

namespace Platform {
namespace DBus {

QVariantList callDBus(QDBusMessage const& message, QString const& signature) {
    auto method = message.member();
    auto bus = QDBusConnection::sessionBus();
    auto res = bus.call(message);
    if (res.type() != QDBusMessage::ReplyMessage) {
        auto err = res.errorMessage();
        throw DBusCallError(method, err);
    }
    auto list = res.arguments();
    if (signature != "" && res.signature() != signature) {
        throw DBusReturnError(method, signature, res.signature());
    }
    return list;
}

QVariant getDBusProperty(QString const& service, QString const& path,
    QString const& interface, QString const& property)
{
    auto msg = QDBusMessage::createMethodCall(service, path,
        "org.freedesktop.DBus.Properties", "Get");
    msg << interface;
    msg << property;
    auto res = callDBus(msg, "v");
    QDBusVariant dbusVariant = res[0].value<QDBusVariant>();
    return dbusVariant.variant();
}

void setDBusProperty(QString const& service, QString const& path,
    QString const& interface, QString const& property, QVariant const& value)
{
    auto msg = QDBusMessage::createMethodCall(service, path,
        "org.freedesktop.DBus.Properties", "Set");
    QDBusVariant dbusVariant;
    dbusVariant.setVariant(value);
    msg << interface;
    msg << property;
    msg << QVariant::fromValue(dbusVariant);
    callDBus(msg);
}

}
}