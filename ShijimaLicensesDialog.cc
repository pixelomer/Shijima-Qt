#include "ShijimaLicensesDialog.hpp"
#include <QVBoxLayout>
#include <QWidget>
#include "licenses_generated.hpp"
#include <QMargins>

ShijimaLicensesDialog::ShijimaLicensesDialog(QWidget *parent): QDialog(parent) {
    auto windowLayout = new QVBoxLayout { this };
    m_textEdit.setParent(this);
    m_textEdit.setReadOnly(true);
    m_textEdit.setPlainText(QString::fromStdString(shijima_licenses));
    setMinimumWidth(480);
    setMinimumHeight(480);
    setWindowTitle("Licenses");
    setLayout(windowLayout);
    windowLayout->setContentsMargins(QMargins {});
    windowLayout->addWidget(&m_textEdit);
}
