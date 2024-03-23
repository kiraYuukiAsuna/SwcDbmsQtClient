#include "EditorAnoAttachment.h"

#include <QFileDialog>
#include <QStandardPaths>

#include "ui_EditorAnoAttachment.h"
#include "src/framework/service/WrappedCall.h"
#include "src/FileIo/AnoIo.hpp"
#include "src/FileIo/ApoIo.hpp"
#include <filesystem>

EditorAnoAttachment::EditorAnoAttachment(const std::string&swcName, QWidget* parent) : QDialog(parent),
    ui(new Ui::EditorAnoAttachment), m_SwcName(swcName) {
    ui->setupUi(this);

    getSwcAnoAttachment();

    connect(ui->OKBtn, &QPushButton::clicked, this, [&]() {
        if (m_IsAnoAttachmentExist) {
            proto::UpdateSwcAttachmentAnoResponse response;
            if (WrappedCall::updateSwcAttachmentAno(m_SwcName, m_AttachmentUuid, ui->ApoFileName->text().toStdString(),
                                                    ui->SwcFileName->text().toStdString(), response, parent
            )) {
                QMessageBox::information(this, "Info", "Update Swc Ano attachment successfully!");
                accept();
            }
            else {
                QMessageBox::critical(this, "Error",
                                      "Update Swc Ano attachment Failed! " + QString::fromStdString(
                                          response.metainfo().message()));
            }
        }
        else {
            proto::CreateSwcAttachmentAnoResponse response;
            if (WrappedCall::createSwcAttachmentAno(m_SwcName, ui->ApoFileName->text().toStdString(),
                                                    ui->SwcFileName->text().toStdString(), response, parent
            )) {
                QMessageBox::information(this, "Info", "Create Swc Ano attachment successfully!");
                accept();
            }
            else {
                QMessageBox::critical(this, "Error",
                                      "Create Swc Ano attachment Failed! " + QString::fromStdString(
                                          response.metainfo().message()));
            }
        }
    });

    connect(ui->DeleteBtn, &QPushButton::clicked, this, [&]() {
        if (m_IsAnoAttachmentExist) {
            proto::DeleteSwcAttachmentAnoResponse response;
            if (WrappedCall::deleteSwcAttachmentAno(m_SwcName, m_AttachmentUuid, response, parent
            )) {
                QMessageBox::information(this, "Info", "Delete Swc Ano attachment successfully!");
                accept();
            }
        }
        else {
            QMessageBox::information(this, "Info", "No Ano Attachment found!");
        }
    });

    connect(ui->ImportBtn, &QPushButton::clicked, this, [&]() {
        QFileDialog dialog;
        dialog.setDirectory(QStandardPaths::displayName(QStandardPaths::HomeLocation));
        dialog.setFilter(QDir::Files);
        dialog.setNameFilter("*.ano");

        if (dialog.exec()) {
            std::string filePath;
            if (int num = dialog.selectedFiles().count(); num != 0) {
                filePath = dialog.selectedFiles()[0].toStdString();
            }

            std::filesystem::path path(filePath);
            if (std::filesystem::exists(path)) {
                AnoIo io(filePath);

                try {
                    io.ReadFromFile();

                    ui->ApoFileName->setText(QString::fromStdString(io.getValue().APOFILE));
                    ui->SwcFileName->setText(QString::fromStdString(io.getValue().SWCFILE));

                    QMessageBox::information(this, "Info", "Import Successfully!");
                }
                catch (std::exception&e) {
                    QMessageBox::critical(this, "Error", e.what());
                }
            }
            else {
                QMessageBox::critical(this, "Error", "Selected Ano File Not Exists!");
            }
        }
    });

    connect(ui->ExportBtn, &QPushButton::clicked, this, [&]() {
        QString dirPath = QFileDialog::getExistingDirectory(this, tr("Select Save Directory"),
                                                            QStandardPaths::displayName(QStandardPaths::HomeLocation),
                                                            QFileDialog::ShowDirsOnly
                                                            | QFileDialog::DontResolveSymlinks);
        if (dirPath.isEmpty()) {
            return;
        }

        std::filesystem::path path(dirPath.toStdString());
        if (std::filesystem::exists(path)) {
            auto filePath = path / (m_SwcName + ".ano");
            AnoIo io(filePath.string());
            AnoUnit unit;
            unit.APOFILE = ui->ApoFileName->text().toStdString();
            unit.SWCFILE = ui->SwcFileName->text().toStdString();
            io.setValue(unit);
            io.WriteToFile();

            QMessageBox::information(this, "Info", "Export Successfully!");
        }
        else {
            QMessageBox::critical(this, "Error", "Selected Ano File Save Path Not Exists!");
        }
    });

    connect(ui->CancelBtn, &QPushButton::clicked, this, [&]() {
        reject();
    });
}

EditorAnoAttachment::~EditorAnoAttachment() {
    delete ui;
}

void EditorAnoAttachment::getSwcAnoAttachment() {
    proto::GetSwcMetaInfoResponse get_swc_meta_info_response;
    if (!WrappedCall::getSwcMetaInfoByName(m_SwcName, get_swc_meta_info_response, this)) {
        return;
    }

    if (!get_swc_meta_info_response.swcinfo().swcattachmentanometainfo().attachmentuuid().empty()) {
        m_IsAnoAttachmentExist = true;
    }
    else {
        QMessageBox::critical(this, "Error", "No Ano Attachment found! You can create a new ano attchment!");
        return;
    }

    proto::GetSwcAttachmentAnoResponse get_swc_attachment_ano_response;
    m_AttachmentUuid = get_swc_meta_info_response.swcinfo().swcattachmentanometainfo().attachmentuuid();
    if (!WrappedCall::getSwcAttachmentAno(m_SwcName, m_AttachmentUuid, get_swc_attachment_ano_response,
                                          this)) {
        return;
    }

    ui->ApoFileName->setText(QString::fromStdString(get_swc_attachment_ano_response.swcattachmentano().apofile()));
    ui->SwcFileName->setText(QString::fromStdString(get_swc_attachment_ano_response.swcattachmentano().swcfile()));
}

