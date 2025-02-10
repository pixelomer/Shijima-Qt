#pragma once

#include <QDialog>
#include <QString>
#include <functional>
#include <string>

class ShijimaWidget;
class QFormLayout;
class QShowEvent;

namespace shijima {
    namespace mascot {
        class manager;
    }
}

class ShimejiInspectorDialog : public QDialog {
private:
    std::vector<std::function<void()>> m_tickCallbacks;
    QFormLayout *m_formLayout;
    void addRow(QString const& label,
        std::function<std::string(shijima::mascot::manager &)> tick);
protected:
    void showEvent(QShowEvent *event) override;
public:
    ShijimaWidget *shijimaParent();
    ShimejiInspectorDialog(ShijimaWidget *parent);
    void tick();
};
