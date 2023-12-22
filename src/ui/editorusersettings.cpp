#include <QMessageBox>
#include <QBuffer>
#include <QFileDialog>
#include <QStandardPaths>
#include "editorusersettings.h"
#include "ui_EditorUserSettings.h"
#include "Service//Service.pb.h"
#include "Service/Service.grpc.pb.h"
#include "src/framework/service/RpcCall.h"
#include "src/framework/service/CachedProtoData.h"
#include "src/framework/defination/ImageDefination.h"
#include "leftclientview.h"

EditorUserSettings::EditorUserSettings(LeftClientView *leftClientView) :
        QDialog(leftClientView), ui(new Ui::EditorUserSettings) {
    ui->setupUi(this);
    setWindowIcon(QIcon(Image::ImageUser));
    m_LeftClientView = leftClientView;

    ui->ChangeHeadPhoto->setIcon(QIcon(Image::ImageEdit));
    connect(ui->ChangeHeadPhoto,&QPushButton::clicked,this,[this](){
        QFileDialog dialog;
        dialog.setWindowTitle("Select your head photo");
        dialog.setDirectory(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
        dialog.setNameFilter(tr("File(*.png)"));
        dialog.setFileMode(QFileDialog::ExistingFile);
        dialog.setViewMode(QFileDialog::Detail);
        if(dialog.exec()){
            if(dialog.selectedFiles().size() < 1){
                return;
            }
            QString fileName = dialog.selectedFiles()[0];
            QPixmap pixmap;
            pixmap.load(fileName);
            pixmap = pixmap.scaled(QSize(128,128),Qt::KeepAspectRatioByExpanding);
            ui->HeadPhoto->setPixmap(pixmap);
        }
    });
    connect(ui->CancelBtn,&QPushButton::clicked,this,[this](){
        reject();
    });
    connect(ui->OKBtn,&QPushButton::clicked,this,[this](){
        proto::UpdateUserRequest request;
        proto::UpdateUserResponse response;
        request.mutable_userinfo()->CopyFrom(CachedProtoData::getInstance().CachedUserMetaInfo);

        if(ui->Description->text().trimmed().isEmpty()){
            QMessageBox::warning(this,"Error","Description cannot be empty!");
            return;
        }
        request.mutable_userinfo()->set_description(ui->Description->text().trimmed().toStdString());

        if(ui->Password->text().trimmed().isEmpty()){
            QMessageBox::warning(this,"Error","Password cannot be empty!");
            return;
        }
        request.mutable_userinfo()->set_password(ui->Password->text().trimmed().toStdString());

        QBuffer buffer;
        if(buffer.open(QIODevice::WriteOnly)){
            ui->HeadPhoto->pixmap().save(&buffer,"png");
            request.mutable_userinfo()->set_headphotobindata(QByteArray(buffer.data()).toStdString());
        }

        grpc::ClientContext context;
        auto status = RpcCall::getInstance().Stub()->UpdateUser(&context,request,&response);
        if(status.ok()){
            if(response.status()){
                CachedProtoData::getInstance().CachedUserMetaInfo.CopyFrom(response.userinfo());
                QMessageBox::information(this,"Notice","Update user info success!");
                m_LeftClientView->refreshTree();
                accept();
            }else{
                QMessageBox::critical(this,"Error",QString::fromStdString(response.message()));
            }
        }else{
            QMessageBox::critical(this,"Error",QString::fromStdString(status.error_message()));
        }
    });
    getUserMetaInfo();
}

EditorUserSettings::~EditorUserSettings() {
    delete ui;
}

void EditorUserSettings::getUserMetaInfo() {
    proto::GetUserRequest request;
    request.mutable_userinfo()->set_name(CachedProtoData::getInstance().CachedUserMetaInfo.name());

    proto::GetUserResponse response;

    grpc::ClientContext context;

    auto result = RpcCall::getInstance().Stub()->GetUser(&context,request,&response);
    if(result.ok()) {
        CachedProtoData::getInstance().CachedUserMetaInfo.CopyFrom(response.userinfo());

        ui->Id->setText(QString::fromStdString(response.userinfo().base()._id()));
        ui->Id->setReadOnly(true);
        ui->Uuid->setText(QString::fromStdString(response.userinfo().base().uuid()));
        ui->Uuid->setReadOnly(true);
        ui->ApiVersion->setText(QString::fromStdString(response.userinfo().base().apiversion()));
        ui->ApiVersion->setReadOnly(true);
        ui->Name->setText(QString::fromStdString(response.userinfo().name()));
        ui->Name->setReadOnly(true);
        ui->Description->setText(QString::fromStdString(response.userinfo().description()));
        ui->Password->setText(QString::fromStdString(response.userinfo().password()));
        ui->CreateTime->setDateTime(QDateTime::fromSecsSinceEpoch(response.userinfo().createtime().seconds()));
        ui->CreateTime->setReadOnly(true);
        QPixmap pixmap;
        auto rawdata = response.userinfo().headphotobindata();
        auto data = QByteArray::fromStdString(rawdata);
        pixmap.loadFromData(data);
        pixmap = pixmap.scaled(QSize(128,128),Qt::KeepAspectRatioByExpanding);
        ui->HeadPhoto->setPixmap(pixmap);
        ui->PermissionGroup->setText(QString::fromStdString(response.userinfo().userpermissiongroup()));
        ui->PermissionGroup->setReadOnly(true);

    }else{
        QMessageBox::critical(this,"Error",QString::fromStdString(result.error_message()));
    }
}
