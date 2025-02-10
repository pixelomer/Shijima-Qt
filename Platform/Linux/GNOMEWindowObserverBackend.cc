#include "GNOMEWindowObserverBackend.hpp"
#include "GNOME.hpp"
#include <QFile>
#include <QDataStream>

#include "gnome_script.c"

namespace Platform {

const QString GNOMEWindowObserverBackend::m_gnomeScriptUUID = "shijima-helper@pixelomer.github.io";
const QString GNOMEWindowObserverBackend::m_gnomeScriptVersion = "1.2";

GNOMEWindowObserverBackend::GNOMEWindowObserverBackend():
    m_gnomeScriptPath(m_gnomeScriptDir.filePath("shijima_gnome_extension.zip"))
{
    if (!GNOME::userExtensionsEnabled()) {
        GNOME::setUserExtensionsEnabled(true);
    }
    if (!m_gnomeScriptDir.isValid()) {
        throw std::runtime_error("failed to create temporary directory for GNOME extension");
    }
    QFile file { m_gnomeScriptPath };
    if (!file.open(QFile::WriteOnly)) {
        throw std::runtime_error("failed to create temporary file for KDE extension");
    }
    QDataStream stream { &file };
    stream << QByteArray(gnome_script, gnome_script_len);
    file.flush();
    file.close();
    GNOME::installExtension(m_gnomeScriptPath);
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
