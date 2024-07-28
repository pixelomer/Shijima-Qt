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
/// @param format Expected response signature.
/// @return Return value of the method.
/// @throws Throws DBusCallError if the call failed. Throws DBusReturnError if
///         the call succeeded but the return value format does not match
///         `format`.
QVariantList callDBus(QDBusMessage const& message, QString const& signature = "");

QVariant getDBusProperty(QString const& service, QString const& path,
    QString const& interface, QString const& property);
void setDBusProperty(QString const& service, QString const& path,
    QString const& interface, QString const& property, QVariant const& value);

}
}