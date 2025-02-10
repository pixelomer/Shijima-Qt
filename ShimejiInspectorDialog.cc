#include "ShimejiInspectorDialog.hpp"
#include "ShijimaWidget.hpp"
#include <QFormLayout>
#include <string>
#include <QLabel>

static std::string doubleToString(double val) {
    auto str = std::to_string(val);
    auto dot = str.rfind('.');
    if (dot != std::string::npos) {
        str = str.substr(0, dot + 3);
    }
    return str;
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

    addRow("X", [](shijima::mascot::manager &mascot){
        return doubleToString(mascot.state->anchor.x);
    });
    addRow("Y", [](shijima::mascot::manager &mascot){
        return doubleToString(mascot.state->anchor.y);
    });
    addRow("Behavior", [](shijima::mascot::manager &mascot){
        return mascot.active_behavior()->name;
    });
    addRow("Image", [](shijima::mascot::manager &mascot){
        return mascot.state->active_frame.get_name(mascot.state->looking_right);
    });
}

void ShimejiInspectorDialog::addRow(QString const& label,
    std::function<std::string(shijima::mascot::manager &)> tick)
{
    auto labelWidget = new QLabel { label };
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
