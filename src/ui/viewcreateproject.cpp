#include "viewcreateproject.h"

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

    ui->tableWidget->clear();
    proto::GetAllUserResponse responseAllUser;
    WrappedCall::getAllUserMetaInfo(responseAllUser, this);
    ui->tableWidget->setRowCount(responseAllUser.userinfo_size());
    ui->tableWidget->setColumnCount(5);
    QStringList headerLabels;
    headerLabels
            << "UserName"
            << "ProjectWritePermissionAddData"
            << "ProjectWritePermissionModifyData"
            << "ProjectWritePermissionDeleteData"
            << "ProjectReadPerimissionQuery";
    ui->tableWidget->setHorizontalHeaderLabels(headerLabels);

    for (int i = 0; i < responseAllUser.userinfo_size(); i++) {
        auto userInfo = responseAllUser.userinfo().Get(i);
        auto userNameItem = new QTableWidgetItem(
            QString::fromStdString(userInfo.name()));
        userNameItem->setCheckState(Qt::Unchecked);
        ui->tableWidget->setItem(i, 0, userNameItem);
        ui->tableWidget->setItem(i, 1,
                                 new QTableWidgetItem(
                                     QString::fromStdString(
                                         std::to_string(1))));
        ui->tableWidget->setItem(i, 2,
                                 new QTableWidgetItem(
                                     QString::fromStdString(
                                         std::to_string(1))));
        ui->tableWidget->setItem(i, 3,
                                 new QTableWidgetItem(
                                     QString::fromStdString(
                                         std::to_string(1))));
        ui->tableWidget->setItem(i, 4,
                                 new QTableWidgetItem(
                                     QString::fromStdString(
                                         std::to_string(1))));
    }
    ui->tableWidget->resizeColumnsToContents();

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
        request.mutable_userinfo()->CopyFrom(CachedProtoData::getInstance().CachedUserMetaInfo);
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

        for (int i = 0; i < ui->tableWidget->rowCount(); i++) {
            auto* userNameItem = ui->tableWidget->item(i, 0);
            if (userNameItem->checkState() == Qt::Checked) {
                auto* permissionOverride = request.mutable_projectinfo()->add_userpermissionoverride();
                permissionOverride->set_username(userNameItem->text().toStdString());
                permissionOverride->mutable_projectpermission()->set_writepermissionadddata(
                    std::stoi(ui->tableWidget->item(i, 1)->text().toStdString()));
                permissionOverride->mutable_projectpermission()->set_writepermissiondeletedata(
                    std::stoi(ui->tableWidget->item(i, 2)->text().toStdString()));
                permissionOverride->mutable_projectpermission()->set_writepermissionmodifydata(
                    std::stoi(ui->tableWidget->item(i, 3)->text().toStdString()));
                permissionOverride->mutable_projectpermission()->set_readperimissionquery(
                    std::stoi(ui->tableWidget->item(i, 4)->text().toStdString()));
            }
        }

        proto::CreateProjectResponse response;
        grpc::ClientContext context;
        auto status = RpcCall::getInstance().Stub()->CreateProject(&context,request,&response);
        if(status.ok()) {
            if(response.status()) {
                QMessageBox::information(parent,"Info","Create Project Successfully!");
                accept();
            }else {
                QMessageBox::critical(parent,"Error",QString::fromStdString(response.message()));
            }
        }else{
            QMessageBox::critical(parent,"Error",QString::fromStdString(status.error_message()));
        }
    });
}

ViewCreateProject::~ViewCreateProject() {
    delete ui;
}
