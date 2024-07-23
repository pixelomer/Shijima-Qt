#include <QApplication>
#include "Platform/Platform.hpp"
#include "ShijimaManager.hpp"

int main(int argc, char **argv) {
    Platform::initialize();
    QApplication app(argc, argv);
    ShijimaManager::defaultManager()->show();
    return app.exec();
}