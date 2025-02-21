#include <QApplication>
#include <QDir>
#include <QStandardPaths>
#include <QMessageBox>
#include <shijima/log.hpp>
#include "Platform/Platform.hpp"
#include "ShijimaManager.hpp"
#include "AssetLoader.hpp"
#include "cli.hpp"
#include <httplib.h>

int main(int argc, char **argv) {
    if (argc > 1) {
        return shijimaRunCli(argc, argv);
    }
    Platform::initialize(argc, argv);
    #ifdef SHIJIMA_LOGGING_ENABLED
        shijima::set_log_level(SHIJIMA_LOG_PARSER | SHIJIMA_LOG_WARNINGS);
    #endif
    QApplication app(argc, argv);
    app.setApplicationName("Shijima-Qt");
    app.setApplicationDisplayName("Shijima-Qt");
    try {
        httplib::Client pingClient { "http://127.0.0.1:32456" };
        pingClient.set_connection_timeout(0, 500000);
        auto pingResult = pingClient.Get("/shijima/api/v1/ping");
        if (pingResult != nullptr) {
            throw std::runtime_error("Shijima-Qt is already running!");
        }
        ShijimaManager::defaultManager()->show();
    }
    catch (std::exception &ex) {
        QMessageBox *msg = new QMessageBox {};
        msg->setText("Shijima-Qt failed to start. Reason: " +
            QString::fromUtf8(ex.what()));
        msg->setStandardButtons(QMessageBox::StandardButton::Close);
        msg->setAttribute(Qt::WA_DeleteOnClose);
        msg->show();
    }
    int ret = app.exec();
    ShijimaManager::finalize();
    AssetLoader::finalize();
    return ret;
}
