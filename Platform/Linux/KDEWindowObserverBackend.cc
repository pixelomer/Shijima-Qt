#include "KDEWindowObserverBackend.hpp"
#include "KWin.hpp"
#include <QFile>
#include <QTextStream>

#include "kwin_script.c"

namespace Platform {

const QString KDEWindowObserverBackend::m_kwinScriptName = "ShijimaScript";
const QString KDEWindowObserverBackend::m_kwinScriptPath = "/tmp/shijima_get_active_window.js";

KDEWindowObserverBackend::KDEWindowObserverBackend(): WindowObserverBackend() {
    createKWinScript();
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

void KDEWindowObserverBackend::createKWinScript() {
    QFile file { m_kwinScriptPath };
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        throw std::runtime_error("could not open file for writing: "
            + m_kwinScriptPath.toStdString());
    }
    QTextStream stream { &file };
    stream << QByteArray(kwin_script, kwin_script_len);
    stream.flush();
    file.flush();
    file.close();
}

void KDEWindowObserverBackend::startKWinScript() {
    stopKWinScript();
    createKWinScript();
    m_kwinScriptID = KWin::loadScript(m_kwinScriptPath, m_kwinScriptName);
    KWin::runScript(m_kwinScriptID);
}

KDEWindowObserverBackend::~KDEWindowObserverBackend() {
    if (alive()) {
        //KWin::stopScript(m_kwinScriptID);
    }
}

}