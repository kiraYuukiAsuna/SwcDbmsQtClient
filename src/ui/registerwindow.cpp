#include "registerwindow.h"

#include <QMessageBox>
#include <grpcpp/client_context.h>
#include <Message/Request.pb.h>

#include "ui_RegisterWindow.h"
#include "src/framework/defination/ImageDefination.h"
#include "src/framework/service/RpcCall.h"


RegisterWindow::RegisterWindow(QWidget *parent) :
    QDialog(parent), ui(new Ui::RegisterWindow) {
    ui->setupUi(this);
    this->setWindowIcon(QIcon(Image::ImageCreateUser));
    this->setWindowFlags(this->windowFlags() & ~Qt::WindowMaximizeButtonHint);
    this->setFixedSize(this->size());

    connect(ui->registerBtn,&QPushButton::clicked, this, &RegisterWindow::onRegisterBtnClicked);

    ui->label->setPixmap(QPixmap::fromImage(QImage(Image::ImageCreateUser).scaled({32,32})));
    ui->label_2->setPixmap(QPixmap::fromImage(QImage(Image::ImagePassword).scaled({32,32})));
    ui->label_3->setPixmap(QPixmap::fromImage(QImage(Image::ImagePassword).scaled({32,32})));

}

RegisterWindow::~RegisterWindow() {
    delete ui;
}

void RegisterWindow::onRegisterBtnClicked(bool checked) {
    if(ui->userNameEditor->text().trimmed().isEmpty()) {
        QMessageBox::warning(this,"Error","User Name cannot be empty!");
        return;
    }
    if(ui->passwordEditor->text().trimmed().isEmpty() || ui->repeatPasswordEditor->text().trimmed().isEmpty()) {
        QMessageBox::warning(this,"Error","Password cannot be empty!");
        return;
    }
    if(ui->passwordEditor->text().trimmed() != ui->repeatPasswordEditor->text().trimmed()) {
        QMessageBox::warning(this,"Error","Password and Repeated Password are not equal!");
        return;
    }

    grpc::ClientContext context;
    proto::CreateUserRequest request;
    request.mutable_userinfo()->set_name(ui->userNameEditor->text().trimmed().toStdString());
    request.mutable_userinfo()->set_password(ui->repeatPasswordEditor->text().trimmed().toStdString());

    auto& rpcCall = RpcCall::getInstance();
    proto::CreateUserResponse response;
    auto status = rpcCall.Stub()->CreateUser(&context, request, &response);
    if(status.ok()){
        if(response.status()) {
            QMessageBox::information(this,"Info","Register Successfully!");
            accept();
        }else {
            QMessageBox::warning(this,"Info","Register Failed!" + QString::fromStdString(response.message()));
        }

    }else{
        QMessageBox::critical(this,"Error",QString::fromStdString(status.error_message()));
    }
}
