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

#include "GNOMEWindowObserverBackend.hpp"
#include "GNOME.hpp"
#include <QFile>
#include <QDataStream>

#include "gnome_script.c"

namespace Platform {

const QString GNOMEWindowObserverBackend::m_gnomeScriptUUID = "shijima-helper@pixelomer.github.io";
const QString GNOMEWindowObserverBackend::m_gnomeScriptVersion = "0.1.0";

GNOMEWindowObserverBackend::GNOMEWindowObserverBackend(): m_extensionFile(
    "shijima_gnome_extension.zip", false, gnome_script, gnome_script_len)
{
    if (!GNOME::userExtensionsEnabled()) {
        GNOME::setUserExtensionsEnabled(true);
    }
    GNOME::installExtension(m_extensionFile.hostPath());
    auto extensionInfo = GNOME::getExtensionInfo(m_gnomeScriptUUID);
    static const QString kVersionName = "version-name";
    std::string restartReason;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    if (!extensionInfo.contains(kVersionName)) {
        restartReason = "Extension was installed for the first time.";
    }
    else if (extensionInfo[kVersionName].type() != QVariant::String) {
        // type() is used here because it also works with Qt5
        restartReason = "Active extension contains malformed metadata.";
    }
    else if (extensionInfo[kVersionName].toString() != m_gnomeScriptVersion) {
        restartReason = "Active extension is outdated.";
    }
#pragma GCC diagnostic pop
    if (restartReason != "") {
        // Shell needs to be restarted
        throw std::runtime_error("Shijima GNOME Helper has been installed. "
            "To use Shijima-Qt, log out and log back in. (Restart reason: "
            + restartReason + ")");
    }
    GNOME::enableExtension(m_gnomeScriptUUID);
}

GNOMEWindowObserverBackend::~GNOMEWindowObserverBackend() {
    if (alive()) {
        GNOME::disableExtension(m_gnomeScriptUUID);
    }
}

bool GNOMEWindowObserverBackend::alive() {
    return GNOME::isExtensionEnabled(m_gnomeScriptUUID);
}

}
