#pragma once

#include <QWidget>
#include <cstdint>
#include "Platform/Platform.hpp"

template<typename T>
class PlatformWidget : public T {
public:
    enum Flags : uint32_t {
        ShowOnAllDesktops = 0x1
    };
    PlatformWidget(): T(), m_flags(0) {}
    PlatformWidget(QWidget *parent): T(parent), m_flags(0) {}
    PlatformWidget(QWidget *parent, Flags flags): T(parent) {
        m_flags = flags;
    }
private:
    uint32_t m_flags;
protected:
    void showEvent(QShowEvent *event) override {
        T::showEvent(event);
        if (m_flags & ShowOnAllDesktops) {
            Platform::showOnAllDesktops(this);
        }
    }
};