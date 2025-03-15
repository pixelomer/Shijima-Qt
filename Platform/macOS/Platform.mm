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

#include "../Platform.hpp"
#include <QWidget>
#include <AppKit/AppKit.h>

namespace Platform {

void initialize(int argc, char **argv) {}

void showOnAllDesktops(QWidget *widget) {
    NSView *view = (__bridge NSView *)((void *)widget->winId());
    NSWindow *window = [view window];
    [window setCollectionBehavior:[window collectionBehavior] |
        NSWindowCollectionBehaviorFullScreenAuxiliary |
        NSWindowCollectionBehaviorCanJoinAllSpaces];
}

bool useWindowMasks() {
    return false;
}

}
