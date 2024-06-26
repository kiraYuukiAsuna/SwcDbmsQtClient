#include "EditorSwcVersionControl.h"

#include <Message/Request.pb.h>
#include <Message/Response.pb.h>
#include "ui_EditorSwcVersionControl.h"
#include "src/framework/service/WrappedCall.h"
#include "src/GraphModel/Registry.hpp"

#include "Renderer/SwcRenderer.h"

EditorSwcVersionControl::EditorSwcVersionControl(const std::string&swcUuid, QWidget* parent) : QDialog(parent),
    ui(new Ui::EditorSwcVersionControl),
    m_SwcUuid(swcUuid),
    m_DataFlowGraphModel(
        registerDataModels()) {
    ui->setupUi(this);

    m_DataFlowGraphicsScene = new QtNodes::DataFlowGraphicsScene(m_DataFlowGraphModel, this);
    m_GraphicsView.setScene(m_DataFlowGraphicsScene);

    ui->GraphicsViewLayout->addWidget(&m_GraphicsView);

    ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());

    connect(ui->ExportBtn, &QPushButton::clicked, this, [&]() {
        getSwcLastSnapshot();
    });

    m_GraphicsView.setContextMenuPolicy(Qt::NoContextMenu);

    refreshVersionGraph();

    connect(ui->Revert, &QPushButton::clicked, this, [&]() {
        m_EndTime = ui->dateTimeEdit->dateTime().toSecsSinceEpoch();
        auto status = QMessageBox::information(this, "Info",
                                               "Are your sure to revert this swc to " +
                                               QDateTime::fromSecsSinceEpoch(m_EndTime).toString() +
                                               "? You may loss all your changes after that! So please export one backup before continue this operation!");
        if (status == QMessageBox::StandardButton::Ok) {
            if (!m_HasExportedStatus) {
                QMessageBox::information(this, "Info", "Please export one backup before reverting this swc!");
                return;
            }

            if (m_SnapshotName.empty()) {
                QMessageBox::information(this, "Info", "No snapshot found based on the end time you provided!");
                return;
            }

            proto::RevertSwcVersionResponse response;
            google::protobuf::Timestamp endTime;
            endTime.set_seconds(m_EndTime);
            if (!WrappedCall::RevertSwcVersionByUuid(m_SwcUuid, endTime, response, this)) {
                return;
            }

            QMessageBox::information(this, "Info", "Revert Swc Version Successfully!");
        }
    });

    connect(ui->Refresh, &QPushButton::clicked, this, [&]() {
        refreshVersionGraph();
    });

    connect(ui->ExportSelectedVersion, &QPushButton::clicked, this, [&]() {
        auto selectedNodes = m_DataFlowGraphicsScene->selectedNodes();
        if (selectedNodes.empty()) {
            QMessageBox::information(this, "Error",
                                     "Select One Snapshot to enter visualization mode!");
            return;
        }

        auto selectedNode = selectedNodes[0];
        auto snapShotCName = m_DataFlowGraphModel.nodeData(selectedNode, QtNodes::NodeRole::Caption).toString();
        auto status = QMessageBox::information(this, "Info", "Export this snapshot? Snapshot: " + snapShotCName);
        if (status != QMessageBox::StandardButton::Ok) {
            return;
        }

        proto::GetSwcMetaInfoResponse response;
        if (!WrappedCall::getSwcMetaInfoByUuid(m_SwcUuid, response, this)) {
            return;
        }

        std::vector<ExportSwcData> exportSwcData;
        exportSwcData.push_back({response.swcinfo(), {}, true, snapShotCName.toStdString()});
        ViewExportSwcToFile view(exportSwcData, true, nullptr);
        view.exec();
    });

    connect(ui->VisualizeBtn, &QPushButton::clicked, this, [&]() {
        auto selectedNodes = m_DataFlowGraphicsScene->selectedNodes();
        if (selectedNodes.empty()) {
            QMessageBox::information(this, "Error",
                                     "Select One Snapshot to enter visualization mode!");
            return;
        }

        auto selectedNode = selectedNodes[0];
        auto snapShotCName = m_DataFlowGraphModel.nodeData(selectedNode, QtNodes::NodeRole::Caption).toString();
        auto status = QMessageBox::information(this, "Info", "Visualize this snapshot? Snapshot: " + snapShotCName);
        if (status != QMessageBox::StandardButton::Ok) {
            return;
        }

        proto::GetSnapshotResponse snapResponse;
        if (!WrappedCall::getSwcSnapshot(snapShotCName.toStdString(), snapResponse, this)) {
            return;
        }

        SwcRendererCreateInfo createInfo;
        createInfo.swcData = snapResponse.swcnodedata();
        SwcRendererDailog renderer(createInfo, this);;
        renderer.exec();
    });

    connect(ui->VisualizeDiffBtn, &QPushButton::clicked, this, [&]() {
        auto selectedNodes = m_DataFlowGraphicsScene->selectedNodes();
        if (selectedNodes.size() != 2) {
            QMessageBox::information(this, "Error",
                                     "Select Two Snapshot to enter visualization mode!");
            return;
        }

        auto selectedNode1 = selectedNodes[0];
        auto snapShotCName1 = m_DataFlowGraphModel.nodeData(selectedNode1, QtNodes::NodeRole::Caption).toString().
                toStdString();

        auto selectedNode2 = selectedNodes[1];
        auto snapShotCName2 = m_DataFlowGraphModel.nodeData(selectedNode2, QtNodes::NodeRole::Caption).toString().
                toStdString();

        google::protobuf::Timestamp selectedNode1Time;
        google::protobuf::Timestamp selectedNode2Time;
        for (auto&snap: m_SwcSnapshots) {
            if (snap.swcsnapshotcollectionname() == snapShotCName1) {
                selectedNode1Time = snap.createtime();
                break;
            }
        }

        for (auto&snap: m_SwcSnapshots) {
            if (snap.swcsnapshotcollectionname() == snapShotCName2) {
                selectedNode2Time = snap.createtime();
                break;
            }
        }

        std::string oldCName;
        std::string newCName;

        if (isEarlier(selectedNode1Time, selectedNode2Time)) {
            oldCName = snapShotCName1;
            newCName = snapShotCName2;
        }
        else {
            newCName = snapShotCName1;
            oldCName = snapShotCName2;
        }

        auto status = QMessageBox::information(this, "Info",
                                               "Visualize two snapshot in diff mode? Old Snapshot: " +
                                               QString::fromStdString(oldCName) + " ,New Snapshot: " +
                                               QString::fromStdString(newCName));
        if (status != QMessageBox::StandardButton::Ok) {
            return;
        }

        proto::GetSnapshotResponse oldSnapResponse;
        if (!WrappedCall::getSwcSnapshot(oldCName, oldSnapResponse, this)) {
            return;
        }

        proto::GetSnapshotResponse newSnapResponse;
        if (!WrappedCall::getSwcSnapshot(newCName, newSnapResponse, this)) {
            return;
        }

        SwcRendererCreateInfo createInfo;
        createInfo.mode = SwcRendererMode::eVisualizeDiffSwc;
        createInfo.swcData = oldSnapResponse.swcnodedata();
        createInfo.newSwcData = newSnapResponse.swcnodedata();
        SwcRendererDailog renderer(createInfo, this);;
        renderer.exec();
    });
}

