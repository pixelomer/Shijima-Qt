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

#include "ShijimaWidget.hpp"
#include <QWidget>
#include <QPainter>
#include <QFile>
#include <QDir>
#include <QScreen>
#include <QMouseEvent>
#include <QMenu>
#include <QWindow>
#include <QDebug>
#include <QGuiApplication>
#include <QTextStream>
#include <shijima/shijima.hpp>
#include "ActiveMascot.hpp"
#include "Platform/Platform.hpp"
#include "ShimejiInspectorDialog.hpp"
#include "AssetLoader.hpp"
#include "MascotBackendWidgets.hpp"
#include "ShijimaContextMenu.hpp"
#include "ShijimaManager.hpp"
#include <shimejifinder/utils.hpp>

using namespace shijima;

ShijimaWidget::ShijimaWidget(MascotBackendWidgets *backend,
    MascotData *mascotData,
    std::unique_ptr<shijima::mascot::manager> mascot,
    int mascotId, QWidget *parent):
#if defined(__APPLE__)
    PlatformWidget(nullptr, PlatformWidget::ShowOnAllDesktops),
#else
    PlatformWidget(parent, PlatformWidget::ShowOnAllDesktops),
#endif
    ActiveMascot(mascotData, std::move(mascot), mascotId),
    m_backend(backend)
{
    widgetSetup();
}

ShijimaWidget::ShijimaWidget(MascotBackendWidgets *backend,
    ActiveMascot &old, QWidget *parent):
#if defined(__APPLE__)
    PlatformWidget(nullptr, PlatformWidget::ShowOnAllDesktops),
#else
    PlatformWidget(parent, PlatformWidget::ShowOnAllDesktops),
#endif
    ActiveMascot(old), m_backend(backend)
{
    widgetSetup();
}

void ShijimaWidget::widgetSetup() {
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_MacShowFocusRect, false);
    Qt::WindowFlags flags = Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint
        | Qt::WindowDoesNotAcceptFocus | Qt::NoDropShadowWindowHint
        | Qt::WindowOverridesSystemGestures;
    #if defined(__APPLE__)
    flags |= Qt::Window;
    #else
    flags |= Qt::Tool;
    #endif
    setWindowFlags(flags);
    setFixedSize(container().width(), container().height());
}

void ShijimaWidget::paintEvent(QPaintEvent *) {
    if (!visible()) {
        return;
    }
    auto &asset = getActiveAsset();
    auto &image = asset.image(isMirroredRender());
    auto scaledSize = image.size() / drawScale();
    QPainter painter(this);
    painter.drawImage(QRect { drawOrigin(), scaledSize }, image);
#ifdef __linux__
    if (Platform::useWindowMasks()) {
        m_windowMask = QBitmap::fromPixmap(asset.mask(isMirroredRender())
            .scaled(scaledSize));
        m_windowMask.translate(drawOrigin());
        auto bounding = m_windowMask.boundingRect();
        bounding.setTop(0);
        bounding.setLeft(0);
        if (bounding.width() > 0 && bounding.height() > 0) {
            setMask(m_windowMask);
        }
        else {
            setMask(QRect { container().height() - 2,
                container().width() - 2, 1, 1 });
        }
    }
#endif
}

bool ShijimaWidget::tick() {
    if (markedForDeletion()) {
        close();
        return false;
    }
    auto &reverseEnv = m_backend->reverseEnv();
    if (!reverseEnv.contains(mascot().state->env.get())) {
        mascot().state->env = m_backend->env()[m_backend->spawnScreen()];
    }
    if (ActiveMascot::tick()) {
        repaint();
        update();

        if (mascot().state->dragging) {
            auto &reverseEnv = m_backend->reverseEnv();
            auto &env = m_backend->env();
            auto oldScreen = reverseEnv[mascot().state->env.get()];
            auto newScreen = QGuiApplication::screenAt(QPoint {
                (int)mascot().state->anchor.x, (int)mascot().state->anchor.y });
            if (newScreen != nullptr && oldScreen != newScreen) {
                mascot().state->env = env[newScreen];
            }
        }
        return true;
    }
    return false;
}

void ShijimaWidget::show() {
    PlatformWidget::show();
}

bool ShijimaWidget::mascotClosed() {
    return !isVisible();
}

ShijimaWidget::~ShijimaWidget() {}

bool ShijimaWidget::updateOffsets() {
    bool needsRepaint = ActiveMascot::updateOffsets();
    auto newRect = container();
    move(newRect.x(), newRect.y());
    if (needsRepaint) {
        setFixedSize(newRect.width(), newRect.height());
    }
    return needsRepaint;
}

void ShijimaWidget::mousePressEvent(QMouseEvent *event) {
    bool handled = ActiveMascot::mousePressEvent(event->button(),
        event->pos(), mapToGlobal(event->pos()));
    if (!handled) {
        event->ignore();
    }
}

void ShijimaWidget::mouseReleaseEvent(QMouseEvent *event) {
    ActiveMascot::mouseReleaseEvent(event->button());
}
