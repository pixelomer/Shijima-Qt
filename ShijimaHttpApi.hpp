#pragma once
#include <string>
#include <thread>

class ShijimaManager;

namespace httplib {
    class Server;
}

class ShijimaHttpApi {
private:
    httplib::Server *m_server;
    std::thread *m_thread;
    ShijimaManager *m_manager;
public:
    void start(std::string const& host, int port);
    void stop();
    ShijimaHttpApi(ShijimaManager *manager);
    ~ShijimaHttpApi();
};
