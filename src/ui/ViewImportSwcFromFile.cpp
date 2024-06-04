#include "ViewImportSwcFromFile.h"
#include "ui_ViewImportSwcFromFile.h"
#include <QFileDialog>
#include "src/FileIo/SwcIo.hpp"
#include "Message/Request.pb.h"
#include "src/framework/service/WrappedCall.h"
#include "MainWindow.h"
#include "src/FileIo/AnoIo.hpp"
#include "src/FileIo/ApoIo.hpp"
#include <filesystem>
#include "src/framework/util/unsortswc/swcutils.h"

ViewImportSwcFromFile::ViewImportSwcFromFile(MainWindow *mainWindow) :
        QDialog(mainWindow), ui(new Ui::ViewImportSwcFromFile) {
    ui->setupUi(this);

    m_MainWindow = mainWindow;

    ui->SwcFileInfo->clear();
    ui->SwcFileInfo->setColumnCount(7);
    QStringList headerLabels;
    headerLabels
            << "Swc FilePath"
            << "Ano FilePath"
            << "Apo FilePath"
            << "Type"
            << "Detected Swc Node Number"
            << "New Name"
            << "Import Status";
    ui->SwcFileInfo->setHorizontalHeaderLabels(headerLabels);
    ui->SwcFileInfo->resizeColumnsToContents();

    connect(ui->SelectFilesBtn, &QPushButton::clicked, this, [this]() {
        QFileDialog fileDialog(this);
        fileDialog.setWindowTitle("Select Swc Files");
        fileDialog.setDirectory(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
        fileDialog.setNameFilter(tr("File(*.swc *.eswc)"));
        fileDialog.setFileMode(QFileDialog::ExistingFiles);
        fileDialog.setViewMode(QFileDialog::Detail);

        m_SwcList.clear();
        m_ESwcList.clear();

        QStringList fileNames;
        if (fileDialog.exec()) {
            fileNames = fileDialog.selectedFiles();

            auto task = [=]() {
                for (int i = 0; i < fileNames.size(); i++) {
                    std::filesystem::path filePath(fileNames[i].toStdString());
                    if (filePath.extension() == ".swc") {
                        std::string unsortedSwcPath;
                        try {
                            unsortedSwcPath = convertSwcToUnsorted(filePath);
                        }catch (std::runtime_error &e) {
                            QApplication::postEvent(this, new UpdateImportUiErrorEvent(e.what() + std::string(" , skip this file! filename: ") + filePath.filename().string()));
                            continue;
                        }

                        Swc swc(unsortedSwcPath);
                        swc.ReadFromFile();
                        auto neuron = swc.getValue();

                        std::string anoPath = (filePath.parent_path() / filePath.stem().string()).string();
                        std::string apoPath = (filePath.parent_path() / (filePath.stem().string() + ".apo")).string();

                        ExtraSwcImportAttribute attribute;

                        if (std::filesystem::exists(anoPath)) {
                            attribute.m_AnoPath = anoPath;
                        }

                        if (std::filesystem::exists(apoPath)) {
                            attribute.m_ApoPath = apoPath;
                        }

                        m_SwcList.emplace_back(swc, attribute);

                        QApplication::postEvent(this, new UpdateImportUiEvent(i,
                           filePath.string(),
                                    attribute.m_AnoPath,
                                    attribute.m_ApoPath,
                                    "swc",
                                    neuron.size(),
                                    "Unprocessed"));

                    } else if (filePath.extension() == ".eswc") {
                        std::string unsortedSwcPath;
                        try {
                            unsortedSwcPath = convertSwcToUnsorted(filePath);
                        }catch (std::runtime_error &e) {
                            QApplication::postEvent(this, new UpdateImportUiErrorEvent(e.what() + std::string(" , skip this file! filename: ") + filePath.filename().string()));
                            continue;
                        }

                        ESwc eSwc(unsortedSwcPath);
                        eSwc.ReadFromFile();
                        auto neuron = eSwc.getValue();

                        std::string anoPath = (filePath.parent_path() / filePath.stem().string()).string();
                        std::string apoPath = (filePath.parent_path() / (filePath.stem().string() + ".apo")).string();

                        ExtraSwcImportAttribute attribute;

                        if (std::filesystem::exists(anoPath)) {
                            attribute.m_AnoPath = anoPath;
                        }

                        if (std::filesystem::exists(apoPath)) {
                            attribute.m_ApoPath = apoPath;
                        }

                        m_ESwcList.emplace_back(eSwc, attribute);

                        QApplication::postEvent(this, new UpdateImportUiEvent(i,
                                               filePath.string(),
                                                        attribute.m_AnoPath,
                                                        attribute.m_ApoPath,
                                                        "eswc",
                                                        neuron.size(),
                                                        "Unprocessed"));

                    }
                }
                QApplication::postEvent(this, new UpdateImportUiEndEvent());
            };

            m_IoContext.post(task);

            m_IoThread = std::thread([this]() {
                m_IoContext.run();
            });
            m_IoThread.detach();

            this->setEnabled(false);
        }

    });

    connect(ui->SelectFolderBtn, &QPushButton::clicked, this, [this](){
        QFileDialog fileDialog(this);
        fileDialog.setWindowTitle("Select Swc Folder");
        fileDialog.setDirectory(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
        fileDialog.setFileMode(QFileDialog::Directory);
        fileDialog.setViewMode(QFileDialog::Detail);

        m_SwcList.clear();
        m_ESwcList.clear();

        QStringList folderNames;
        if (fileDialog.exec()) {
            folderNames = fileDialog.selectedFiles();

            auto task = [=]() {
                int currentRow = 0;
                for (auto folder: folderNames) {
                    std::filesystem::path folderPath(folder.toStdString());

                    for (auto &dirEntry: std::filesystem::recursive_directory_iterator(folderPath)) {
                        const std::filesystem::path &filePath(dirEntry.path());
                        if (filePath.extension() == ".swc") {
                            std::string unsortedSwcPath;
                            try {
                                unsortedSwcPath = convertSwcToUnsorted(filePath);
                            }catch (std::runtime_error &e) {
                                QApplication::postEvent(this, new UpdateImportUiErrorEvent(e.what() + std::string(" , skip this file! filename: ") + filePath.filename().string()));
                                continue;
                            }

                            Swc swc(unsortedSwcPath);
                            swc.ReadFromFile();
                            auto neuron = swc.getValue();

                            std::string anoPath = (filePath.parent_path() / filePath.stem().string()).string();
                            std::string apoPath = (filePath.parent_path() / (filePath.stem().string() + ".apo")).string();

                            ExtraSwcImportAttribute attribute;

                            if (std::filesystem::exists(anoPath)) {
                                attribute.m_AnoPath = anoPath;
                            }

                            if (std::filesystem::exists(apoPath)) {
                                attribute.m_ApoPath = apoPath;
                            }

                            m_SwcList.emplace_back(swc, attribute);

                            QApplication::postEvent(this, new UpdateImportUiEvent(currentRow,
                                                                        filePath.string(),
                                                                                 attribute.m_AnoPath,
                                                                                 attribute.m_ApoPath,
                                                                                 "swc",
                                                                                 neuron.size(),
                                                                                 "Unprocessed"));

                            currentRow++;
                        } else if (filePath.extension() == ".eswc") {
                            std::string unsortedSwcPath;
                            try {
                                unsortedSwcPath = convertSwcToUnsorted(filePath);
                            }catch (std::runtime_error &e) {
                                QApplication::postEvent(this, new UpdateImportUiErrorEvent(e.what() + std::string(" , skip this file! filename: ") + filePath.filename().string()));
                                continue;
                            }

                            ESwc eSwc(unsortedSwcPath);
                            eSwc.ReadFromFile();
                            auto neuron = eSwc.getValue();

                            std::string anoPath = (filePath.parent_path() / filePath.stem().string()).string();
                            std::string apoPath = (filePath.parent_path() / (filePath.stem().string() + ".apo")).string();

                            ExtraSwcImportAttribute attribute;

                            if (std::filesystem::exists(anoPath)) {
                                attribute.m_AnoPath = anoPath;
                            }

                            if (std::filesystem::exists(apoPath)) {
                                attribute.m_ApoPath = apoPath;
                            }

                            m_ESwcList.emplace_back(eSwc, attribute);

                            QApplication::postEvent(this, new UpdateImportUiEvent(currentRow,
                                                                       filePath.string(),
                                                                                attribute.m_AnoPath,
                                                                                attribute.m_ApoPath,
                                                                                "eswc",
                                                                                neuron.size(),
                                                                                "Unprocessed"));

                            currentRow++;
                        }
                    }
                }

                QApplication::postEvent(this, new UpdateImportUiEndEvent());
            };

            m_IoContext.post(task);

            m_IoThread = std::thread([this]() {
                m_IoContext.run();
            });
            m_IoThread.detach();

            this->setEnabled(false);
        }

    });

    connect(ui->ImportBtn, &QPushButton::clicked, this, [this]() {
        if (ui->SwcFileInfo->rowCount() != m_SwcList.size() + m_ESwcList.size()) {
            QMessageBox::critical(this, "Error", "Data outdated! Please reopen this import window!");
            return;
        }

        if (m_ActionImportComplete) {
            QMessageBox::information(this, "Warning",
                                     "Import action has completed! Please reopen this import window if you want to import more swc data!");
            return;
        }

        this->setEnabled(false);

        int processedSwcNumber = 0;
        int processedESwcNumber = 0;
        for (int i = 0; i < ui->SwcFileInfo->rowCount(); i++) {
            QApplication::processEvents();

            auto swcName = ui->SwcFileInfo->item(i, 5)->text().toStdString();

            bool errorDetected = false;

            if (ui->SwcFileInfo->item(i, 3)->text() == "swc") {
                std::string description = "Auto Generated By SwcManagerClient.";
                proto::CreateSwcResponse response;
                if (WrappedCall::createSwcMeta(swcName, description, response, this)) {
                    proto::SwcDataV1 swcData;
                    auto &newSwcRawData = m_SwcList[processedSwcNumber].first.getValue();
                    for (auto &val: newSwcRawData) {
                        auto *newData = swcData.add_swcdata();
                        auto internalData = newData->mutable_swcnodeinternaldata();
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
                    }

                    proto::CreateSwcNodeDataResponse response1;
                    if (WrappedCall::addSwcNodeDataByUuid(response.swcinfo().base().uuid(), swcData, response1, this)) {
                        ui->SwcFileInfo->item(i, 6)->setText("Import Success!");
                        setAllGridColor(i, Qt::green);

                    } else {
                        ui->SwcFileInfo->item(i, 6)->setText("Create Swc Node failed!");
                        setAllGridColor(i, Qt::red);
                    }
                } else {
                    ui->SwcFileInfo->item(i, 6)->setText("Create Swc Meta Info failed!");
                    setAllGridColor(i, Qt::red);
                }

                if (!m_SwcList[processedSwcNumber].second.m_ApoPath.empty()) {
                    ApoIo io(m_SwcList[processedSwcNumber].second.m_ApoPath);

                    try {
                        io.ReadFromFile();
                        auto &value = io.getValue();

                        std::vector<proto::SwcAttachmentApoV1> modelData;
                        std::for_each(value.begin(), value.end(), [&](ApoUnit &val) {
                            proto::SwcAttachmentApoV1 data;
                            data.set_n(val.n);
                            data.set_orderinfo(val.orderinfo);
                            data.set_name(val.name);
                            data.set_comment(val.comment);
                            data.set_z(val.z);
                            data.set_x(val.x);
                            data.set_y(val.y);
                            data.set_pixmax(val.pixmax);
                            data.set_intensity(val.intensity);
                            data.set_sdev(val.sdev);
                            data.set_volsize(val.volsize);
                            data.set_mass(val.mass);
                            data.set_colorr(val.color_r);
                            data.set_colorg(val.color_g);
                            data.set_colorb(val.color_b);
                            modelData.push_back(data);
                        });

                        std::vector<proto::SwcAttachmentApoV1> m_SwcAttachmentApoData = modelData;

                        proto::CreateSwcAttachmentApoResponse responseApo;
                        if (!WrappedCall::createSwcAttachmentApo(swcName, m_SwcAttachmentApoData, responseApo,
                                                                 this)) {
                            if (!errorDetected) {
                                ui->SwcFileInfo->item(i, 6)->setText(
                                        "Create Swc Apo attachment Failed! " + QString::fromStdString(
                                                responseApo.metainfo().message()));
                                setAllGridColor(i, Qt::red);
                            }
                        }
                    }
                    catch (std::exception &e) {
                        ui->SwcFileInfo->item(i, 6)->setText(e.what());
                        ui->SwcFileInfo->item(i, 1)->setBackground(QBrush(Qt::red));
                    }
                }

                if (!m_SwcList[processedSwcNumber].second.m_AnoPath.empty()) {
                    AnoIo io(m_SwcList[processedSwcNumber].second.m_AnoPath);
                    io.ReadFromFile();
                    proto::CreateSwcAttachmentAnoResponse responseAno;
                    if (!WrappedCall::createSwcAttachmentAno(swcName, io.getValue().APOFILE,
                                                             io.getValue().SWCFILE, responseAno, this
                    )) {
                        if (!errorDetected) {
                            ui->SwcFileInfo->item(i, 6)->setText(
                                    "Create Swc Ano attachment Failed! " + QString::fromStdString(
                                            responseAno.metainfo().message()));
                            setAllGridColor(i, Qt::red);
                        }
                    }
                }

                processedSwcNumber++;
            } else if (ui->SwcFileInfo->item(i, 3)->text() == "eswc") {
                std::string description = "Auto Generated By SwcManagerClient.";
                proto::CreateSwcResponse response;
                if (WrappedCall::createSwcMeta(swcName, description, response, this)) {
                    proto::SwcDataV1 swcData;
                    auto &newSwcRawData = m_ESwcList[processedESwcNumber].first.getValue();
                    for (auto &j: newSwcRawData) {
                        auto *newData = swcData.add_swcdata();
                        auto internalData = newData->mutable_swcnodeinternaldata();
                        internalData->set_n(j.n);
                        internalData->set_type(j.type);
                        internalData->set_x(j.x);
                        internalData->set_y(j.y);
                        internalData->set_z(j.z);
                        internalData->set_radius(j.radius);
                        internalData->set_parent(j.parent);
                        internalData->set_seg_id(j.seg_id);
                        internalData->set_level(j.level);
                        internalData->set_mode(j.mode);
                        internalData->set_timestamp(j.timestamp);
                        internalData->set_feature_value(j.feature_value);
                    }

                    proto::CreateSwcNodeDataResponse response1;
                    if (WrappedCall::addSwcNodeDataByUuid(swcName, swcData, response1, this)) {
                        ui->SwcFileInfo->item(i, 6)->setText("Import Success!");
                        setAllGridColor(i, Qt::green);
                    } else {
                        ui->SwcFileInfo->item(i, 6)->setText("Create Swc Node failed!");
                        setAllGridColor(i, Qt::red);
                    }
                } else {
                    ui->SwcFileInfo->item(i, 6)->setText("Create Swc Meta Info failed!");
                    setAllGridColor(i, Qt::red);
                }

                if (!m_ESwcList[processedESwcNumber].second.m_ApoPath.empty()) {
                    ApoIo io(m_ESwcList[processedESwcNumber].second.m_ApoPath);

                    try {
                        io.ReadFromFile();
                        auto &value = io.getValue();

                        std::vector<proto::SwcAttachmentApoV1> modelData;
                        std::for_each(value.begin(), value.end(), [&](ApoUnit &val) {
                            proto::SwcAttachmentApoV1 data;
                            data.set_n(val.n);
                            data.set_orderinfo(val.orderinfo);
                            data.set_name(val.name);
                            data.set_comment(val.comment);
                            data.set_z(val.z);
                            data.set_x(val.x);
                            data.set_y(val.y);
                            data.set_pixmax(val.pixmax);
                            data.set_intensity(val.intensity);
                            data.set_sdev(val.sdev);
                            data.set_volsize(val.volsize);
                            data.set_mass(val.mass);
                            data.set_colorr(val.color_r);
                            data.set_colorg(val.color_g);
                            data.set_colorb(val.color_b);
                            modelData.push_back(data);
                        });

                        std::vector<proto::SwcAttachmentApoV1> m_SwcAttachmentApoData = modelData;

                        proto::CreateSwcAttachmentApoResponse responseApo;
                        if (!WrappedCall::createSwcAttachmentApo(swcName, m_SwcAttachmentApoData, responseApo,
                                                                 this)) {
                            if (!errorDetected) {
                                ui->SwcFileInfo->item(i, 6)->setText(
                                        "Create Swc Apo attachment Failed! " + QString::fromStdString(
                                                responseApo.metainfo().message()));
                                setAllGridColor(i, Qt::red);
                            }
                        }
                    }
                    catch (std::exception &e) {
                        ui->SwcFileInfo->item(i, 6)->setText(e.what());
                        ui->SwcFileInfo->item(i, 1)->setBackground(QBrush(Qt::red));
                    }
                }

                if (!m_ESwcList[processedESwcNumber].second.m_AnoPath.empty()) {
                    AnoIo io(m_ESwcList[processedESwcNumber].second.m_AnoPath);
                    io.ReadFromFile();
                    proto::CreateSwcAttachmentAnoResponse responseAno;
                    if (!WrappedCall::createSwcAttachmentAno(swcName, io.getValue().APOFILE,
                                                             io.getValue().SWCFILE, responseAno, this
                    )) {
                        if (!errorDetected) {
                            ui->SwcFileInfo->item(i, 6)->setText(
                                    "Create Swc Ano attachment Failed! " + QString::fromStdString(
                                            responseAno.metainfo().message()));
                            setAllGridColor(i, Qt::red);
                        }
                    }
                }

                processedESwcNumber++;
            }
        }
        this->setEnabled(true);
        ui->SwcFileInfo->resizeColumnsToContents();
        m_ActionImportComplete = true;
        QMessageBox::information(this, "Info",
                                 "Import action has completed! Please check the <Import Status> in the table below!");
    });

    connect(ui->CancelBtn, &QPushButton::clicked, this, [this]() {
        reject();
    });
}

ViewImportSwcFromFile::~ViewImportSwcFromFile() {
    delete ui;
}

bool ViewImportSwcFromFile::event(QEvent* e) {
    if(e->type() == UpdateImportUiEvent::TYPE) {

        ui->SwcFileInfo->setRowCount(ui->SwcFileInfo->rowCount() + 1);

        auto ev = dynamic_cast<UpdateImportUiEvent*>(e);

        ui->SwcFileInfo->setItem(ev->currentRow, 0,
                                 new QTableWidgetItem(QString::fromStdString(ev->swcFilePath)));
        ui->SwcFileInfo->setItem(ev->currentRow, 1,
                                 new QTableWidgetItem(QString::fromStdString(ev->anoFilePath)));
        ui->SwcFileInfo->setItem(ev->currentRow, 2,
                                 new QTableWidgetItem(QString::fromStdString(ev->apoFilePath)));
        ui->SwcFileInfo->setItem(ev->currentRow, 3,
                                 new QTableWidgetItem(QString::fromStdString(ev->swcType)));
        ui->SwcFileInfo->setItem(ev->currentRow, 4,
                                 new QTableWidgetItem(
                                         QString::fromStdString(std::to_string(ev->nodeSize))));
        ui->SwcFileInfo->setItem(ev->currentRow, 5,
                                 new QTableWidgetItem(
                                         QString::fromStdString(std::filesystem::path(ev->swcFilePath).filename().string())));
        ui->SwcFileInfo->setItem(ev->currentRow, 6,
                                 new QTableWidgetItem(QString::fromStdString("Unprocessed")));

        ui->SwcFileInfo->resizeColumnsToContents();
    }else if(e->type() == UpdateImportUiEndEvent::TYPE) {
        this->setEnabled(true);
    }else if(e->type() == UpdateImportUiErrorEvent::TYPE) {
        auto ev = dynamic_cast<UpdateImportUiErrorEvent*>(e);
        QMessageBox::critical(this, "Error", QString::fromStdString(ev->errorMessage));
    }

    return QWidget::event(e);
}

void ViewImportSwcFromFile::setAllGridColor(int row, const QColor &color) {
    ui->SwcFileInfo->item(row, 0)->setBackground(QBrush(color));
    ui->SwcFileInfo->item(row, 1)->setBackground(QBrush(color));
    ui->SwcFileInfo->item(row, 2)->setBackground(QBrush(color));
    ui->SwcFileInfo->item(row, 3)->setBackground(QBrush(color));
    ui->SwcFileInfo->item(row, 4)->setBackground(QBrush(color));
    ui->SwcFileInfo->item(row, 5)->setBackground(QBrush(color));
    ui->SwcFileInfo->item(row, 6)->setBackground(QBrush(color));
}

std::string ViewImportSwcFromFile::convertSwcToUnsorted(const std::filesystem::path&filePath) {
    QString inputSwcPath = QString::fromStdString(filePath.string());
    QString outputSwcPath = QString::fromStdString((getTempLocation() / filePath.filename()).string());

    convertSWC2UnSorted(inputSwcPath, outputSwcPath);

    return outputSwcPath.toStdString();
}
