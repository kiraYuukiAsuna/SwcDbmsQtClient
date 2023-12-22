#include <QFileDialog>
#include <QStandardPaths>
#include <QMessageBox>
#include "vieweportswctofile.h"
#include "ui_ViewEportSwcToFile.h"
#include "src/framework/defination/ImageDefination.h"
#include "src/swcio/SwcIO.h"
#include "Message/Response.pb.h"
#include "src/framework/service/WrappedCall.h"


ViewEportSwcToFile::ViewEportSwcToFile(std::vector<ExportSwcData> exportSwcData, bool getDataFromServer, QWidget *parent) :
    QDialog(parent), ui(new Ui::ViewEportSwcToFile) {
    ui->setupUi(this);
    std::string stylesheet = std::string("QListWidget::indicator:checked{image:url(")
                             + Image::ImageCheckBoxChecked + ");}" +
                             "QListWidget::indicator:unchecked{image:url(" +
                             Image::ImageCheckBoxUnchecked + ");}";
    ui->SwcList->setStyleSheet(QString::fromStdString(stylesheet));

    m_ExportSwcData = exportSwcData;
    m_GetDataFromServer = getDataFromServer;

    for (auto & val : m_ExportSwcData) {
        auto userInfo = val;
        auto *item = new QListWidgetItem;
        item->setText(QString::fromStdString(val.swcMetaInfo.name()));
        item->setCheckState(Qt::Checked);
        ui->SwcList->addItem(item);
    }

    ui->ResultTable->clear();
    ui->ResultTable->setColumnCount(5);
    QStringList headerLabels;
    headerLabels
            << "Swc Name"
            << "Swc Type"
            << "Swc Node Number"
            << "Export Status"
            << "Save Path";
    ui->ResultTable->setHorizontalHeaderLabels(headerLabels);
    ui->ResultTable->setRowCount(m_ExportSwcData.size());
    ui->ResultTable->resizeColumnsToContents();

    connect(ui->SelectSavePathBtn, &QPushButton::clicked,this,[this](){
        QFileDialog fileDialog(this);
        fileDialog.setWindowTitle("Select Swc Files");
        fileDialog.setDirectory(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
        fileDialog.setNameFilter(tr("File(*.swc *.eswc)"));
        fileDialog.setFileMode(QFileDialog::Directory);
        fileDialog.setViewMode(QFileDialog::Detail);

        QStringList fileNames;
        if (fileDialog.exec()) {
            fileNames = fileDialog.selectedFiles();
            if(!fileNames.empty()){
                ui->SavePath->setText(fileNames[0]);
                m_SavePath = fileNames[0].toStdString();
                std::filesystem::path saveDir = m_SavePath;
                saveDir = saveDir / "ExportSwc";
                if(std::filesystem::exists(saveDir)){
                    QMessageBox::information(this, "Warning", "Selected directoty contains <ExportSwc> folder! Please remove it first!");
                    m_SavePath = "";
                    return;
                }
                if(!std::filesystem::create_directories(saveDir)){
                    QMessageBox::critical(this, "Error", "Create <ExportSwc> directory in save path failed!");
                    m_SavePath = "";
                    return;
                }
                m_SavePath = saveDir.string();
            }
        }
    });

    connect(ui->CancelBtn, &QPushButton::clicked,this,[this](){
        reject();
    });

    connect(ui->ExportBtn, &QPushButton::clicked,this,[this](){
        if(m_SavePath.empty()){
            QMessageBox::information(this, "Warning", "You did not select a valid save path!");
            return;
        }

        if(m_ActionExportComplete){
            QMessageBox::information(this, "Warning", "Export action complete! Please reopen this export window if you want export again!");
            return;
        }

        for (int i=0;i<m_ExportSwcData.size();i++ ) {
            ui->ResultTable->setItem(i,0,
                                     new QTableWidgetItem(QString::fromStdString(m_ExportSwcData[i].swcMetaInfo.name())));
            ui->ResultTable->setItem(i,1,
                                     new QTableWidgetItem(QString::fromStdString(m_ExportSwcData[i].swcMetaInfo.swctype())));
            ui->ResultTable->setItem(i,2,
                                     new QTableWidgetItem(QString::fromStdString(std::to_string(m_ExportSwcData[i].swcData.swcdata_size()))));
            QString statusMessage = "Ignored";
            QString fileSavePath = "";
            if(ui->SwcList->item(i)->checkState() == Qt::Checked){
                std::filesystem::path savePath = m_SavePath;

                if(m_ExportSwcData[i].swcMetaInfo.swctype() == "swc"){
                    savePath = savePath / (m_ExportSwcData[i].swcMetaInfo.name() + ".swc");

                    if(m_GetDataFromServer){
                        proto::GetSwcFullNodeDataResponse response;
                        if(!WrappedCall::getSwcFullNodeData(m_ExportSwcData[i].swcMetaInfo.name(), response, this)){
                            QMessageBox::critical(this,"Error",QString::fromStdString(response.message()));
                            ui->ResultTable->setItem(i,3,
                                                     new QTableWidgetItem("Get Data From Server Failed"));
                            ui->ResultTable->setItem(i,4,
                                                     new QTableWidgetItem(fileSavePath));
                            ui->ResultTable->item(i, 0)->setBackground(QBrush(Qt::red));
                            ui->ResultTable->item(i, 1)->setBackground(QBrush(Qt::red));
                            ui->ResultTable->item(i, 2)->setBackground(QBrush(Qt::red));
                            ui->ResultTable->item(i, 3)->setBackground(QBrush(Qt::red));
                            ui->ResultTable->item(i, 4)->setBackground(QBrush(Qt::red));
                            continue;
                        }
                        m_ExportSwcData[i].swcData = response.swcnodedata();
                    }

                    std::vector<NeuronUnit> neurons;
                    auto swcData = m_ExportSwcData[i].swcData;
                    for (int j=0;j<swcData.swcdata_size();j++) {
                        NeuronUnit unit;
                        unit.n = swcData.swcdata(j).swcnodeinternaldata().n();
                        unit.type = swcData.swcdata(j).swcnodeinternaldata().type();
                        unit.x = swcData.swcdata(j).swcnodeinternaldata().x();
                        unit.y = swcData.swcdata(j).swcnodeinternaldata().y();
                        unit.z = swcData.swcdata(j).swcnodeinternaldata().z();
                        unit.radius = swcData.swcdata(j).swcnodeinternaldata().radius();
                        unit.parent = swcData.swcdata(j).swcnodeinternaldata().parent();
                        neurons.push_back(unit);
                    }

                    Swc swc(savePath.string());
                    swc.setNeuron(neurons);
                    if(swc.WriteToFile()){
                        ui->ResultTable->setItem(i,3,
                                                 new QTableWidgetItem("Successfully"));
                        ui->ResultTable->setItem(i,4,
                                                 new QTableWidgetItem(QString::fromStdString(savePath.string())));
                        ui->ResultTable->item(i, 0)->setBackground(QBrush(Qt::green));
                        ui->ResultTable->item(i, 1)->setBackground(QBrush(Qt::green));
                        ui->ResultTable->item(i, 2)->setBackground(QBrush(Qt::green));
                        ui->ResultTable->item(i, 3)->setBackground(QBrush(Qt::green));
                        ui->ResultTable->item(i, 4)->setBackground(QBrush(Qt::green));
                    }else{
                        statusMessage= "Write Swc File Failed!";
                        fileSavePath = "";
                    }
                }else if(m_ExportSwcData[i].swcMetaInfo.swctype() == "eswc") {
                    savePath = savePath / (m_ExportSwcData[i].swcMetaInfo.name() + ".eswc");

                    if(m_GetDataFromServer){
                        proto::GetSwcFullNodeDataResponse response;
                        if(!WrappedCall::getSwcFullNodeData(m_ExportSwcData[i].swcMetaInfo.name(), response, this)){
                            QMessageBox::critical(this,"Error",QString::fromStdString(response.message()));
                            ui->ResultTable->setItem(i,3,
                                                     new QTableWidgetItem("Get Data From Server Failed"));
                            ui->ResultTable->setItem(i,4,
                                                     new QTableWidgetItem(fileSavePath));
                            ui->ResultTable->item(i, 0)->setBackground(QBrush(Qt::red));
                            ui->ResultTable->item(i, 1)->setBackground(QBrush(Qt::red));
                            ui->ResultTable->item(i, 2)->setBackground(QBrush(Qt::red));
                            ui->ResultTable->item(i, 3)->setBackground(QBrush(Qt::red));
                            ui->ResultTable->item(i, 4)->setBackground(QBrush(Qt::red));
                            continue;
                        }
                        m_ExportSwcData[i].swcData = response.swcnodedata();
                    }

                    std::vector<NeuronUnit> neurons;
                    auto swcData = m_ExportSwcData[i].swcData;
                    for (int j=0;j<swcData.swcdata_size();j++) {
                        NeuronUnit unit;
                        unit.n = swcData.swcdata(j).swcnodeinternaldata().n();
                        unit.type = swcData.swcdata(j).swcnodeinternaldata().type();
                        unit.x = swcData.swcdata(j).swcnodeinternaldata().x();
                        unit.y = swcData.swcdata(j).swcnodeinternaldata().y();
                        unit.z = swcData.swcdata(j).swcnodeinternaldata().z();
                        unit.radius = swcData.swcdata(j).swcnodeinternaldata().radius();
                        unit.parent = swcData.swcdata(j).swcnodeinternaldata().parent();
                        unit.seg_id = swcData.swcdata(j).swcnodeinternaldata().seg_id();
                        unit.level = swcData.swcdata(j).swcnodeinternaldata().level();
                        unit.mode = swcData.swcdata(j).swcnodeinternaldata().mode();
                        unit.timestamp = swcData.swcdata(j).swcnodeinternaldata().timestamp();
                        unit.feature_value = swcData.swcdata(j).swcnodeinternaldata().feature_value();
                        neurons.push_back(unit);
                    }

                    ESwc eSwc(savePath.string());
                    eSwc.setNeuron(neurons);
                    if(eSwc.WriteToFile()){
                        ui->ResultTable->setItem(i,3,
                                                 new QTableWidgetItem("Successfully"));
                        ui->ResultTable->setItem(i,4,
                                                 new QTableWidgetItem(QString::fromStdString(savePath.string())));
                        ui->ResultTable->item(i, 0)->setBackground(QBrush(Qt::green));
                        ui->ResultTable->item(i, 1)->setBackground(QBrush(Qt::green));
                        ui->ResultTable->item(i, 2)->setBackground(QBrush(Qt::green));
                        ui->ResultTable->item(i, 3)->setBackground(QBrush(Qt::green));
                        ui->ResultTable->item(i, 4)->setBackground(QBrush(Qt::green));
                        continue;
                    }else{
                        statusMessage= "Write Swc File Failed!";
                        fileSavePath = "";
                    }
                }else{
                    statusMessage= "Ignored, Unknown Swc Type";
                    fileSavePath = "";
                }
            }
            ui->ResultTable->setItem(i,3,
                                     new QTableWidgetItem(statusMessage));
            ui->ResultTable->setItem(i,4,
                                     new QTableWidgetItem(fileSavePath));
            ui->ResultTable->item(i, 0)->setBackground(QBrush(Qt::red));
            ui->ResultTable->item(i, 1)->setBackground(QBrush(Qt::red));
            ui->ResultTable->item(i, 2)->setBackground(QBrush(Qt::red));
            ui->ResultTable->item(i, 3)->setBackground(QBrush(Qt::red));
            ui->ResultTable->item(i, 4)->setBackground(QBrush(Qt::red));
        }
        m_ActionExportComplete = true;
        ui->ResultTable->resizeColumnsToContents();
    });

}

ViewEportSwcToFile::~ViewEportSwcToFile() {
    delete ui;
}
