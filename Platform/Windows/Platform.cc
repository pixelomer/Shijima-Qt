#include "../Platform.hpp"
#include <QWidget>
#include <windows.h>

namespace Platform {

void initialize(int argc, char **argv) {}
void showOnAllDesktops(QWidget *widget) {
    HWND window = (HWND)widget->winId();
    LONG_PTR exstyle = GetWindowLongPtr(window, GWL_EXSTYLE);
    if (exstyle != 0) {
        exstyle |= WS_EX_TOOLWINDOW;
        SetWindowLongPtr(window, GWL_EXSTYLE, exstyle);
    }
}

}