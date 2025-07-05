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

#include <QString>

namespace Platform {

class ActiveWindow {
public:
    bool available;
    QString uid;
    long pid;
    double x, y, width, height;
    ActiveWindow(QString const& uid, long pid, double x, double y,
        double width, double height):
        available(true), uid(uid), pid(pid), x(x), y(y), width(width),
        height(height) {}
    ActiveWindow(): available(false) {}
};

}
