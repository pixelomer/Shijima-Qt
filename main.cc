#include <QApplication>
#include <QDir>
#include <shijima/log.hpp>
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
    #ifdef _WIN32
        freopen("shijima_stdout.txt", "a", stdout);
        freopen("shijima_stderr.txt", "a", stderr);
    #endif
    #ifdef SHIJIMA_LOGGING_ENABLED
        shijima::set_log_level(SHIJIMA_LOG_PARSER | SHIJIMA_LOG_WARNINGS);
    #endif
    QApplication app(argc, argv);
    ShijimaManager::defaultManager()->show();
    int ret = app.exec();
    ShijimaManager::finalize();
    AssetLoader::finalize();
    std::cout << std::endl;
    return ret;
}