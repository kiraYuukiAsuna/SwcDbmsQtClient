#include "editorprojectmetainfo.h"
#include "ui_EditorProjectMetaInfo.h"
#include <QDateTime>

#include "MainWindow.h"
#include "src/framework/defination/ImageDefination.h"
#include "src/framework/service/WrappedCall.h"

EditorProjectMetaInfo::EditorProjectMetaInfo(proto::GetProjectResponse &response, QWidget *parent) : QWidget(parent),
                                                                                                     ui(new Ui::EditorProjectMetaInfo) {
    ui->setupUi(this);
    setWindowIcon(QIcon(Image::ImageProject));

    std::string stylesheet = std::string("QListWidget::indicator:checked{image:url(")
                             + Image::ImageCheckBoxChecked + ");}" +
                             "QListWidget::indicator:unchecked{image:url(" +
                             Image::ImageCheckBoxUnchecked + ");}";
    ui->SwcList->setStyleSheet(QString::fromStdString(stylesheet));

    refresh(response);
}

EditorProjectMetaInfo::~EditorProjectMetaInfo() {
    delete ui;
}

bool EditorProjectMetaInfo::save() {
    proto::UpdateProjectRequest request;
    proto::UpdateProjectResponse response;
    grpc::ClientContext context;

    request.mutable_userinfo()->CopyFrom(CachedProtoData::getInstance().CachedUserMetaInfo);
    request.mutable_projectinfo()->CopyFrom(m_ProjectMetaInfo);

    if (ui->Dsecription->text().isEmpty()) {
        QMessageBox::warning(this, "Error", "Description cannot be empty!");
        return false;
    }
    if (ui->WorkMode->text().isEmpty()) {
        QMessageBox::warning(this, "Error", "WorkMode cannot be empty!");
        return false;
    }

    request.mutable_projectinfo()->set_description(ui->Dsecription->text().toStdString());
    request.mutable_projectinfo()->set_workmode(ui->WorkMode->text().toStdString());

    request.mutable_projectinfo()->clear_swclist();
    for (int i = 0; i < ui->SwcList->count(); i++) {
        auto *item = ui->SwcList->item(i);
        if (item->checkState() == Qt::Checked) {
            auto *swc = request.mutable_projectinfo()->add_swclist();
            *swc = item->text().toStdString();
        }
    }

    request.mutable_projectinfo()->clear_userpermissionoverride();
    for (int i = 0; i < ui->UserPermissionOverride->rowCount(); i++) {
        auto *userNameItem = ui->UserPermissionOverride->item(i, 0);
        if (userNameItem->checkState() == Qt::Checked) {
            auto *permissionOverride = request.mutable_projectinfo()->add_userpermissionoverride();
            permissionOverride->set_username(userNameItem->text().toStdString());
            permissionOverride->mutable_projectpermission()->set_writepermissionadddata(
                    std::stoi(ui->UserPermissionOverride->item(i, 1)->text().toStdString()));
            permissionOverride->mutable_projectpermission()->set_writepermissiondeletedata(
                    std::stoi(ui->UserPermissionOverride->item(i, 2)->text().toStdString()));
            permissionOverride->mutable_projectpermission()->set_writepermissionmodifydata(
                    std::stoi(ui->UserPermissionOverride->item(i, 3)->text().toStdString()));
            permissionOverride->mutable_projectpermission()->set_readperimissionquery(
                    std::stoi(ui->UserPermissionOverride->item(i, 4)->text().toStdString()));
        }
    }

    auto status = RpcCall::getInstance().Stub()->UpdateProject(&context, request, &response);
    if (status.ok()) {
        if (response.status()) {
            QMessageBox::information(this, "Info", "Update Project Successfully!");
            return true;
        }
        QMessageBox::critical(this, "Error", QString::fromStdString(response.message()));
    }
    QMessageBox::critical(this, "Error", QString::fromStdString(status.error_message()));
    return false;
}