EditorSwcVersionControl::~EditorSwcVersionControl() {
    delete ui;
}

void EditorSwcVersionControl::refreshVersionGraph() {
    getAllSnapshot();
    getAllSwcIncrementRecord();

    for (auto&nodeId: m_DataFlowGraphModel.allNodeIds()) {
        m_DataFlowGraphModel.deleteNode(nodeId);
    }

    std::map<std::string, QtNodes::NodeId> snapshotNodeMap;

    for (int idx = 0; idx < m_SwcSnapshots.size(); idx++) {
        QtNodes::NodeId const newId = m_DataFlowGraphModel.addNode("SnapshotDelegateModel");
        m_DataFlowGraphModel.setNodeData(newId, QtNodes::NodeRole::Position,
                                         QPointF{static_cast<qreal>(idx * 900), 0});
        m_DataFlowGraphModel.setNodeData(newId, QtNodes::NodeRole::Caption,
                                         QString::fromStdString(m_SwcSnapshots[idx].swcsnapshotcollectionname()));
        m_DataFlowGraphModel.setNodeData(newId, QtNodes::NodeRole::Size,
                                         QSize{360, 100});
        snapshotNodeMap.insert({m_SwcSnapshots[idx].swcsnapshotcollectionname(), newId});
    }

    for (int idx = 0; idx < m_SwcIncrements.size(); idx++) {
        QtNodes::NodeId const newId = m_DataFlowGraphModel.addNode("IncrementOperationDelegateModel");
        m_DataFlowGraphModel.setNodeData(newId, QtNodes::NodeRole::Position,
                                         QPointF{static_cast<qreal>(400 + idx * 900), 300});
        m_DataFlowGraphModel.setNodeData(newId, QtNodes::NodeRole::Caption,
                                         QString::fromStdString(
                                             m_SwcIncrements[idx].incrementoperationcollectionname()));
        m_DataFlowGraphModel.setNodeData(newId, QtNodes::NodeRole::Size,
                                         QSize{300, 100});
        for (int idx2 = 0; idx2 < m_SwcSnapshots.size(); idx2++) {
            auto snap = m_SwcSnapshots[idx2];
            if (m_SwcIncrements[idx].startsnapshot() == snap.swcsnapshotcollectionname()) {
                m_DataFlowGraphModel.addConnection({
                    snapshotNodeMap[snap.swcsnapshotcollectionname()], 0, newId, 0
                });
                if (idx2 + 1 < m_SwcSnapshots.size()) {
                    m_DataFlowGraphModel.addConnection({
                        newId, 0, snapshotNodeMap[m_SwcSnapshots[idx + 1].swcsnapshotcollectionname()], 0
                    });
                }
                break;
            }
        }
    }
}

