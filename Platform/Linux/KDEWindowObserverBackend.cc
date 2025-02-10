#include "KDEWindowObserverBackend.hpp"
#include "KWin.hpp"
#include <QTextStream>
#include <QTemporaryDir>

#include "kwin_script.c"

namespace Platform {

const QString KDEWindowObserverBackend::m_kwinScriptName = "ShijimaScript";

KDEWindowObserverBackend::KDEWindowObserverBackend(): WindowObserverBackend(),
    m_kwinScriptPath(m_kwinScriptDir.filePath("shijima_kwin_script.js"))
{
    if (!m_kwinScriptDir.isValid()) {
        throw std::runtime_error("failed to create temporary directory for KDE extension");
    }
    QFile file { m_kwinScriptPath };
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        throw std::runtime_error("failed to create temporary file for KDE extension");
    }
    QTextStream stream { &file };
    stream << QByteArray(kwin_script, kwin_script_len);
    stream.flush();
    file.flush();
    file.close();
    loadKWinScript();
    startKWinScript();
}

bool KDEWindowObserverBackend::alive() {
    return isKWinScriptLoaded();
}

bool KDEWindowObserverBackend::isKWinScriptLoaded() {
    return KWin::isScriptLoaded(m_kwinScriptName);
}

void KDEWindowObserverBackend::stopKWinScript() {
    if (KWin::isScriptLoaded(m_kwinScriptName)) {
        KWin::unloadScript(m_kwinScriptName);
    }
    m_kwinScriptID = -1;
}

void KDEWindowObserverBackend::loadKWinScript() {
    m_kwinScriptID = KWin::loadScript(m_kwinScriptPath, m_kwinScriptName);
}

void KDEWindowObserverBackend::startKWinScript() {
    stopKWinScript();
    loadKWinScript();
    KWin::runScript(m_kwinScriptID);
}

KDEWindowObserverBackend::~KDEWindowObserverBackend() {
    if (alive()) {
        KWin::stopScript(m_kwinScriptID);
        KWin::unloadScript(m_kwinScriptName);
    }
}

}