void EditorProjectMetaInfo::refresh(proto::GetProjectResponse &response) {
    m_ProjectMetaInfo.CopyFrom(response.projectinfo());

    ui->Id->setText(QString::fromStdString(m_ProjectMetaInfo.base()._id()));
    ui->Id->setReadOnly(true);
    ui->Uuid->setText(QString::fromStdString(m_ProjectMetaInfo.base().uuid()));
    ui->Uuid->setReadOnly(true);
    ui->ApiVersion->setText(QString::fromStdString(m_ProjectMetaInfo.base().apiversion()));
    ui->ApiVersion->setReadOnly(true);
    ui->Name->setText(QString::fromStdString(m_ProjectMetaInfo.name()));
    ui->Name->setReadOnly(true);
    ui->Creator->setText(QString::fromStdString(m_ProjectMetaInfo.creator()));
    ui->Creator->setReadOnly(true);
    ui->WorkMode->setText(QString::fromStdString(m_ProjectMetaInfo.workmode()));
    ui->CreateTime->setDateTime(QDateTime::fromSecsSinceEpoch(m_ProjectMetaInfo.createtime().seconds()));
    ui->CreateTime->setReadOnly(true);
    ui->LastModifiedTime->setDateTime(QDateTime::fromSecsSinceEpoch(m_ProjectMetaInfo.lastmodifiedtime().seconds()));
    ui->LastModifiedTime->setReadOnly(true);
    ui->Dsecription->setText(QString::fromStdString(m_ProjectMetaInfo.description()));

    ui->SwcList->clear();

    proto::GetAllSwcMetaInfoResponse responseAllSwc;
    WrappedCall::getAllSwcMetaInfo(responseAllSwc, this);

    for (int i = 0; i < responseAllSwc.swcinfo_size(); i++) {
        auto swcInfo = responseAllSwc.swcinfo().Get(i);
        bool bFind = false;
        auto *item = new QListWidgetItem;
        item->setText(QString::fromStdString(swcInfo.name()));
        for (int j = 0; j < m_ProjectMetaInfo.swclist().size(); j++) {
            auto name = m_ProjectMetaInfo.swclist().Get(j);
            if (name == swcInfo.name()) {
                bFind = true;
            }
        }
        if (bFind) {
            item->setCheckState(Qt::Checked);
        } else {
            item->setCheckState(Qt::Unchecked);
        }
        ui->SwcList->addItem(item);
    }

    proto::GetAllUserResponse responseAllUser;
    WrappedCall::getAllUserMetaInfo(responseAllUser, this);

    ui->UserPermissionOverride->clear();
    ui->UserPermissionOverride->setRowCount(responseAllUser.userinfo_size());
    ui->UserPermissionOverride->setColumnCount(5);
    QStringList headerLabels;
    headerLabels
            << "UserName"
            << "ProjectWritePermissionAddData"
            << "ProjectWritePermissionModifyData"
            << "ProjectWritePermissionDeleteData"
            << "ProjectReadPerimissionQuery";
    ui->UserPermissionOverride->setHorizontalHeaderLabels(headerLabels);
    ui->UserPermissionOverride->resizeColumnsToContents();

    for (int i = 0; i < responseAllUser.userinfo_size(); i++) {
        auto userInfo = responseAllUser.userinfo().Get(i);
        auto userNameItem = new QTableWidgetItem(
                QString::fromStdString(userInfo.name()));
        ui->UserPermissionOverride->setItem(i, 0, userNameItem);
        ui->UserPermissionOverride->setItem(i, 1,
                                            new QTableWidgetItem(QString::fromStdString(std::to_string(1))));
        ui->UserPermissionOverride->setItem(i, 2,
                                            new QTableWidgetItem(QString::fromStdString(std::to_string(1))));
        ui->UserPermissionOverride->setItem(i, 3,
                                            new QTableWidgetItem(QString::fromStdString(std::to_string(1))));
        ui->UserPermissionOverride->setItem(i, 4,
                                            new QTableWidgetItem(QString::fromStdString(std::to_string(1))));
        proto::UserPermissionOverrideMetaInfoV1 permissionOverride;
        bool bFind = false;
        for (int j = 0; j < m_ProjectMetaInfo.userpermissionoverride().size(); j++) {
            permissionOverride = m_ProjectMetaInfo.userpermissionoverride().Get(i);
            if (permissionOverride.username() == userInfo.name()) {
                bFind = true;
            }
        }
        if (bFind) {
            ui->UserPermissionOverride->setItem(i, 0, userNameItem);
            ui->UserPermissionOverride->setItem(i, 1,
                                                new QTableWidgetItem(
                                                        QString::fromStdString(
                                                                std::to_string(
                                                                        permissionOverride.projectpermission().writepermissionadddata()))));
            ui->UserPermissionOverride->setItem(i, 2,
                                                new QTableWidgetItem(
                                                        QString::fromStdString(
                                                                std::to_string(
                                                                        permissionOverride.projectpermission().writepermissionmodifydata()))));
            ui->UserPermissionOverride->setItem(i, 3,
                                                new QTableWidgetItem(
                                                        QString::fromStdString(
                                                                std::to_string(
                                                                        permissionOverride.projectpermission().writepermissiondeletedata()))));
            ui->UserPermissionOverride->setItem(i, 4,
                                                new QTableWidgetItem(
                                                        QString::fromStdString(
                                                                std::to_string(
                                                                        permissionOverride.projectpermission().readperimissionquery()))));
            userNameItem->setCheckState(Qt::Checked);
        } else {
            userNameItem->setCheckState(Qt::Unchecked);
        }
    }

    ui->UserPermissionOverride->resizeColumnsToContents();
}
