#pragma once
#include <QString>
#include <QDBusMessage>
#include <QVariantList>
#include <stdexcept>

namespace Platform {
namespace KWin {

class DBusReturnError : public std::runtime_error {
public:
    DBusReturnError(QString const& method):
        std::runtime_error("KWin::" + method.toStdString() +
            "() DBus call returned unexpected response") {}
};

class DBusCallError : public std::runtime_error {
public:
    DBusCallError(QString const& method, QString const& error):
        std::runtime_error("KWin::" + method.toStdString() +
            "() DBus call failed: " + error.toStdString()) {}
};

/// @brief Calls a DBus method and returns its return value if successful.
/// @param message Message to send to DBus.
/// @param format A null-terminated string describing the expected return value
///               of the method. For example, "iib" represents a return value
///               of 2 integers and 1 boolean. Allowed types are: 'b'=boolean,
///               'i'=integer. The format may be null, in which case the return
///               value will not be checked.
/// @return Return value of the method.
/// @throws Throws DBusCallError if the call failed. Throws DBusReturnError if
///         the call succeeded but the return value format does not match
///         `format`.
QVariantList callDBus(QDBusMessage const& message, const char *format);

/// @brief Checks whether a script with the given plugin name is loaded.
/// @param pluginName Plugin name.
/// @return Whether the script is loaded or not.
bool isScriptLoaded(QString const& pluginName);

/// @brief Loads a script to be used in KWin.
/// @param path Absolute path to JavaScript file.
/// @param pluginName Plugin name.
/// @return Loaded script ID.
int loadScript(QString const& path, QString const& pluginName);

/// @brief Runs a previously loaded KWin script.
/// @param id Script ID returned by `KWin::loadScript()`.
void runScript(int id);

/// @brief Stops a KWin script.
/// @param id Script ID returned by `KWin::loadScript()`.
void stopScript(int id);

/// @brief Unloads a script with the given plugin name.
/// @param pluginName Plugin name.
/// @returns Whether the operation was successful or not.
bool unloadScript(QString const& pluginName);

}
}