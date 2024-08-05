#pragma once

#include <QWidget>
#include <memory>
#include "Asset.hpp"
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
    explicit ShijimaWidget(std::string const& mascotName,
        std::string const& imgRoot,
        std::unique_ptr<shijima::mascot::manager> mascot,
        QWidget *parent = nullptr);
    void tick();
    void markForDeletion() { m_markedForDeletion = true; }
    bool paused() const { return m_paused || m_contextMenuVisible; }
    shijima::mascot::manager &mascot() {
        return *m_mascot;
    }
    void setEnv(std::shared_ptr<shijima::mascot::environment> env) {
        m_mascot->state->env = env;
    }
    std::shared_ptr<shijima::mascot::environment> env() {
        return m_mascot->state->env; 
    }
    std::string const& mascotName() {
        return m_mascotName;
    }
protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
private:
    void closeAction();
    void contextMenuClosed(QCloseEvent *);
    void showContextMenu(QPoint const&);
    bool updateOffsets();
    Asset const& getActiveAsset();
    std::unique_ptr<shijima::mascot::manager> m_mascot;
    std::string m_mascotName;
    std::string m_imgRoot;
    QRect m_imageRect;
    QPoint m_anchorInWindow;
    QPoint m_drawOffset;
    int m_windowHeight;
    int m_windowWidth;
    bool m_visible;
    bool m_contextMenuVisible = false;
    bool m_paused = false;
    bool m_markedForDeletion = false;
};