#include <QApplication>
#include "Platform/Platform.hpp"
#include "ShijimaManager.hpp"

int main(int argc, char **argv) {
    // Wayland does not allow windows to reposition themselves.
    // Set WAYLAND_DISPLAY to an invalid value to prevent its use.
    setenv("WAYLAND_DISPLAY", "", 1);
    QApplication app(argc, argv);
    ShijimaManager::defaultManager()->show();
    return app.exec();
}
