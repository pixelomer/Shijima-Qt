#pragma once

#include <QWidget>
#include <cstdint>

class PlatformWidget : public QWidget {
public:
    enum Flags : uint32_t {
        ShowOnAllDesktops = 0x1
    };
    PlatformWidget();
    PlatformWidget(QWidget *parent);
    PlatformWidget(QWidget *parent, Flags flags);
private:
    uint32_t m_flags;
protected:
    void showEvent(QShowEvent *) override;
};