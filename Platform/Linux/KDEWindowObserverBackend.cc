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
        m_kwinScriptID = KWin::loadScript(m_extensionFile.hostPath(),
            m_kwinScriptName);
    }
    catch (...) {
        // try unloading first?
        KWin::unloadScript(m_kwinScriptName);
        m_kwinScriptID = KWin::loadScript(m_extensionFile.hostPath(),
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