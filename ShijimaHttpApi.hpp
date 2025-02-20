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
    std::string m_host;
    int m_port;
public:
    void start(std::string const& host, int port);
    void stop();
    bool running();
    int port();
    std::string const& host();
    ShijimaHttpApi(ShijimaManager *manager);
    ~ShijimaHttpApi();
};
