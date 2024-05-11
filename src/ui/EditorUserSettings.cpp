#include <QMessageBox>
#include <QBuffer>
#include <QFileDialog>
#include <QStandardPaths>
#include "EditorUserSettings.h"
#include "ui_EditorUserSettings.h"
#include "Service//Service.pb.h"
#include "Service/Service.grpc.pb.h"
#include "src/framework/service/RpcCall.h"
#include "src/framework/service/CachedProtoData.h"
#include "src/framework/defination/ImageDefination.h"
#include "LeftClientView.h"
#include "src/framework/service/WrappedCall.h"

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
        proto::UpdateUserResponse response;

        proto::UserMetaInfoV1 userMetaInfo;
        userMetaInfo.CopyFrom(CachedProtoData::getInstance().CachedUserMetaInfo);

        userMetaInfo.set_description(ui->Description->text().trimmed().toStdString());

        if(ui->Password->text().trimmed().isEmpty()){
            QMessageBox::warning(this,"Error","Password cannot be empty!");
            return;
        }
        userMetaInfo.set_password(ui->Password->text().trimmed().toStdString());

        QBuffer buffer;
        if(buffer.open(QIODevice::WriteOnly)){
            ui->HeadPhoto->pixmap().save(&buffer,"png");
            userMetaInfo.set_headphotobindata(QByteArray(buffer.data()).toStdString());
        }

        if (WrappedCall::UpdateUser(userMetaInfo, response, this)) {
            if (response.metainfo().status()) {
                CachedProtoData::getInstance().CachedUserMetaInfo.CopyFrom(response.userinfo());
                QMessageBox::information(this, "Notice", "Update user info success!");
                m_LeftClientView->refreshTree();
                accept();
            }
            else {
                QMessageBox::critical(this, "Error", QString::fromStdString(response.metainfo().message()));
            }
        }
    });
    getUserMetaInfo();
}

EditorUserSettings::~EditorUserSettings() {
    delete ui;
}

void EditorUserSettings::getUserMetaInfo() {
    proto::GetUserByUuidResponse response;
    auto result = WrappedCall::GetUserInfoByUuid(CachedProtoData::getInstance().UserUuid,response,this);
    if(result) {
        CachedProtoData::getInstance().CachedUserMetaInfo.CopyFrom(response.userinfo());

        ui->Id->setText(QString::fromStdString(response.userinfo().base()._id()));
        ui->Id->setReadOnly(true);
        ui->Uuid->setText(QString::fromStdString(response.userinfo().base().uuid()));
        ui->Uuid->setReadOnly(true);
        ui->DataAccessModelVersion->setText(QString::fromStdString(response.userinfo().base().dataaccessmodelversion()));
        ui->DataAccessModelVersion->setReadOnly(true);
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

        proto::GetPermissionGroupByUuidResponse responseGetPermissionGroup;
        WrappedCall::GetPermissionGroupByUuid(response.userinfo().permissiongroupuuid(),responseGetPermissionGroup,this);

        ui->PermissionGroup->setText(QString::fromStdString(responseGetPermissionGroup.permissiongroup().name()));
        ui->PermissionGroup->setReadOnly(true);
        ui->UserId->setText(QString::fromStdString(std::to_string(response.userinfo().userid())));
        ui->UserId->setReadOnly(true);
    }
}
