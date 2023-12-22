#include "loginwindow.h"
#include "ui_LoginWindow.h"
#include <QMessageBox>
#include <Message/Request.pb.h>
#include "registerwindow.h"
#include "src/framework/service/RpcCall.h"
#include "src/framework/config/AppConfig.h"
#include <QTimer>
#include <QTimerEvent>

#include "src/framework/defination/ImageDefination.h"
#include "src/framework/service/CachedProtoData.h"

LoginWindow::LoginWindow(QWidget *parent) :
    QDialog(parent), ui(new Ui::LoginWindow) {
    ui->setupUi(this);

    this->setWindowFlags(this->windowFlags() & ~Qt::WindowMaximizeButtonHint);
    this->setFixedSize(this->size());

    connect(ui->loginBtn,&QPushButton::clicked, this, &LoginWindow::onLoginBtnClicked);
    connect(ui->registerBtn, &QPushButton::clicked, this, &LoginWindow::onRegisterBtnClicked);

    ui->label->setPixmap(QPixmap::fromImage(QImage(Image::ImageUser).scaled({32,32})));
    ui->label_2->setPixmap(QPixmap::fromImage(QImage(Image::ImagePassword).scaled({32,32})));

    static QTimer timer;
    connect(&timer,&QTimer::timeout,this,[this]() {
        verifyCachedAccount();
    });
    timer.setSingleShot(true);
    timer.start(300);
}

LoginWindow::~LoginWindow() {
    delete ui;
}

void LoginWindow::verifyCachedAccount() {
    auto cachedUserName = AppConfig::getInstance().getSecurityConfig(AppConfig::SecurityConfigItem::eCachedUserName);
    auto cachedPassword = AppConfig::getInstance().getSecurityConfig(AppConfig::SecurityConfigItem::eCachedPassword);
    auto accountExpiredTime = AppConfig::getInstance().getSecurityConfig(AppConfig::SecurityConfigItem::eAccountExpiredTime);

    auto timestampeNow = std::chrono::system_clock::now().time_since_epoch().count();
    long long timestampeAccountExpired = 0;
    if(!accountExpiredTime.empty()) {
        timestampeAccountExpired = std::stoll(accountExpiredTime);
    }

    if(!cachedUserName.empty() && !cachedPassword.empty() && timestampeAccountExpired > timestampeNow) {
        doLogin(QString::fromStdString(cachedUserName), QString::fromStdString(cachedPassword), true);
    }
}

void LoginWindow::onLoginBtnClicked(bool checked) {
    if(ui->userNameEditor->text().trimmed().isEmpty()) {
        QMessageBox::warning(this,"Error","User Name cannot be empty!");
        return;
    }
    if(ui->passwordEditor->text().trimmed().isEmpty()) {
        QMessageBox::warning(this,"Error","Password cannot be empty!");
        return;
    }
    doLogin(ui->userNameEditor->text().trimmed(), ui->passwordEditor->text().trimmed());
}

void LoginWindow::onRegisterBtnClicked(bool checked) {
    RegisterWindow registerWindow{this};
    registerWindow.exec();
}

bool LoginWindow::doLogin(QString userName, QString password, bool slientMode) {
    grpc::ClientContext context;
    proto::UserLoginRequest request;
    request.set_username(userName.toStdString());
    request.set_password(password.toStdString());

    auto& rpcCall = RpcCall::getInstance();
    proto::UserLoginResponse response;
    auto status = rpcCall.Stub()->UserLogin(&context, request, &response);
    if(status.ok()){
        if(response.status()) {
            if(!slientMode) {
                QMessageBox::information(this,"Info","Login Successfully!");
            }
            AppConfig::getInstance().setSecurityConfig(AppConfig::SecurityConfigItem::eCachedUserName, userName.toStdString());
            AppConfig::getInstance().setSecurityConfig(AppConfig::SecurityConfigItem::eCachedPassword, password.toStdString());

            auto timestampeNow = std::chrono::system_clock::now();
            std::chrono::days days(15);
            auto expiredTime = timestampeNow + days;
            auto seconds_since_epoch = expiredTime.time_since_epoch().count();

            AppConfig::getInstance().setSecurityConfig(AppConfig::SecurityConfigItem::eAccountExpiredTime, std::to_string(seconds_since_epoch));

            AppConfig::getInstance().writeSecurityConfig();

            CachedProtoData::getInstance().CachedUserMetaInfo = response.userinfo();
            CachedProtoData::getInstance().OnlineStatus = true;

            accept();
            return true;
        }else {
            QMessageBox::warning(this,"Info","Login Failed!" + QString::fromStdString(response.message()));
        }

    }else{
        QMessageBox::critical(this,"Error",QString::fromStdString(status.error_message()));
    }
    return false;
}
