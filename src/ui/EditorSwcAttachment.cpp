#include "EditorSwcAttachment.h"

#include <QFileDialog>
#include <QStandardPaths>

#include "ui_EditorSwcAttachment.h"
#include "src/framework/service/WrappedCall.h"
#include "ViewSwcNodeData.h"
#include "src/FileIo/SwcIo.hpp"

EditorSwcAttachment::EditorSwcAttachment(const std::string& swcUuid, QWidget *parent) :
    QDialog(parent), ui(new Ui::EditorSwcAttachment), m_SwcUuid(swcUuid){
    ui->setupUi(this);

    connect(ui->OKBtn, &QPushButton::clicked, this, [&]() {
        if (m_IsSwcAttachmentExist) {
            proto::UpdateSwcAttachmentSwcResponse response;
            if (WrappedCall::UpdateSwcAttachmentSwcByUuid(m_SwcUuid, m_SwcAttachmentUuid, m_SwcAttachmentSwcData, response,
                                                    this)) {
                QMessageBox::information(this, "Info", "Update Swc Swc attachment successfully!");
                accept();
            } else {
                QMessageBox::critical(this, "Error",
                                      "Update Swc Swc attachment Failed! " + QString::fromStdString(
                                              response.metainfo().message()));
            }
        } else {
            proto::CreateSwcAttachmentSwcResponse response;
            if (WrappedCall::CreateSwcAttachmentSwcByUuid(m_SwcUuid, m_SwcAttachmentSwcData, response,
                                                    this)) {
                QMessageBox::information(this, "Info", "Create Swc Swc attachment successfully!");
                accept();
            } else {
                QMessageBox::critical(this, "Error",
                                      "Create Swc Swc attachment Failed! " + QString::fromStdString(
                                              response.metainfo().message()));
            }
        }
    });

    connect(ui->CancelBtn, &QPushButton::clicked, this, [&]() {
        reject();
    });

    connect(ui->DeleteData, &QPushButton::clicked, this, [&]() {
        if (m_SwcAttachmentSwcData.empty()) {
            return;
        }

        QModelIndex currentIndex = ui->tableView->selectionModel()->currentIndex();
        int currentRow = currentIndex.row();
        if (currentRow < 0) {
            QMessageBox::information(this, "Info", "You need to select one row first!");
            return;
        }

        m_SwcAttachmentSwcData.erase(m_SwcAttachmentSwcData.begin() + currentRow);
        loadSwcAttachmentSwcData();
        QMessageBox::information(this, "Info", "Delete Successfully!");
    });

    connect(ui->AddData, &QPushButton::clicked, this, [&]() {
        ViewSwcNodeData editor(this);
        if (editor.exec() == QDialog::Accepted) {
            auto swcNodeInternalData = editor.getSwcNodeInternalData();
            proto::SwcNodeDataV1 swcNodeData;
            swcNodeData.mutable_swcnodeinternaldata()->CopyFrom(swcNodeInternalData);
            m_SwcAttachmentSwcData.push_back(swcNodeData);
            loadSwcAttachmentSwcData();
            QMessageBox::information(this, "Info", "Add Successfully!");
        }
    });

    connect(ui->ModifyData, &QPushButton::clicked, this, [&]() {
        ViewSwcNodeData editor(true, this);

        QModelIndex currentIndex = ui->tableView->selectionModel()->currentIndex();
        int currentRow = currentIndex.row();
        if (currentRow < 0) {
            QMessageBox::information(this, "Info", "You need to select one row first!");
            return;
        }

        auto InitSwcSwcData = m_SwcAttachmentSwcData.at(currentRow);
        auto swcData = InitSwcSwcData.mutable_swcnodeinternaldata();
        editor.setSwcNodeInternalData(*swcData);
        if (editor.exec() == QDialog::Accepted) {
            auto swcNodeInternalData = editor.getSwcNodeInternalData();
            proto::SwcNodeDataV1 swcNodeData;
               swcNodeData.mutable_swcnodeinternaldata()->CopyFrom(swcNodeInternalData);
            m_SwcAttachmentSwcData[currentRow] = swcNodeData;
            loadSwcAttachmentSwcData();
            QMessageBox::information(this, "Info", "Modify Successfully!");
        }
    });

    connect(ui->DeleteBtnFromDbBtn, &QPushButton::clicked, this, [&]() {
        proto::DeleteSwcAttachmentSwcResponse response;
        if (WrappedCall::DeleteSwcAttachmentSwcByUuid(m_SwcUuid, m_SwcAttachmentUuid, response,
                                                this)) {
            QMessageBox::information(this, "Info", "Delete Swc Swc attachment successfully!");
            accept();
        } else {
            QMessageBox::critical(this, "Error",
                                  "Delete Swc Swc attachment Failed! " + QString::fromStdString(
                                          response.metainfo().message()));
        }
    });

    connect(ui->ImportBtn, &QPushButton::clicked, this, [&]() {
        QFileDialog dialog;
        dialog.setDirectory(QStandardPaths::displayName(QStandardPaths::HomeLocation));
        dialog.setFilter(QDir::Files);
        dialog.setNameFilter("*.eswc");

        if (dialog.exec()) {
            std::string filePath;
            if (int num = dialog.selectedFiles().count(); num != 0) {
                filePath = dialog.selectedFiles()[0].toStdString();
            }

            std::filesystem::path path(filePath);
            if (std::filesystem::exists(path)) {
                ESwc io(filePath);

                try {
                    io.ReadFromFile();
                    auto &value = io.getValue();

                    std::vector<proto::SwcNodeDataV1> modelData;
                    std::for_each(value.begin(), value.end(), [&](NeuronUnit &val) {
                        proto::SwcNodeDataV1 data;
                        auto* internalData = data.mutable_swcnodeinternaldata();
                        internalData->set_n(val.n);
                        internalData->set_type(val.type);
                        internalData->set_x(val.x);
                        internalData->set_y(val.y);
                        internalData->set_z(val.z);
                        internalData->set_radius(val.radius);
                        internalData->set_parent(val.parent);
                        internalData->set_seg_id(val.seg_id);
                        internalData->set_level(val.level);
                        internalData->set_mode(val.mode);
                        internalData->set_timestamp(val.timestamp);
                        internalData->set_feature_value(val.feature_value);
                        modelData.push_back(data);
                    });

                    m_SwcAttachmentSwcData = modelData;
                    loadSwcAttachmentSwcData();

                    QMessageBox::information(this, "Info", "Import Successfully!");
                }
                catch (std::exception &e) {
                    QMessageBox::critical(this, "Error", e.what());
                }
            } else {
                QMessageBox::critical(this, "Error", "Selected Swc File Not Exists!");
            }
        }
    });

    connect(ui->ExportBtn, &QPushButton::clicked, this, [&]() {
        if (m_SwcAttachmentSwcData.empty()) {
            QMessageBox::information(this, "Info", "No data to export!");
        }

        QString dirPath = QFileDialog::getExistingDirectory(this, tr("Select Save Directory"),
                                                            QStandardPaths::displayName(QStandardPaths::HomeLocation),
                                                            QFileDialog::ShowDirsOnly
                                                            | QFileDialog::DontResolveSymlinks);
        if (dirPath.isEmpty()) {
            return;
        }

        std::filesystem::path path(dirPath.toStdString());
        if (std::filesystem::exists(path)) {
            proto::GetSwcMetaInfoResponse response;
            WrappedCall::getSwcMetaInfoByUuid(m_SwcUuid,response,this);
            auto filePath = path / (response.swcinfo().name() + ".Swc.Swc");
            ESwc io(filePath.string());
            std::vector<NeuronUnit> units;
            std::for_each(m_SwcAttachmentSwcData.begin(), m_SwcAttachmentSwcData.end(),
                          [&](proto::SwcNodeDataV1 &val) {
                              NeuronUnit unit;
                              unit.n = val.swcnodeinternaldata().n();
                              unit.type = val.swcnodeinternaldata().type();
                              unit.x = val.swcnodeinternaldata().x();
                              unit.y = val.swcnodeinternaldata().y();
                              unit.z = val.swcnodeinternaldata().z();
                              unit.radius = val.swcnodeinternaldata().radius();
                              unit.parent = val.swcnodeinternaldata().parent();
                              unit.seg_id = val.swcnodeinternaldata().seg_id();
                              unit.level = val.swcnodeinternaldata().level();
                              unit.mode = val.swcnodeinternaldata().mode();
                              unit.timestamp = val.swcnodeinternaldata().timestamp();
                              unit.feature_value = val.swcnodeinternaldata().feature_value();
                              units.push_back(unit);
                          });

            io.setValue(units);
            io.WriteToFile();

            QMessageBox::information(this, "Info", "Export Successfully!");
        } else {
            QMessageBox::critical(this, "Error", "Selected Swc File Save Path Not Exists!");
        }
    });

    getSwcAttachmentSwc();
    loadSwcAttachmentSwcData();
}

