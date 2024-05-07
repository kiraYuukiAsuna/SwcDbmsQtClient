#include <QListWidgetItem>
#include "EditorSwcNode.h"
#include "ui_EditorSwcNode.h"
#include "src/framework/service/WrappedCall.h"
#include "src/framework/defination/ImageDefination.h"
#include "ViewSwcNodeData.h"
#include "ViewExportSwcToFile.h"
#include "Renderer/SwcRenderer.h"

EditorSwcNode::EditorSwcNode(const std::string&swcName, QWidget* parent) : QWidget(parent), ui(new Ui::EditorSwcNode) {
    ui->setupUi(this);
    std::string stylesheet = std::string("QListWidget::indicator:checked{image:url(")
                             + Image::ImageCheckBoxChecked + ");}" +
                             "QListWidget::indicator:unchecked{image:url(" +
                             Image::ImageCheckBoxUnchecked + ");}";
    ui->UserList->setStyleSheet(QString::fromStdString(stylesheet));

    m_SwcName = swcName;

    ui->StartTime->setDateTime(QDateTime(QDate::currentDate(), QTime::currentTime()));
    ui->EndTime->setDateTime(QDateTime(QDate::currentDate(), QTime::currentTime()));

    refreshUserArea();

    connect(ui->AddData, &QPushButton::clicked, this, [this]() {
        ViewSwcNodeData editor(this);
        if (editor.exec() == QDialog::Accepted) {
            auto swcNodeInternalData = editor.getSwcNodeInternalData();
            proto::SwcDataV1 swcData;
            auto* newData = swcData.add_swcdata();
            newData->mutable_swcnodeinternaldata()->CopyFrom(swcNodeInternalData);

            proto::CreateSwcNodeDataResponse response;
            if (WrappedCall::addSwcNodeData(m_SwcName, swcData, response, this)) {
                QMessageBox::information(this, "Info", "Create Swc node successfully!");
                refreshAll();
            }
        }
    });

    connect(ui->ModifyData, &QPushButton::clicked, this, [this]() {
        ViewSwcNodeData editor(true, this);

        QModelIndex currentIndex = ui->SwcNodeDataTable->selectionModel()->currentIndex();
        int currentRow = currentIndex.row();
        if (currentRow < 0) {
            QMessageBox::information(this, "Info", "You need to select one row first!");
            return;
        }

        if (m_SwcData.swcdata_size() <= currentRow) {
            QMessageBox::critical(this, "Error", "Swc Data outdated! Please refresh query result!");
        }
        auto InitSwcNodeData = m_SwcData.swcdata().Get(currentRow);
        auto* InitSwcNodeInternalData = InitSwcNodeData.mutable_swcnodeinternaldata();
        editor.setSwcNodeInternalData(*InitSwcNodeInternalData);
        if (editor.exec() == QDialog::Accepted) {
            auto swcNodeInternalData = editor.getSwcNodeInternalData();
            proto::SwcNodeDataV1 swcNodeData;
            swcNodeData.CopyFrom(InitSwcNodeData);
            swcNodeData.mutable_swcnodeinternaldata()->CopyFrom(swcNodeInternalData);

            proto::SwcDataV1 swcData;
            swcData.add_swcdata()->CopyFrom(swcNodeData);

            proto::UpdateSwcNodeDataResponse response;
            if (WrappedCall::modifySwcNodeData(m_SwcName, swcData, response, this)) {
                QMessageBox::information(this, "Info", "Modify Swc node successfully!");
                refreshAll();
            }
        }
    });

    connect(ui->DeleteData, &QPushButton::clicked, this, [this]() {
        QModelIndex currentIndex = ui->SwcNodeDataTable->selectionModel()->currentIndex();
        int currentRow = currentIndex.row();
        if (currentRow < 0) {
            QMessageBox::information(this, "Info", "You need to select one row first!");
            return;
        }

        if (m_SwcData.swcdata_size() <= currentRow) {
            QMessageBox::critical(this, "Error", "Swc Data outdated! Please refresh query result!");
        }
        auto InitSwcNodeData = m_SwcData.swcdata().Get(currentRow);

        auto result = QMessageBox::information(this, "Info", "Are your sure to delete this swc node?",
                                               QMessageBox::StandardButton::Ok, QMessageBox::StandardButton::Cancel);
        if (result == QMessageBox::Ok) {
            proto::SwcDataV1 swcData;
            auto* newData = swcData.add_swcdata();
            newData->CopyFrom(InitSwcNodeData);

            proto::DeleteSwcNodeDataResponse response;
            if (WrappedCall::deleteSwcNodeData(m_SwcName, swcData, response, this)) {
                QMessageBox::information(this, "Info", "Delete Swc node successfully!");
                refreshAll();
            }
        }
    });

    connect(ui->QueryAllBtn, &QPushButton::clicked, this, [this]() {
        refreshAll();
    });

    connect(ui->QueryByUserAndTimeBtn, &QPushButton::clicked, this, [this]() {
        refreshByQueryOption();
    });

    connect(ui->ExportQueryResultBtn, &QPushButton::clicked, this, [this]() {
        proto::GetSwcMetaInfoResponse response;
        if (!WrappedCall::getSwcMetaInfoByName(m_SwcName, response, this)) {
            QMessageBox::critical(this, "Error", "Get Swc MetaInfo Failed!");
            return;
        }

        std::vector<ExportSwcData> dataList;
        ExportSwcData data;
        data.swcData = m_SwcData;
        data.swcMetaInfo = response.swcinfo();
        dataList.push_back(data);

        ViewExportSwcToFile view(dataList, false, this);
        view.exec();
    });

    connect(ui->VisualizeBtn, &QPushButton::clicked, this, [this]() {
        SwcRendererCreateInfo createInfo;
        createInfo.swcData = m_SwcData;
        SwcRendererDailog renderer(createInfo, this);;
        renderer.exec();
    });
}

