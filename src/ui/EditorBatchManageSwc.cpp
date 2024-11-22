#include "EditorBatchManageSwc.h"

#include "EditorPermission.h"
#include "ui_EditorBatchManageSwc.h"
#include "src/framework/defination/ImageDefination.h"
#include "src/framework/service/WrappedCall.h"

EditorBatchManageSwc::EditorBatchManageSwc(const std::vector<std::string>&swcUuidList,
                                           QWidget* parent) : QDialog(parent), ui(new Ui::EditorBatchManageSwc),
                                                              m_SwcUuidList(swcUuidList) {
    ui->setupUi(this);

    std::string stylesheet = std::string("QListWidget::indicator:checked{image:url(")
                             + Image::ImageCheckBoxChecked + ");}" +
                             "QListWidget::indicator:unchecked{image:url(" +
                             Image::ImageCheckBoxUnchecked + ");}";
    ui->SwcList->setStyleSheet(QString::fromStdString(stylesheet));

    connect(ui->SelectAll, &QPushButton::clicked, this, [this] {
        for (int i = 0; i < ui->SwcList->count(); i++) {
            QListWidgetItem* item = ui->SwcList->item(i);
            item->setCheckState(Qt::Checked);
        }
    });

    connect(ui->UnselectAll, &QPushButton::clicked, this, [this] {
        for (int i = 0; i < ui->SwcList->count(); i++) {
            QListWidgetItem* item = ui->SwcList->item(i);
            item->setCheckState(Qt::Unchecked);
        }
    });

    ui->SwcList->clear();

    for (auto&swcUuid: m_SwcUuidList) {
        proto::GetSwcMetaInfoResponse response;
        if (!WrappedCall::getSwcMetaInfoByUuid(swcUuid, response, this)) {
            continue;
        }
        auto* item = new QListWidgetItem;
        item->setText(QString::fromStdString(response.swcinfo().name()));
        item->setData(Qt::UserRole, QString::fromStdString(swcUuid));
        item->setCheckState(Qt::Unchecked);

        ui->SwcList->addItem(item);
    }

    connect(ui->DeleteSelectedSwc, &QPushButton::clicked, this, [this] {
        QMessageBox::StandardButton reply = QMessageBox::question(this, "Delete Selected Swc",
                                                                  "Are you sure to delete the selected Swc?",
                                                                  QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            for (int i = ui->SwcList->count() - 1; i >= 0 ; i--) {
                QApplication::processEvents();
                QListWidgetItem* item = ui->SwcList->item(i);
                if (item->checkState() == Qt::Checked) {
                    proto::DeleteSwcResponse response;
                    auto uuid = item->data(Qt::UserRole).value<QString>().toStdString();
                    if (!WrappedCall::DeleteSwc(uuid, response, this)) {
                        continue;
                    }
                    delete item;
                }
            }
        }
    });

    connect(ui->ChangePermissionOfSelectedSwc, &QPushButton::clicked, this, [this] {
        EditorPermission editor("", MetaInfoType::eProjectSwc, true, this);
        editor.exec();
        auto permission = editor.getPermissionMetaInfo();
        for (int i = 0; i < ui->SwcList->count(); i++) {
            QApplication::processEvents();
            QListWidgetItem* item = ui->SwcList->item(i);
            if (item->checkState() == Qt::Checked) {
                proto::GetSwcMetaInfoResponse rsp1;
                auto uuid = item->data(Qt::UserRole).value<QString>().toStdString();
                WrappedCall::getSwcMetaInfoByUuid(uuid, rsp1, this);

                rsp1.mutable_swcinfo()->mutable_permission()->CopyFrom(permission);
                proto::UpdateSwcResponse rsp2;
                if (!WrappedCall::UpdateSwcMetaInfo(rsp1.swcinfo(), rsp2, this)) {
                    return;
                }
            }
        }
    });
}

EditorBatchManageSwc::~EditorBatchManageSwc() {
    delete ui;
}
