#pragma once
#include <QDialog>
#include <QPlainTextEdit>

class ShijimaLicensesDialog : public QDialog {
public:
    ShijimaLicensesDialog(QWidget *parent);
private:
    QPlainTextEdit m_textEdit;
};