EditorSwcAttachment::~EditorSwcAttachment() {
    delete ui;
}

void EditorSwcAttachment::getSwcAttachmentSwc() {
    proto::GetSwcMetaInfoResponse get_swc_meta_info_response;
    if (!WrappedCall::getSwcMetaInfoByUuid(m_SwcUuid, get_swc_meta_info_response, this)) {
        return;
    }

    if (!get_swc_meta_info_response.swcinfo().swcattachmentswcuuid().empty()) {
        m_IsSwcAttachmentExist = true;
    } else {
        QMessageBox::critical(this, "Error", "No Swc Attachment found! You can create a new swc attchment!");
        return;
    }

    proto::GetSwcAttachmentSwcResponse response;
    m_SwcAttachmentUuid = get_swc_meta_info_response.swcinfo().swcattachmentswcuuid();
    if (!WrappedCall::GetSwcAttachmentSwc(m_SwcUuid, m_SwcAttachmentUuid, response, this)) {
        return;
    }

    for (auto &data: response.swcdata()) {
        m_SwcAttachmentSwcData.push_back(data);
    }
}

void EditorSwcAttachment::loadSwcAttachmentSwcData() {
    auto *model = new SwcAttachmentTableModel(m_SwcAttachmentSwcData, this);
    ui->tableView->setModel(model);
    ui->tableView->resizeColumnsToContents();
}
