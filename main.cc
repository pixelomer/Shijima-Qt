#include <QApplication>
#include <QDir>
#include <QStandardPaths>
#include <shijima/log.hpp>
#include "Platform/Platform.hpp"
#include "ShijimaManager.hpp"
#include "AssetLoader.hpp"

int main(int argc, char **argv) {
    Platform::initialize(argc, argv);
    #ifdef _WIN32
        freopen("shijima_stdout.txt", "a", stdout);
        freopen("shijima_stderr.txt", "a", stderr);
    #endif
    #ifdef SHIJIMA_LOGGING_ENABLED
        shijima::set_log_level(SHIJIMA_LOG_PARSER | SHIJIMA_LOG_WARNINGS);
    #endif
    QApplication app(argc, argv);
    app.setApplicationName("Shijima-Qt");
    app.setApplicationDisplayName("Shijima-Qt");
    ShijimaManager::defaultManager()->show();
    int ret = app.exec();
    ShijimaManager::finalize();
    AssetLoader::finalize();
    std::cout << std::endl;
    return ret;
}