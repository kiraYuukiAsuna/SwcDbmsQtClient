#pragma once

#include <QDialog>


QT_BEGIN_NAMESPACE
namespace Ui { class LoginWindow; }
QT_END_NAMESPACE

class LoginWindow : public QDialog {
Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow() override;

    void verifyCachedAccount();

    void onLoginBtnClicked(bool checked);
    void onRegisterBtnClicked(bool checked);

private:
    bool doLogin(QString userName, QString password, bool slientMode = false);

    Ui::LoginWindow *ui;
};
