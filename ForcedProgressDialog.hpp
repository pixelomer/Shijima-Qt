#pragma once
#include <QProgressDialog>

class ForcedProgressDialog : public QProgressDialog {
private:
    bool m_allowsClose = false;
public:
    ForcedProgressDialog(QWidget *parent = nullptr): QProgressDialog(parent) {}
    bool close();
    void closeEvent(QCloseEvent *) override;
};
