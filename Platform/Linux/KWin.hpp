#pragma once
#include <QString>
#include <QDBusMessage>
#include <QVariantList>
#include <stdexcept>

namespace Platform {
namespace KWin {

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

bool running();

}
}