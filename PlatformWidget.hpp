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

#include <QWidget>
#include <cstdint>
#include <QTimer>
#include <QApplication>
#include "Platform/Platform.hpp"

template<typename T>
class PlatformWidget : public T {
public:
    enum Flags : uint32_t {
        ShowOnAllDesktops = 0x1
    };
    PlatformWidget(QWidget *parent = nullptr, Flags flags = 0): T(parent) {
        setupDispatchTimer();
        m_flags = flags;
    }
private:
    uint32_t m_flags;
    QTimer m_dispatchTimer;
    void setupDispatchTimer() {
        this->connect(&m_dispatchTimer, &QTimer::timeout, this, &PlatformWidget::showEventDispatch);
        m_dispatchTimer.setInterval(0);
        m_dispatchTimer.setSingleShot(true);
        m_dispatchTimer.moveToThread(qApp->thread());
    }
    void showEventDispatch() {
        if (this->winId() != 0) {
            if (m_flags & ShowOnAllDesktops) {
                Platform::showOnAllDesktops(this);
            }
        }
    }
protected:
    void showEvent(QShowEvent *event) override {
        T::showEvent(event);
        showEventDispatch();
        m_dispatchTimer.start();
    }
};