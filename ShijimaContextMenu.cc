#include "ShijimaContextMenu.hpp"
#include "ShijimaWidget.hpp"

ShijimaContextMenu::ShijimaContextMenu(ShijimaWidget *parent)
    : QMenu("Context menu", parent)
{
    QAction *action;

    // Behaviors menu   
    {
        std::vector<std::string> behaviors;
        auto &list = parent->m_mascot->initial_behavior_list();
        auto flat = list.flatten_unconditional();
        for (auto &behavior : flat) {
            if (!behavior->hidden) {
                behaviors.push_back(behavior->name);
            }
        }
        auto behaviorsMenu = addMenu("Behaviors");
        for (std::string &behavior : behaviors) {
            action = behaviorsMenu->addAction(QString::fromStdString(behavior));
            connect(action, &QAction::triggered, [this, behavior](){
                shijimaParent()->m_mascot->next_behavior(behavior);
            });
        }
    }

    // Pause checkbox
    action = addAction("Pause");
    action->setCheckable(true);
    action->setChecked(parent->m_paused);
    connect(action, &QAction::triggered, [this](bool checked){
        shijimaParent()->m_paused = checked;
    });

    // Close button
    action = addAction("Kill");
    connect(action, &QAction::triggered, parent, &ShijimaWidget::closeAction);
}

void ShijimaContextMenu::closeEvent(QCloseEvent *event) {
    shijimaParent()->contextMenuClosed(event);
    QMenu::closeEvent(event);
}

/*
ShijimaContextMenu::~ShijimaContextMenu() {
    auto allActions = actions();
    for (QAction *action : allActions) {
        removeAction(action);
        delete action;
    }
}
*/