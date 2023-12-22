#include <QListWidgetItem>
#include "editorswcnode.h"
#include "ui_EditorSwcNode.h"
#include "src/framework/service/WrappedCall.h"
#include "src/framework/defination/ImageDefination.h"
#include "viewswcnodedata.h"
#include "vieweportswctofile.h"

EditorSwcNode::EditorSwcNode(const std::string &swcName, QWidget *parent) :
        QWidget(parent), ui(new Ui::EditorSwcNode) {
    ui->setupUi(this);
    std::string stylesheet = std::string("QListWidget::indicator:checked{image:url(")
                             + Image::ImageCheckBoxChecked + ");}" +
                             "QListWidget::indicator:unchecked{image:url(" +
                             Image::ImageCheckBoxUnchecked + ");}";
    ui->UserList->setStyleSheet(QString::fromStdString(stylesheet));

    m_SwcName = swcName;

    connect(ui->AddData,&QPushButton::clicked,this,[this](){
        ViewSwcNodeData editor(this);
        if(editor.exec() == QDialog::Accepted){
            auto swcNodeInternalData = editor.getSwcNodeInternalData();
            proto::SwcDataV1 swcData;
            auto* newData = swcData.add_swcdata();
            newData->mutable_swcnodeinternaldata()->CopyFrom(swcNodeInternalData);

            proto::CreateSwcNodeDataResponse response;
            if(WrappedCall::addSwcNodeData(m_SwcName, swcData, response, this)){
                QMessageBox::information(this,"Info","Create Swc node successfully!");
                refreshAll();
            }
        }
    });

    connect(ui->ModifyData,&QPushButton::clicked,this,[this](){
        ViewSwcNodeData editor(true,this);

        int currectRow = ui->SwcNodeDataTable->currentRow();
        if(currectRow<0){
            QMessageBox::information(this,"Info","You need to select one row first!");
            return;
        }

        if(m_SwcData.swcdata_size() <= currectRow){
            QMessageBox::critical(this,"Error","Swc Data outdated! Please refresh query result!");
        }
        auto InitSwcNodeData = m_SwcData.swcdata().Get(currectRow);
        auto* InitSwcNodeInternalData= InitSwcNodeData.mutable_swcnodeinternaldata();
        editor.setSwcNodeInternalData(*InitSwcNodeInternalData);
        if(editor.exec() == QDialog::Accepted){
            auto swcNodeInternalData = editor.getSwcNodeInternalData();
            proto::SwcNodeDataV1 swcNodeData;
            swcNodeData.CopyFrom(InitSwcNodeData);
            swcNodeData.mutable_swcnodeinternaldata()->CopyFrom(swcNodeInternalData);

            proto::UpdateSwcNodeDataResponse response;
            if(WrappedCall::modifySwcNodeData(m_SwcName, swcNodeData, response, this)){
                QMessageBox::information(this,"Info","Modify Swc node successfully!");
                refreshAll();
            }
        }
    });

    connect(ui->DeleteData,&QPushButton::clicked,this,[this](){
        int currectRow = ui->SwcNodeDataTable->currentRow();
        if(currectRow<0){
            QMessageBox::information(this,"Info","You need to select one row first!");
            return;
        }

        if(m_SwcData.swcdata_size() <= currectRow){
            QMessageBox::critical(this,"Error","Swc Data outdated! Please refresh query result!");
        }
        auto InitSwcNodeData = m_SwcData.swcdata().Get(currectRow);

        auto result = QMessageBox::information(this,"Info","Are your sure to delete this swc node?",
                                               QMessageBox::StandardButton::Ok,QMessageBox::StandardButton::Cancel);
        if(result == QMessageBox::Ok){
            proto::SwcDataV1 swcData;
            auto* newData = swcData.add_swcdata();
            newData->CopyFrom(InitSwcNodeData);

            proto::DeleteSwcNodeDataResponse response;
            if(WrappedCall::deleteSwcNodeData(m_SwcName, swcData, response, this)){
                QMessageBox::information(this,"Info","Delete Swc node successfully!");
                refreshAll();
            }
        }
    });

    connect(ui->QueryAllBtn,&QPushButton::clicked,this,[this](){
        refreshAll();
    });

    connect(ui->QueryByUserAndTimeBtn,&QPushButton::clicked,this,[this](){
        refreshByQueryOption();
    });

    connect(ui->ExportQueryResultBtn,&QPushButton::clicked,this,[this](){
        proto::GetSwcMetaInfoResponse response;
        if(!WrappedCall::getSwcMetaInfoByName(m_SwcName,response,this)){
            QMessageBox::critical(this,"Error","Get Swc MetaInfo Failed!");
            return;
        }

        std::vector<ExportSwcData> dataList;
        ExportSwcData data;
        data.swcData = m_SwcData;
        data.swcMetaInfo = response.swcinfo();
        dataList.push_back(data);

        ViewEportSwcToFile view(dataList,false,this);
        view.exec();
    });


    refreshUserArea();
}

EditorSwcNode::~EditorSwcNode() {
    delete ui;
}

void EditorSwcNode::refreshUserArea() {
    proto::GetAllUserResponse response;
    WrappedCall::getAllUserMetaInfo(response, this);
    for (int i = 0; i < response.userinfo_size(); i++) {
        auto userInfo = response.userinfo().Get(i);
        auto *item = new QListWidgetItem;
        item->setText(QString::fromStdString(userInfo.name()));
        item->setCheckState(Qt::Checked);
        ui->UserList->addItem(item);
    }
}

