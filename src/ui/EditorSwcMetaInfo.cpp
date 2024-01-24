#include "EditorSwcMetaInfo.h"
#include <QIcon>
#include <QMessageBox>
#include <grpcpp/client_context.h>
#include <Message/Request.pb.h>
#include <Message/Response.pb.h>

#include "ui_EditorSwcMetaInfo.h"
#include "src/framework/defination/ImageDefination.h"
#include "src/framework/service/CachedProtoData.h"
#include "src/framework/service/RpcCall.h"


EditorSwcMetaInfo::EditorSwcMetaInfo(proto::GetSwcMetaInfoResponse& response, QWidget *parent) :
    QWidget(parent), ui(new Ui::EditorSwcMetaInfo) {
    ui->setupUi(this);
    setWindowIcon(QIcon(Image::ImageNode));

    refresh(response);
}

EditorSwcMetaInfo::~EditorSwcMetaInfo() {
    delete ui;
}

bool EditorSwcMetaInfo::save() {
    proto::UpdateSwcRequest request;
    request.mutable_metainfo()->set_apiversion(RpcCall::ApiVersion);
    proto::UpdateSwcResponse response;
    grpc::ClientContext context;

    auto* userInfo = request.mutable_userverifyinfo();
    userInfo->set_username(CachedProtoData::getInstance().UserName);
    userInfo->set_usertoken(CachedProtoData::getInstance().UserToken);
    request.mutable_swcinfo()->CopyFrom(m_SwcMetaInfo);

    if(ui->Description->text().isEmpty()) {
        QMessageBox::warning(this,"Error","Description cannot be empty!");
        return false;
    }

    request.mutable_swcinfo()->set_description(ui->Description->text().toStdString());

    auto status = RpcCall::getInstance().Stub()->UpdateSwc(&context,request,&response);
    if(status.ok()) {
        if(response.metainfo().status()) {
            QMessageBox::information(this,"Info","Update Swc Successfully!");
            return true;
        }
        QMessageBox::critical(this,"Error",QString::fromStdString(response.metainfo().message()));
    }
    QMessageBox::critical(this,"Error",QString::fromStdString(status.error_message()));
    return false;
}

void EditorSwcMetaInfo::refresh(proto::GetSwcMetaInfoResponse& response) {
    m_SwcMetaInfo.CopyFrom(response.swcinfo());

    ui->Id->setText(QString::fromStdString(m_SwcMetaInfo.base()._id()));
    ui->Id->setReadOnly(true);
    ui->Uuid->setText(QString::fromStdString(m_SwcMetaInfo.base().uuid()));
    ui->Uuid->setReadOnly(true);
    ui->DataAccessModelVersion->setText(QString::fromStdString(m_SwcMetaInfo.base().dataaccessmodelversion()));
    ui->DataAccessModelVersion->setReadOnly(true);
    ui->Name->setText(QString::fromStdString(m_SwcMetaInfo.name()));
    ui->Name->setReadOnly(true);
    ui->Description->setText(QString::fromStdString(m_SwcMetaInfo.description()));
    ui->Creator->setText(QString::fromStdString(m_SwcMetaInfo.creator()));
    ui->Creator->setReadOnly(true);
    ui->CreateTime->setDateTime(QDateTime::fromSecsSinceEpoch(m_SwcMetaInfo.createtime().seconds()));
    ui->CreateTime->setReadOnly(true);
    ui->LastModifiedTime->setDateTime(QDateTime::fromSecsSinceEpoch(m_SwcMetaInfo.lastmodifiedtime().seconds()));
    ui->LastModifiedTime->setReadOnly(true);
}
