#pragma once

#include <QDialog>


QT_BEGIN_NAMESPACE
namespace Ui { class RegisterWindow; }
QT_END_NAMESPACE

class RegisterWindow : public QDialog {
Q_OBJECT

public:
    explicit RegisterWindow(QWidget *parent = nullptr);
    ~RegisterWindow() override;

    void onRegisterBtnClicked(bool checked);

private:
    Ui::RegisterWindow *ui;
};