void EditorSwcNode::refreshTable(){
    ui->SwcNodeDataTable->clear();
    ui->SwcNodeDataTable->setColumnCount(12);
    QStringList headerLabels;
    headerLabels
            << "n"
            << "type"
            << "x"
            << "y"
            << "z"
            << "radius"
            << "parent"
            << "seg_id"
            << "level"
            << "mode"
            << "timestamp"
            << "feature_value";
    ui->SwcNodeDataTable->setHorizontalHeaderLabels(headerLabels);
}

void EditorSwcNode::refreshAll() {
    proto::GetSwcFullNodeDataResponse response;
    if(!WrappedCall::getSwcFullNodeData(m_SwcName, response, this)){
        QMessageBox::critical(this,"Error","Get Swc Node Data Failed!");
    }

    refreshTable();
    ui->SwcNodeDataTable->setRowCount(response.swcnodedata().swcdata_size());

    auto swcData = response.swcnodedata();
    loadSwcData(swcData);
}

void EditorSwcNode::refreshByQueryOption() {
    proto::GetSwcNodeDataListByTimeAndUserResponse response;

    google::protobuf::Timestamp startTime;
    startTime.set_seconds(ui->StartTime->dateTime().toSecsSinceEpoch());
    google::protobuf::Timestamp endTime;
    endTime.set_seconds(ui->EndTime->dateTime().toSecsSinceEpoch());

    int checkedUserNumber = 0;
    std::vector<std::string> checkedUserNames;
    for(int i=0;i<ui->UserList->count();i++){
        if(ui->UserList->item(i)->checkState() == Qt::Checked){
            checkedUserNames.push_back(ui->UserList->item(i)->text().toStdString());
            checkedUserNumber++;
        }
    }

    if(checkedUserNumber > 1){
        QMessageBox::information(this, "Warning", "Current only support select one user as query option!");
        return;
    }

    std::string userName = checkedUserNames[0];
    if(checkedUserNumber == 0){
        userName = "";
    }

    WrappedCall::getSwcNodeDataListByTimeAndUserResponse(m_SwcName, userName, startTime, endTime, response, this);

    refreshTable();
    ui->SwcNodeDataTable->setRowCount(response.swcnodedata().swcdata_size());

    auto swcData = response.swcnodedata();
    loadSwcData(swcData);
}

void EditorSwcNode::loadSwcData(proto::SwcDataV1& swcData) {
    for (int i = 0; i < swcData.swcdata_size(); i++) {
        auto info = swcData.swcdata().Get(i);
        ui->SwcNodeDataTable->setItem(i, 0,
                                      new QTableWidgetItem(QString::fromStdString(
                                              std::to_string(info.mutable_swcnodeinternaldata()->n()))));
        ui->SwcNodeDataTable->setItem(i, 1,
                                      new QTableWidgetItem(
                                              QString::fromStdString(
                                                      std::to_string(info.mutable_swcnodeinternaldata()->type()))));
        ui->SwcNodeDataTable->setItem(i, 2,
                                      new QTableWidgetItem(
                                              QString::fromStdString(
                                                      std::to_string(info.mutable_swcnodeinternaldata()->x()))));
        ui->SwcNodeDataTable->setItem(i, 3,
                                      new QTableWidgetItem(
                                              QString::fromStdString(
                                                      std::to_string(info.mutable_swcnodeinternaldata()->y()))));
        ui->SwcNodeDataTable->setItem(i, 4,
                                      new QTableWidgetItem(
                                              QString::fromStdString(
                                                      std::to_string(info.mutable_swcnodeinternaldata()->z()))));
        ui->SwcNodeDataTable->setItem(i, 5,
                                      new QTableWidgetItem(
                                              QString::fromStdString(
                                                      std::to_string(info.mutable_swcnodeinternaldata()->radius()))));
        ui->SwcNodeDataTable->setItem(i, 6,
                                      new QTableWidgetItem(
                                              QString::fromStdString(
                                                      std::to_string(info.mutable_swcnodeinternaldata()->parent()))));
        ui->SwcNodeDataTable->setItem(i, 7,
                                      new QTableWidgetItem(
                                              QString::fromStdString(
                                                      std::to_string(info.mutable_swcnodeinternaldata()->seg_id()))));
        ui->SwcNodeDataTable->setItem(i, 8,
                                      new QTableWidgetItem(
                                              QString::fromStdString(
                                                      std::to_string(info.mutable_swcnodeinternaldata()->level()))));
        ui->SwcNodeDataTable->setItem(i, 9,
                                      new QTableWidgetItem(
                                              QString::fromStdString(
                                                      std::to_string(info.mutable_swcnodeinternaldata()->mode()))));
        ui->SwcNodeDataTable->setItem(i, 10,
                                      new QTableWidgetItem(
                                              QString::fromStdString(std::to_string(
                                                      info.mutable_swcnodeinternaldata()->timestamp()))));
        ui->SwcNodeDataTable->setItem(i, 11,
                                      new QTableWidgetItem(
                                              QString::fromStdString(std::to_string(
                                                      info.mutable_swcnodeinternaldata()->feature_value()))));
    }

    m_SwcData.CopyFrom(swcData);

    ui->SwcNodeDataTable->resizeColumnsToContents();
}
