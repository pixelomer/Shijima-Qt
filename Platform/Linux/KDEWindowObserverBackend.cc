#include "KDEWindowObserverBackend.hpp"
#include "KWin.hpp"
#include <QTextStream>
#include <QTemporaryDir>

#include "kwin_script.c"

namespace Platform {

const QString KDEWindowObserverBackend::m_kwinScriptName = "ShijimaScript";

KDEWindowObserverBackend::KDEWindowObserverBackend(): WindowObserverBackend(),
    m_extensionFile("shijima_kwin_script.js", true, kwin_script, kwin_script_len)
{
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
    try {
        m_kwinScriptID = KWin::loadScript(m_extensionFile.path(),
            m_kwinScriptName);
    }
    catch (...) {
        // try unloading first?
        KWin::unloadScript(m_kwinScriptName);
        m_kwinScriptID = KWin::loadScript(m_extensionFile.path(),
            m_kwinScriptName);
    }
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