#pragma once

// 
// Shijima-Qt - Cross-platform shimeji simulation app for desktop
// Copyright (C) 2025 pixelomer
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// 

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
