#include "EditorDailyStatisticsMetaInfo.h"

#include <QMessageBox>
#include <grpcpp/client_context.h>
#include <Message/Request.pb.h>

#include "ui_EditorDailyStatisticsMetaInfo.h"
#include "src/framework/defination/ImageDefination.h"
#include "src/framework/service/CachedProtoData.h"
#include "src/framework/service/RpcCall.h"
#include <filesystem>

EditorDailyStatisticsMetaInfo::EditorDailyStatisticsMetaInfo(proto::GetDailyStatisticsResponse& response, QWidget *parent) :
    QWidget(parent), ui(new Ui::EditorDailyStatisticsMetaInfo) {
    ui->setupUi(this);
    setWindowIcon(QIcon(Image::ImageDaily));

    refresh(response);
}

EditorDailyStatisticsMetaInfo::~EditorDailyStatisticsMetaInfo() {
    delete ui;
}

void EditorDailyStatisticsMetaInfo::refresh(proto::GetDailyStatisticsResponse& response) {
    m_DailyStatisticsMetaInfo.CopyFrom(response.dailystatisticsinfo());

    ui->Id->setText(QString::fromStdString(m_DailyStatisticsMetaInfo.base()._id()));
    ui->Id->setReadOnly(true);
    ui->Uuid->setText(QString::fromStdString(m_DailyStatisticsMetaInfo.base().uuid()));
    ui->Uuid->setReadOnly(true);
    ui->DataAccessModelVersion->setText(QString::fromStdString(m_DailyStatisticsMetaInfo.base().dataaccessmodelversion()));
    ui->DataAccessModelVersion->setReadOnly(true);
    ui->Name->setText(QString::fromStdString(m_DailyStatisticsMetaInfo.name()));
    ui->Name->setReadOnly(true);
    ui->Description->setText(QString::fromStdString(m_DailyStatisticsMetaInfo.description()));
    ui->Day->setText(QString::fromStdString(m_DailyStatisticsMetaInfo.day()));
    ui->Day->setReadOnly(true);
    ui->CreatedProjectNumber->setText(QString::fromStdString(std::to_string(m_DailyStatisticsMetaInfo.createdprojectnumber())));
    ui->CreatedProjectNumber->setReadOnly(true);
    ui->CreatedSwcNumber->setText(QString::fromStdString(std::to_string(m_DailyStatisticsMetaInfo.createdswcnumber())));
    ui->CreatedSwcNumber->setReadOnly(true);
    ui->CreateSwcNodeNumber->setText(QString::fromStdString(std::to_string(m_DailyStatisticsMetaInfo.createswcnodenumber())));
    ui->CreateSwcNodeNumber->setReadOnly(true);
    ui->DeletedProjectNumber->setText(QString::fromStdString(std::to_string(m_DailyStatisticsMetaInfo.deletedprojectnumber())));
    ui->DeletedProjectNumber->setReadOnly(true);
    ui->DeletedSwcNumber->setText(QString::fromStdString(std::to_string(m_DailyStatisticsMetaInfo.deletedswcnumber())));
    ui->DeletedSwcNumber->setReadOnly(true);
    ui->DeletedSwcNodeNumber->setText(QString::fromStdString(std::to_string(m_DailyStatisticsMetaInfo.deletedswcnodenumber())));
    ui->DeletedSwcNodeNumber->setReadOnly(true);
    ui->ModifiedProjectNumber->setText(QString::fromStdString(std::to_string(m_DailyStatisticsMetaInfo.modifiedprojectnumber())));
    ui->ModifiedProjectNumber->setReadOnly(true);
    ui->ModifiedSwcNumber->setText(QString::fromStdString(std::to_string(m_DailyStatisticsMetaInfo.modifiedswcnodenumber())));
    ui->ModifiedSwcNumber->setReadOnly(true);
    ui->ModifiedSwcNodeNumber->setText(QString::fromStdString(std::to_string(m_DailyStatisticsMetaInfo.modifiedswcnodenumber())));
    ui->ModifiedSwcNodeNumber->setReadOnly(true);
    ui->ProjectQueryNumber->setText(QString::fromStdString(std::to_string(m_DailyStatisticsMetaInfo.projectquerynumber())));
    ui->ProjectQueryNumber->setReadOnly(true);
    ui->SwcQueryNumber->setText(QString::fromStdString(std::to_string(m_DailyStatisticsMetaInfo.swcquerynumber())));
    ui->SwcQueryNumber->setReadOnly(true);
    ui->NodeQueryNumber->setText(QString::fromStdString(std::to_string(m_DailyStatisticsMetaInfo.nodequerynumber())));
    ui->NodeQueryNumber->setReadOnly(true);
    ui->ActiveUserNumber->setText(QString::fromStdString(std::to_string(m_DailyStatisticsMetaInfo.activeusernumber())));
    ui->ActiveUserNumber->setReadOnly(true);
}

bool EditorDailyStatisticsMetaInfo::save() {
    proto::UpdateDailyStatisticsRequest request;
    request.mutable_metainfo()->set_apiversion(RpcCall::ApiVersion);
    proto::UpdateDailyStatisticsResponse response;
    grpc::ClientContext context;

    auto* userInfo = request.mutable_userverifyinfo();
    userInfo->set_username(CachedProtoData::getInstance().UserName);
    userInfo->set_usertoken(CachedProtoData::getInstance().UserToken);
    request.mutable_dailystatisticsinfo()->CopyFrom(m_DailyStatisticsMetaInfo);

    if(ui->Description->text().isEmpty()) {
        QMessageBox::warning(this,"Error","Description cannot be empty!");
        return false;
    }

    request.mutable_dailystatisticsinfo()->set_description(ui->Description->text().toStdString());

    auto status = RpcCall::getInstance().Stub()->UpdateDailyStatistics(&context,request,&response);
    if(status.ok()) {
        if(response.metainfo().status()) {
            QMessageBox::information(this,"Info","Update DailyStatistics Successfully!");
            return true;
        }
        QMessageBox::critical(this,"Error",QString::fromStdString(response.metainfo().message()));
    }
    QMessageBox::critical(this,"Error",QString::fromStdString(status.error_message()));
    return false;
}
