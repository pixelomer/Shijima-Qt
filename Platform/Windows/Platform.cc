#include "../Platform.hpp"
#include <QWidget>
#include <windows.h>

namespace Platform {

void initialize(int argc, char **argv) {
    freopen("shijima_stdout.txt", "a", stdout);
    freopen("shijima_stderr.txt", "a", stderr);
}

void showOnAllDesktops(QWidget *widget) {
    HWND window = (HWND)widget->winId();
    LONG_PTR exstyle = GetWindowLongPtr(window, GWL_EXSTYLE);
    if (exstyle != 0) {
        exstyle |= WS_EX_TOOLWINDOW;
        SetWindowLongPtr(window, GWL_EXSTYLE, exstyle);
    }
}

}