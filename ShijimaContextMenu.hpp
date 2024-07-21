#pragma once
#include <QMenu>
#include "ShijimaWidget.hpp"

class QCloseEvent;
class QActionEvent;

class ShijimaContextMenu : public QMenu {
public:
    ShijimaWidget *shijimaParent() {
        return static_cast<ShijimaWidget *>(parent());
    }
    explicit ShijimaContextMenu(ShijimaWidget *parent);
protected:
    void closeEvent(QCloseEvent *) override;
};