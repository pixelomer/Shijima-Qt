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

#include <QtGlobal>
#if QT_VERSION < 0x060000
#   define QT5_MANUALLY_DETERMINE_SCALE
#   include <QDesktopWidget>
#   include <cmath>
#   include <charconv>
#   include <QApplication>
#endif
#include "../Platform.hpp"
#include <stdlib.h>
#include <QWidget>
#include <QApplication>
#include <X11/Xlib.h>
#include <iostream>
#include <X11/Xatom.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>

namespace Platform {

int terminateServerFd = -1;
int terminateClientFd = -1;
bool windowMasksEnabled = true;

void handleSignal(int sig) {
    write(terminateServerFd, "\x01", 1);
}

#ifdef QT5_MANUALLY_DETERMINE_SCALE
void determineScale(int argc, char **argv) {
    // Initializing QApplication multiple times breaks QDBus.
    // Therefore, this function gets the display scale and "resets"
    // the program by calling execv().

    // Do not continue if the scale has already been determined.
    {
        char *shijimaScaleSet = getenv("SHIJIMA_SCALE_SET");
        if (shijimaScaleSet != NULL && strcmp(shijimaScaleSet, "1") == 0) {
            return;
        }
    }

    // Start Qt in Wayland and get the display width
    double scaledWidth;
    {
        QApplication app(argc, argv);
        auto desktop = QApplication::desktop();
        scaledWidth = desktop->width();
        QApplication::exit(0);
    }

    // Set WAYLAND_DISPLAY to an invalid value to force Qt to use X11
    setenv("WAYLAND_DISPLAY", "", 1);

    // Start Qt in X11 and get the display width
    double unscaledWidth;
    {
        QApplication app(argc, argv);
        auto desktop = QApplication::desktop();
        unscaledWidth = desktop->width();
        QApplication::exit(0);
    }

    // Divide X11 width to Wayland width to get the scale factor
    double scaleFactor = unscaledWidth / scaledWidth;
    scaleFactor = std::round(scaleFactor * 100.0) / 100.0;
    std::array<char, 10> scaleFactorBuf;
    scaleFactorBuf.fill(0);
    std::to_chars(&scaleFactorBuf[0], &scaleFactorBuf[scaleFactorBuf.size()],
        scaleFactor, std::chars_format::general, 3);
    std::string scaleFactorStr { &scaleFactorBuf[0] };

    // Set necessary environment variables for Qt
    setenv("QT_AUTO_SCREEN_SCALE_FACTOR", "0", 1);
    setenv("QT_SCALE_FACTOR", scaleFactorStr.c_str(), 1);
    setenv("QT_SCALE_FACTOR_ROUNDING_POLICY", "PassThrough", 1);
    setenv("SHIJIMA_SCALE_SET", "1", 1);

    // Reset the application.
    execv(argv[0], argv);
    throw std::system_error({ errno, std::generic_category() }, strerror(errno));
}
#endif

void initialize(int argc, char **argv) {
    #ifdef QT5_MANUALLY_DETERMINE_SCALE
    determineScale(argc, argv);
    #endif

    // Create sockets for notifying signals
    int fds[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == -1) {
        throw std::system_error({ errno, std::generic_category() }, strerror(errno));
    }
    terminateServerFd = fds[0];
    terminateClientFd = fds[1];

    // Install signal handlers
    struct sigaction action = { 0 };
    action.sa_handler = handleSignal;
    sigaction(SIGINT, &action, NULL);
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGHUP, &action, NULL);

    // Wayland does not allow windows to reposition themselves.
    // Set WAYLAND_DISPLAY to an invalid value to prevent its use.
    setenv("WAYLAND_DISPLAY", "", 1);
}

void showOnAllDesktops(QWidget *widget) {
    unsigned long data = 0xFFFFFFFF;
    QNativeInterface::QX11Application *x11App = qApp->nativeInterface<QNativeInterface::QX11Application>();
    Display *displayID = x11App->display();
    WId windowID = widget->winId();
    XChangeProperty(displayID, windowID,
        XInternAtom(displayID, "_NET_WM_DESKTOP", False),
        XA_CARDINAL, 32, PropModeReplace,
        reinterpret_cast<unsigned char *>(&data), 1);
    Atom stickyAtom = XInternAtom(displayID, "_NET_WM_STATE_STICKY", 1);
    Atom stateAtom = XInternAtom(displayID, "_NET_WM_STATE", 1);
    if (stickyAtom != None && stateAtom != None) {
        XClientMessageEvent xclient;
        memset(&xclient, 0, sizeof (xclient));
        xclient.type = ClientMessage;
        xclient.window = windowID;
        xclient.message_type = stateAtom;
        xclient.format = 32;
        xclient.data.l[0] = 1; // _NET_WM_STATE_ADD
        xclient.data.l[1] = stickyAtom;
        xclient.data.l[2] = 0;
        xclient.data.l[3] = 0;
        xclient.data.l[4] = 0;
        XSendEvent(displayID,
          DefaultRootWindow(displayID),
          False,
          SubstructureRedirectMask | SubstructureNotifyMask,
          (XEvent *)&xclient);
        XFlush(displayID);
    }
}

bool useWindowMasks() {
    return windowMasksEnabled;
}

}