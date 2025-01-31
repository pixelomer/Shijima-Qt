#include "ForcedProgressDialog.hpp"
#include <QCloseEvent>

void ForcedProgressDialog::closeEvent(QCloseEvent *event) {
    if (!m_allowsClose) {
        event->ignore();
    }
}

bool ForcedProgressDialog::close() {
    m_allowsClose = true;
    return QWidget::close();
}