EditorSwcNode::~EditorSwcNode() {
    delete ui;
}

void EditorSwcNode::refreshUserArea() {
    proto::GetAllUserResponse response;
    WrappedCall::getAllUserMetaInfo(response, this);
    for (int i = 0; i < response.userinfo_size(); i++) {
        auto userInfo = response.userinfo().Get(i);
        auto* item = new QListWidgetItem;
        item->setText(QString::fromStdString(userInfo.name()));
        item->setCheckState(Qt::Unchecked);
        ui->UserList->addItem(item);
    }
}

void EditorSwcNode::refreshAll() {
    proto::GetSwcFullNodeDataResponse response;
    if (!WrappedCall::getSwcFullNodeData(m_SwcName, response, this)) {
        QMessageBox::critical(this, "Error", "Get Swc Node Data Failed!");
    }

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
    for (int i = 0; i < ui->UserList->count(); i++) {
        if (ui->UserList->item(i)->checkState() == Qt::Checked) {
            checkedUserNames.push_back(ui->UserList->item(i)->text().toStdString());
            checkedUserNumber++;
        }
    }

    if (checkedUserNumber > 1) {
        QMessageBox::information(this, "Warning", "Current only support select one user as query option!");
        return;
    }

    if (checkedUserNumber == 0) {
        QMessageBox::information(this, "Warning", "No user has been selected!");
        return;
    }
    std::string userName = checkedUserNames[0];


    WrappedCall::getSwcNodeDataListByTimeAndUserResponse(m_SwcName, userName, startTime, endTime, response, this);

    auto swcData = response.swcnodedata();
    loadSwcData(swcData);
}

void EditorSwcNode::loadSwcData(proto::SwcDataV1&swcData) {
    m_SwcData.CopyFrom(swcData);

    auto* model = new SwcTableModel(m_SwcData, this);
    ui->SwcNodeDataTable->setModel(model);
    ui->SwcNodeDataTable->resizeColumnsToContents();
}
