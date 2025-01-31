#pragma once
#include <QProgressDialog>

class ForcedProgressDialog : public QProgressDialog {
private:
    bool m_allowsClose = false;
protected:
    void closeEvent(QCloseEvent *) override;
public:
    ForcedProgressDialog(QWidget *parent = nullptr): QProgressDialog(parent) {}
    bool close();
};
