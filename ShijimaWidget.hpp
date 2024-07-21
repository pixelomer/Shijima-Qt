#pragma once

#include <QWidget>
#include <memory>
#include <shijima/mascot/manager.hpp>
#include <shijima/mascot/environment.hpp>

class QPushButton;
class QPaintEvent;
class QMouseEvent;
class QCloseEvent;
class ShijimaContextMenu;

class ShijimaWidget : public QWidget
{
public:
    friend class ShijimaContextMenu;
    explicit ShijimaWidget(std::shared_ptr<shijima::mascot::environment> env,
        QWidget *parent = nullptr);
    void tick();
    bool paused() const { return m_paused || m_contextMenuVisible; }
protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
private:
    void closeAction();
    void contextMenuClosed(QCloseEvent *);
    void showContextMenu(QPoint const&);
    std::unique_ptr<shijima::mascot::manager> m_mascot;
    std::shared_ptr<shijima::mascot::environment> m_env;
    int m_offsetX;
    int m_offsetY;
    bool m_visible;
    bool m_contextMenuVisible = false;
    bool m_paused = false;
};