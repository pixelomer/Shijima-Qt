#include "PrivateActiveWindowObserver.hpp"
#include <windows.h>
#include <iostream>

namespace Platform {

PrivateActiveWindowObserver::PrivateActiveWindowObserver(): m_scaleRatio(0) {}

ActiveWindow PrivateActiveWindowObserver::getActiveWindow() {
    if (m_scaleRatio == 0) {
        HWND topWindow = GetTopWindow(NULL);
        if (topWindow == NULL) {
            return m_activeWindow = {};
        }
        UINT dpi = GetDpiForWindow(topWindow);
        if (dpi == 0) {
            return m_activeWindow = {};
        }
        m_scaleRatio = 96.0 / dpi;
    }
    HWND foregroundWindow = GetForegroundWindow();
    if (foregroundWindow == NULL) {
        return m_activeWindow;
    }
    DWORD newPid;
    if (GetWindowThreadProcessId(foregroundWindow, &newPid) == 0) {
        return m_activeWindow = {};
    }
    if ((long)newPid == _getpid()) {
        return m_activeWindow;
    }
    RECT rect;
    if (!GetWindowRect(foregroundWindow, &rect)) {
        return m_activeWindow = {};
    }
    QString uid = QString::fromStdString(std::to_string(newPid) + "-"
        + std::to_string((unsigned long long)foregroundWindow));
    return m_activeWindow = { uid, (long)newPid,
        rect.left * m_scaleRatio,
        rect.top * m_scaleRatio,
        (rect.right - rect.left) * m_scaleRatio,
        (rect.bottom - rect.top) * m_scaleRatio };
}

}