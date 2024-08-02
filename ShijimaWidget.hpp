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
    struct ActiveImage {
    public:
        QImage const* image;
        QPoint drawOrigin;
        QRect sourceRect;
        bool available;
        ActiveImage(): available(false) {}
        ActiveImage(QImage const* image, QPoint const& origin,
            QRect const& rect): image(image), drawOrigin(origin),
            sourceRect(rect), available(true) {}
    };
    void closeAction();
    void contextMenuClosed(QCloseEvent *);
    void showContextMenu(QPoint const&);
    ActiveImage const& getActiveImage();
    std::unique_ptr<shijima::mascot::manager> m_mascot;
    std::string m_mascotName;
    std::string m_imgRoot;
    int m_offsetX;
    int m_offsetY;
    bool m_visible;
    bool m_contextMenuVisible = false;
    bool m_paused = false;
    bool m_markedForDeletion = false;
    ActiveImage m_activeImage;
};