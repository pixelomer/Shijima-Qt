#pragma once

#include <QWidget>
#include <memory>
#include "Asset.hpp"
#include "SoundEffectManager.hpp"
#include <shijima/mascot/manager.hpp>
#include <shijima/mascot/environment.hpp>
#include "PlatformWidget.hpp"
#include "MascotData.hpp"

class QPushButton;
class QPaintEvent;
class QMouseEvent;
class QCloseEvent;
class ShijimaContextMenu;
class ShimejiInspectorDialog;

class ShijimaWidget : public PlatformWidget<QWidget>
{
public:
    friend class ShijimaContextMenu;
    explicit ShijimaWidget(MascotData *mascotData,
        std::unique_ptr<shijima::mascot::manager> mascot,
        int mascotId, QWidget *parent = nullptr);
    void tick();
    bool pointInside(QPoint const& point);
    int mascotId() { return m_mascotId; }
    void showInspector();
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
    MascotData *mascotData() {
        return m_data;
    }
    QString const& mascotName() {
        return m_data->name();
    }
    ~ShijimaWidget();
protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
private:
    void setDragTarget(ShijimaWidget *target);
    bool isMirroredRender() const;
    void closeAction();
    void contextMenuClosed(QCloseEvent *);
    void showContextMenu(QPoint const&);
    bool updateOffsets();
    MascotData *m_data;
    ShimejiInspectorDialog *m_inspector;
    SoundEffectManager m_sounds;
    Asset const& getActiveAsset();
    ShijimaWidget *m_dragTarget = nullptr;
    ShijimaWidget **m_dragTargetPt = nullptr;
    std::unique_ptr<shijima::mascot::manager> m_mascot;
    QRect m_imageRect;
    QPoint m_anchorInWindow;
    double m_drawScale = 1.0;
    QPoint m_drawOrigin;
    int m_windowHeight;
    int m_windowWidth;
    bool m_visible;
    bool m_contextMenuVisible = false;
    bool m_paused = false;
    bool m_markedForDeletion = false;
    int m_mascotId;
};