#include <QApplication>
#include <QDir>
#include "Platform/Platform.hpp"
#include "ShijimaManager.hpp"
#include "AssetLoader.hpp"

int main(int argc, char **argv) {
    Platform::initialize(argc, argv);
    if (argc == 2) {
        bool ret = QDir::setCurrent(argv[1]);
        if (!ret) {
            throw std::runtime_error("Could not change working directory");
        }
    }
    QApplication app(argc, argv);
    ShijimaManager::defaultManager()->show();
    int ret = app.exec();
    ShijimaManager::finalize();
    AssetLoader::finalize();
    return ret;
}