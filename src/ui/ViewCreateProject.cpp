#include "ViewCreateProject.h"

#include <QMessageBox>

#include "ui_ViewCreateProject.h"
#include "src/framework/defination/ImageDefination.h"
#include "src/framework/service/WrappedCall.h"


ViewCreateProject::ViewCreateProject(QWidget* parent) : QDialog(parent), ui(new Ui::ViewCreateProject) {
    ui->setupUi(this);
    setWindowIcon(QIcon(Image::ImageProject));

    ui->listWidget->clear();

    std::string stylesheet = std::string("QListWidget::indicator:checked{image:url(")
                                + Image::ImageCheckBoxChecked + ");}" +
                                    "QListWidget::indicator:unchecked{image:url(" +
                                        Image::ImageCheckBoxUnchecked+");}";
    ui->listWidget->setStyleSheet(QString::fromStdString(stylesheet));

    proto::GetAllSwcMetaInfoResponse responseAllSwc;
    WrappedCall::getAllSwcMetaInfo(responseAllSwc, this);
    for (int i = 0; i < responseAllSwc.swcinfo_size(); i++) {
        auto swcInfo = responseAllSwc.swcinfo().Get(i);
        auto* item = new QListWidgetItem;
        item->setText(QString::fromStdString(swcInfo.name()));
        item->setCheckState(Qt::Unchecked);
        ui->listWidget->addItem(item);
    }

    connect(ui->CancelBtn, &QPushButton::clicked, this, [this]() {
        reject();
    });

    connect(ui->OKBtn, &QPushButton::clicked, this, [this,parent]() {
        if (ui->Name->text().trimmed().isEmpty()) {
            QMessageBox::information(this, "Warning", "Project Name Cannot be null!");
            return;
        }
        if (ui->WorkMode->text().trimmed().isEmpty()) {
            QMessageBox::information(this, "Warning", "WorkMode Cannot be null!");
            return;
        }

        proto::CreateProjectRequest request;
        request.mutable_metainfo()->set_apiversion(RpcCall::ApiVersion);
        auto* userInfo = request.mutable_userverifyinfo();
        userInfo->set_username(CachedProtoData::getInstance().UserName);
        userInfo->set_usertoken(CachedProtoData::getInstance().UserToken);
        request.mutable_projectinfo()->set_name(ui->Name->text().trimmed().toStdString());
        request.mutable_projectinfo()->set_description(ui->Description->text().toStdString());
        request.mutable_projectinfo()->set_workmode(ui->WorkMode->text().toStdString());

        for (int i = 0; i < ui->listWidget->count(); i++) {
            auto* item = ui->listWidget->item(i);
            if (item->checkState() == Qt::Checked) {
                auto* swc = request.mutable_projectinfo()->add_swclist();
                *swc = item->text().toStdString();
            }
        }

        proto::CreateProjectResponse response;
        grpc::ClientContext context;
        auto status = RpcCall::getInstance().Stub()->CreateProject(&context,request,&response);
        if(status.ok()) {
            if(response.metainfo().status()) {
                QMessageBox::information(parent,"Info","Create Project Successfully!");
                accept();
            }else {
                QMessageBox::critical(parent,"Error",QString::fromStdString(response.metainfo().message()));
            }
        }else{
            QMessageBox::critical(parent,"Error",QString::fromStdString(status.error_message()));
        }
    });
}

ViewCreateProject::~ViewCreateProject() {
    delete ui;
}
