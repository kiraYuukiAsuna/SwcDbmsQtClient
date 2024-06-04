#include "EditorProjectMetaInfo.h"
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
    if (ui->Dsecription->text().isEmpty()) {
        QMessageBox::warning(this, "Error", "Description cannot be empty!");
        return false;
    }
    if (ui->WorkMode->text().isEmpty()) {
        QMessageBox::warning(this, "Error", "WorkMode cannot be empty!");
        return false;
    }

    m_ProjectMetaInfo.set_description(ui->Dsecription->text().toStdString());
    m_ProjectMetaInfo.set_workmode(ui->WorkMode->text().toStdString());

    m_ProjectMetaInfo.clear_swclist();
    for (int i = 0; i < ui->SwcList->count(); i++) {
        auto *item = ui->SwcList->item(i);
        if (item->checkState() == Qt::Checked) {
            auto *swc = m_ProjectMetaInfo.add_swclist();
            *swc = item->data(Qt::UserRole).value<QString>().toStdString();
        }
    }

    proto::UpdateProjectResponse response;
    if(!WrappedCall::UpdateProjectMetaInfo(m_ProjectMetaInfo,response,this)) {
        return false;
    }
    return true;
}

void EditorProjectMetaInfo::refresh(proto::GetProjectResponse &response) {
    m_ProjectMetaInfo.CopyFrom(response.projectinfo());

    ui->Id->setText(QString::fromStdString(m_ProjectMetaInfo.base()._id()));
    ui->Id->setReadOnly(true);
    ui->Uuid->setText(QString::fromStdString(m_ProjectMetaInfo.base().uuid()));
    ui->Uuid->setReadOnly(true);
    ui->DataAccessModelVersion->setText(QString::fromStdString(m_ProjectMetaInfo.base().dataaccessmodelversion()));
    ui->DataAccessModelVersion->setReadOnly(true);
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
            auto uuid = m_ProjectMetaInfo.swclist().Get(j);
            if (uuid == swcInfo.base().uuid()) {
                bFind = true;
            }
        }
        item->setData(Qt::UserRole, QString::fromStdString(swcInfo.base().uuid()));
        if (bFind) {
            item->setCheckState(Qt::Checked);
        } else {
            item->setCheckState(Qt::Unchecked);
        }
        ui->SwcList->addItem(item);
    }
}
