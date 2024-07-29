#include "GNOMEWindowObserverBackend.hpp"
#include "GNOME.hpp"
#include <QFile>
#include <QDataStream>

#include "gnome_script.c"

namespace Platform {

const QString GNOMEWindowObserverBackend::m_gnomeScriptUUID = "shijima-helper@pixelomer.github.io";
const QString GNOMEWindowObserverBackend::m_gnomeScriptPath = "/tmp/gnome-shijima-helper.zip";

GNOMEWindowObserverBackend::GNOMEWindowObserverBackend() {
    if (!GNOME::userExtensionsEnabled()) {
        GNOME::setUserExtensionsEnabled(true);
    }
    writeExtensionToDisk();
    GNOME::installExtension(m_gnomeScriptPath);
    if (GNOME::isExtensionInstalled(m_gnomeScriptUUID)) {
        GNOME::enableExtension(m_gnomeScriptUUID);
    }
    else {
        throw std::runtime_error("Shijima GNOME Helper has been installed. "
            "To use Shijima-Qt, log out and log back in.");
    }
}

void GNOMEWindowObserverBackend::writeExtensionToDisk() {
    QFile file { m_gnomeScriptPath };
    if (!file.open(QFile::WriteOnly)) {
        throw std::runtime_error("could not open file for writing: "
            + m_gnomeScriptPath.toStdString());
    }
    QDataStream stream { &file };
    stream << QByteArray(gnome_script, gnome_script_len);
    file.flush();
    file.close();
}

GNOMEWindowObserverBackend::~GNOMEWindowObserverBackend() {
    if (alive()) {
        //GNOME::disableExtension(m_gnomeScriptUUID);
    }
}

bool GNOMEWindowObserverBackend::alive() {
    return GNOME::isExtensionEnabled(m_gnomeScriptUUID);
}

}