void EditorSwcVersionControl::getSwcLastSnapshot() {
    grpc::ClientContext context;

    proto::GetAllSnapshotMetaInfoRequest request;
    proto::GetAllSnapshotMetaInfoResponse response;
    WrappedCall::setCommonRequestField(request);

    request.set_swcuuid(m_SwcUuid);

    if (auto status = RpcCall::getInstance().Stub()->GetAllSnapshotMetaInfo(&context, request, &response); status.
        ok()) {
        if (response.metainfo().status()) {
            if (response.swcsnapshotlist_size() == 0) {
                QMessageBox::critical(this, "Error",
                                      "You need to create at least one snapshot to enable version control system!");
            }
            else {
                auto queryEndTime = ui->dateTimeEdit->dateTime().toSecsSinceEpoch();
                int64_t currentSnapTime = 0;
                proto::SwcSnapshotMetaInfoV1 currentSnap;
                for (auto&snap: response.swcsnapshotlist()) {
                    if (snap.createtime().seconds() <= queryEndTime && snap.createtime().seconds() >= currentSnapTime) {
                        currentSnapTime = snap.createtime().seconds();
                        currentSnap.CopyFrom(snap);
                    }
                }
                if (currentSnapTime == 0) {
                    QMessageBox::critical(this, "Error",
                                          "No snapshot found based on the end time you provided! End time must based on one snapshot!");
                }
                else {
                    getSwcIncrementRecord(currentSnap, queryEndTime);
                }
            }
        }
        else {
            QMessageBox::critical(this, "Error", QString::fromStdString(response.metainfo().message()));
        }
    }
    else {
        QMessageBox::critical(this, "Error", QString::fromStdString(status.error_message()));
    }
}

void EditorSwcVersionControl::getSwcIncrementRecord(proto::SwcSnapshotMetaInfoV1 snapshot, int64_t endTime) {
    grpc::ClientContext context;

    proto::GetAllIncrementOperationMetaInfoRequest request;
    proto::GetAllIncrementOperationMetaInfoResponse response;
    WrappedCall::setCommonRequestField(request);

    request.set_swcuuid(m_SwcUuid);

    proto::SwcIncrementOperationMetaInfoV1 currentIncrement;

    if (auto status = RpcCall::getInstance().Stub()->GetAllIncrementOperationMetaInfo(&context, request, &response);
        status.ok()) {
        if (response.metainfo().status()) {
            for (auto&increment: response.swcincrementoperationmetainfo()) {
                if (increment.startsnapshot() == snapshot.swcsnapshotcollectionname()) {
                    currentIncrement.CopyFrom(increment);
                    break;
                }
            }
            if (currentIncrement.startsnapshot().empty()) {
                QMessageBox::critical(this, "Error", "Cannot found the corresponding increment record!");
            }
            else {
                proto::GetSnapshotResponse snapResponse;
                if (!WrappedCall::getSwcSnapshot(snapshot.swcsnapshotcollectionname(), snapResponse, this)) {
                    return;
                }
                std::vector<proto::SwcNodeDataV1> nodeData;
                for (auto&node: snapResponse.swcnodedata().swcdata()) {
                    nodeData.push_back(node);
                }

                proto::GetIncrementOperationResponse incrementResponse;
                if (!WrappedCall::getSwcIncrementRecord(currentIncrement.incrementoperationcollectionname(),
                                                        incrementResponse,
                                                        this)) {
                    return;
                }

                m_EndTime = endTime;
                m_SnapshotName = snapshot.swcsnapshotcollectionname();
                m_IncremrntOpName = currentIncrement.incrementoperationcollectionname();

                for (auto&increment: incrementResponse.swcincrementoperationlist().swcincrementoperation()) {
                    promoteOperation(nodeData, increment, endTime);
                }

                proto::GetSwcMetaInfoResponse swc_meta_info_response;
                if (!WrappedCall::getSwcMetaInfoByUuid(m_SwcUuid, swc_meta_info_response, this)) {
                    return;
                }

                proto::SwcDataV1 exportSwcData;

                for (auto&node: nodeData) {
                    auto data = exportSwcData.add_swcdata();
                    data->CopyFrom(node);
                }

                // export data
                ExportSwcData exportData{
                    .swcMetaInfo = swc_meta_info_response.swcinfo(),
                    .swcData = exportSwcData,
                    .isSnapshot = true,
                    .swcSnapshotCollectionName = ""
                };

                std::vector<ExportSwcData> exportDataVec;
                exportDataVec.push_back(exportData);

                ViewExportSwcToFile view(exportDataVec, false, this);
                view.exec();
                m_HasExportedStatus = true;
            }
        }
        else {
            QMessageBox::critical(this, "Error", QString::fromStdString(response.metainfo().message()));
        }
    }
    else {
        QMessageBox::critical(this, "Error", QString::fromStdString(status.error_message()));
    }
}

