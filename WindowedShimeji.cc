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

#include "WindowedShimeji.hpp"
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
#include "MascotBackendWindowed.hpp"
#include "ShijimaManager.hpp"
#include <shimejifinder/utils.hpp>

using namespace shijima;

WindowedShimeji::WindowedShimeji(MascotBackendWindowed *backend,
    MascotData *mascotData,
    std::unique_ptr<shijima::mascot::manager> mascot,
    int mascotId, QWidget *parent):
    QWidget(parent),
    ActiveMascot(mascotData, std::move(mascot), mascotId),
    m_backend(backend)
{
    widgetSetup();
}

WindowedShimeji::WindowedShimeji(MascotBackendWindowed *backend,
    ActiveMascot &old, QWidget *parent):
    QWidget(parent), ActiveMascot(old), m_backend(backend)
{
    widgetSetup();
}

void WindowedShimeji::widgetSetup() {
    setFixedSize(container().width(), container().height());
}

void WindowedShimeji::paintEvent(QPaintEvent *) {
    if (!visible()) {
        return;
    }
    auto &asset = getActiveAsset();
    auto &image = asset.image(isMirroredRender());
    auto scaledSize = image.size() / drawScale();
    QPainter painter(this);
    painter.drawImage(QRect { drawOrigin(), scaledSize }, image);
}

bool WindowedShimeji::tick() {
    if (markedForDeletion()) {
        close();
        hide();
        return false;
    }
    if (ActiveMascot::tick()) {
        repaint();
        update();
        return true;
    }
    return false;
}

void WindowedShimeji::show() {
    QWidget::show();
}

bool WindowedShimeji::mascotClosed() {
    return !isVisible();
}

WindowedShimeji::~WindowedShimeji() {}

bool WindowedShimeji::updateOffsets() {
    bool needsRepaint = ActiveMascot::updateOffsets();
    auto newRect = container();
    move(newRect.x(), newRect.y());
    if (needsRepaint) {
        setFixedSize(newRect.width(), newRect.height());
    }
    return needsRepaint;
}

void WindowedShimeji::mousePressEvent(QMouseEvent *event) {
    bool handled = ActiveMascot::mousePressEvent(event->button(),
        event->pos(), mapToGlobal(event->pos()));
    if (!handled) {
        event->ignore();
    }
}

void WindowedShimeji::mouseReleaseEvent(QMouseEvent *event) {
    ActiveMascot::mouseReleaseEvent(event->button());
}
