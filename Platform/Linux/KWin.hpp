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