void EditorSwcVersionControl::promoteOperation(std::vector<proto::SwcNodeDataV1>&nodeData,
                                               const proto::SwcIncrementOperationV1&op, int64_t endTime) {
    if (op.createtime().seconds() <= endTime) {
        switch (op.incrementoperation()) {
            case proto::Unknown:
                break;
            case proto::Create: {
                for (auto&newData: op.swcdata().swcdata()) {
                    nodeData.push_back(newData);
                }
                break;
            }
            case proto::Delete: {
                for (auto&deletedData: op.swcdata().swcdata()) {
                    auto it = std::find_if(nodeData.begin(), nodeData.end(), [&](proto::SwcNodeDataV1&node) {
                        return node.base().uuid() == deletedData.base().uuid();
                    });
                    if (it != nodeData.end()) {
                        nodeData.erase(it);
                    }
                }

                int counter = 1;
                proto::SwcNodeDataV1* lastNode = nullptr;
                for (auto&node: nodeData) {
                    node.mutable_swcnodeinternaldata()->set_n(counter++);
                    if (lastNode != nullptr) {
                        if (lastNode->swcnodeinternaldata().parent() != -1) {
                            lastNode->mutable_swcnodeinternaldata()->set_parent(node.swcnodeinternaldata().n());
                        }
                    }
                    lastNode = &node;
                }
                break;
            }
            case proto::Update: {
                for (auto&updateData: op.swcdata().swcdata()) {
                    for (auto&node: nodeData) {
                        if (node.base().uuid() == updateData.base().uuid()) {
                            node.CopyFrom(updateData);
                        }
                    }
                }
                break;
            }
            case proto::UpdateNParent: {
                std::unordered_map<std::string, int> indexMap;
                for (auto&node: nodeData) {
                    indexMap[node.base().uuid()] = node.swcnodeinternaldata().n();
                }

                for (auto&updateData: op.nodenparent()) {
                    auto iter = indexMap.find(updateData.nodeuuid());
                    if (iter != indexMap.end()) {
                        nodeData.at(iter->second).mutable_swcnodeinternaldata()->set_n(updateData.n());
                        nodeData.at(iter->second).mutable_swcnodeinternaldata()->set_parent(updateData.parent());
                    }
                }
                break;
            }
            case proto::ClearAll: {
                nodeData.clear();
                break;
            }
            case proto::OverwriteAll: {
                nodeData.clear();
                for (auto&newData: op.swcdata().swcdata()) {
                    nodeData.push_back(newData);
                }
                break;
            }
            default: ;
        }
    }
}

void EditorSwcVersionControl::getAllSnapshot() {
    grpc::ClientContext context;
    proto::GetAllSnapshotMetaInfoRequest request;
    proto::GetAllSnapshotMetaInfoResponse response;
    WrappedCall::setCommonRequestField(request);
    request.set_swcuuid(m_SwcUuid);

    auto status = RpcCall::getInstance().Stub()->GetAllSnapshotMetaInfo(&context, request, &response);
    if (status.ok()) {
        if (response.has_metainfo() && response.metainfo().status() == true) {
            m_SwcSnapshots.clear();
            for (auto&snapshot: response.swcsnapshotlist()) {
                m_SwcSnapshots.push_back(snapshot);
            }
        }
        else {
            QMessageBox::critical(this, "Error", QString::fromStdString(response.metainfo().message()));
        }
    }
    else {
        QMessageBox::critical(this, "Error", QString::fromStdString(status.error_message()));
    }
}

void EditorSwcVersionControl::getAllSwcIncrementRecord() {
    proto::GetAllIncrementOperationMetaInfoResponse response;

    if (WrappedCall::getAllSwcIncrementRecordByUuid(m_SwcUuid, response, this)) {
        m_SwcIncrements.clear();
        for (auto&incrementRecordCollection: response.swcincrementoperationmetainfo()) {
            m_SwcIncrements.push_back(incrementRecordCollection);
        }
    }
}
