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

#include "ShimejiInspectorDialog.hpp"
#include "ShijimaWidget.hpp"
#include <QFormLayout>
#include <string>
#include <QLabel>
#include <QPoint>

static std::string doubleToString(double val) {
    auto str = std::to_string(val);
    auto dot = str.rfind('.');
    if (dot != std::string::npos) {
        str = str.substr(0, dot + 3);
    }
    return str;
}

static std::string vecToString(shijima::math::vec2 const& vec) {
    return "x: " + doubleToString(vec.x) +
        ", y: " + doubleToString(vec.y);
}

static std::string vecToString(QPoint const& vec) {
    return "x: " + doubleToString(vec.x()) +
        ", y: " + doubleToString(vec.y());
}

static std::string vecToString(shijima::mascot::environment::dvec2 const& vec) {
    return "x: " + doubleToString(vec.x) +
        ", y: " + doubleToString(vec.y) +
        ", dx: " + doubleToString(vec.dx) +
        ", dy: " + doubleToString(vec.dy);
}

static std::string areaToString(shijima::mascot::environment::area const& area) {
    return "x: " + doubleToString(area.left) +
        ", y: " + doubleToString(area.top) +
        ", width: " + doubleToString(area.width()) +
        ", height: " + doubleToString(area.height());
}

static std::string areaToString(shijima::mascot::environment::darea const& area) {
    return "x: " + doubleToString(area.left) +
        ", y: " + doubleToString(area.top) +
        ", width: " + doubleToString(area.width()) +
        ", height: " + doubleToString(area.height()) + 
        ", dx: " + doubleToString(area.dx) + 
        ", dy: " + doubleToString(area.dy);
}

ShimejiInspectorDialog::ShimejiInspectorDialog(ShijimaWidget *parent):
    QDialog(parent), m_formLayout(new QFormLayout)
{
    setWindowFlags((windowFlags() | Qt::CustomizeWindowHint |
        Qt::WindowCloseButtonHint) &
        ~(Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint));
    setLayout(m_formLayout);
    m_formLayout->setFormAlignment(Qt::AlignLeft);
    m_formLayout->setLabelAlignment(Qt::AlignRight);

    addRow("Window", [this](shijima::mascot::manager &mascot){
        return vecToString(shijimaParent()->pos());
    });
    addRow("Anchor", [](shijima::mascot::manager &mascot){
        return vecToString(mascot.state->anchor);
    });
    addRow("Cursor", [](shijima::mascot::manager &mascot){
        return vecToString(mascot.state->get_cursor());
    });
    addRow("Behavior", [](shijima::mascot::manager &mascot){
        return mascot.active_behavior()->name;
    });
    addRow("Image", [](shijima::mascot::manager &mascot){
        return mascot.state->active_frame.get_name(mascot.state->looking_right);
    });
    addRow("Screen", [](shijima::mascot::manager &mascot){
        return areaToString(mascot.state->env->screen);
    });
    addRow("Work Area", [](shijima::mascot::manager &mascot){
        return areaToString(mascot.state->env->work_area);
    });
    addRow("Active IE", [](shijima::mascot::manager &mascot){
        if (mascot.state->env->active_ie.visible()) {
            return areaToString(mascot.state->env->active_ie);
        }
        else {
            return std::string { "not visible" };
        }
    });
}

void ShimejiInspectorDialog::addRow(QString const& label,
    std::function<std::string(shijima::mascot::manager &)> tick)
{
    auto labelWidget = new QLabel { label };
    labelWidget->setStyleSheet("font-weight: bold;");
    auto dataWidget = new QLabel {};
    m_tickCallbacks.push_back([this, dataWidget, tick](){
        auto newText = tick(shijimaParent()->mascot());
        dataWidget->setText(QString::fromStdString(newText));
    });
    m_formLayout->addRow(labelWidget, dataWidget);
}

ShijimaWidget *ShimejiInspectorDialog::shijimaParent() {
    return static_cast<ShijimaWidget *>(parent());
}

void ShimejiInspectorDialog::showEvent(QShowEvent *event) {
    //setMinimumSize(size());
    //setMaximumSize(size());
}

void ShimejiInspectorDialog::tick() {
    for (auto &callback : m_tickCallbacks) {
        callback();
    }
}
