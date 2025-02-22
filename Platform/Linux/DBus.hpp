#pragma once

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

#include <QDBusMessage>
#include <QString>
#include <stdexcept>

namespace Platform {
namespace DBus {

class DBusReturnError : public std::runtime_error {
public:
    DBusReturnError(QString const& method, QString const& expected,
        QString const& actual):
        std::runtime_error((method +
            "() DBus call returned unexpected response "
            "(expected: " + expected + ", actual: " + actual + ")")
            .toStdString()) {}
};

class DBusCallError : public std::runtime_error {
public:
    DBusCallError(QString const& method, QString const& error):
        std::runtime_error(method.toStdString() +
            "() DBus call failed: " + error.toStdString()) {}
    DBusCallError(QDBusMessage const& message, QString const& error):
        DBusCallError(message.member(), error) {}
};

/// @brief Calls a DBus method and returns its return value if successful.
/// @param message Message to send to DBus.
/// @param signature Expected response signature.
/// @return Return value of the method.
/// @throws Throws DBusCallError if the call failed. Throws DBusReturnError if
///         the call succeeded but the return value format does not match
///         `signature`.
QVariantList callDBus(QDBusMessage const& message, QString const& signature = "");

QVariant getDBusProperty(QString const& service, QString const& path,
    QString const& interface, QString const& property);
void setDBusProperty(QString const& service, QString const& path,
    QString const& interface, QString const& property, QVariant const& value);

}
}