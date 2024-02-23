#include "EditorSwcVersionControl.h"

#include <Message/Request.pb.h>
#include <Message/Response.pb.h>
#include "ui_EditorSwcVersionControl.h"
#include "src/framework/service/WrappedCall.h"


EditorSwcVersionControl::EditorSwcVersionControl(const std::string &swcName, QWidget *parent) : QDialog(parent),
                                                                                                ui(new Ui::EditorSwcVersionControl),
                                                                                                m_SwcName(swcName) {
    ui->setupUi(this);

    m_DataFlowGraphModel.add
    m_GraphicsView.setScene(&m_DataFlowGraphicsScene);
    ui->GraphicsViewLayout->addWidget(&m_GraphicsView);

    ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());

    connect(ui->ExportBtn, &QPushButton::clicked, this, [&]() {
        getSwcLastSnapshot();
    });

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
            if (!WrappedCall::RevertSwcVersion(m_SwcName, endTime, response, this)) {
                return;
            }

            QMessageBox::information(this, "Info", "Revert Swc Version Successfully!");
        }
    });
}

EditorSwcVersionControl::~EditorSwcVersionControl() {
    delete ui;
}

void EditorSwcVersionControl::getSwcLastSnapshot() {
    grpc::ClientContext context;

    proto::GetAllSnapshotMetaInfoRequest request;
    proto::GetAllSnapshotMetaInfoResponse response;
    WrappedCall::setCommonRequestField(request);

    request.set_swcname(m_SwcName);

    if (auto status = RpcCall::getInstance().Stub()->GetAllSnapshotMetaInfo(&context, request, &response); status.
            ok()) {
        if (response.metainfo().status()) {
            if (response.swcsnapshotlist_size() == 0) {
                QMessageBox::critical(this, "Error",
                                      "You need to create at least one snapshot to enable version control system!");
            } else {
                auto queryEndTime = ui->dateTimeEdit->dateTime().toSecsSinceEpoch();
                int64_t currentSnapTime = 0;
                proto::SwcSnapshotMetaInfoV1 currentSnap;
                for (auto &snap: response.swcsnapshotlist()) {
                    if (snap.createtime().seconds() <= queryEndTime && snap.createtime().seconds() >= currentSnapTime) {
                        currentSnapTime = snap.createtime().seconds();
                        currentSnap.CopyFrom(snap);
                    }
                }
                if (currentSnapTime == 0) {
                    QMessageBox::critical(this, "Error",
                                          "No snapshot found based on the end time you provided! End time must based on one snapshot!");
                } else {
                    getSwcIncrementRecord(currentSnap, queryEndTime);
                }
            }
        } else {
            QMessageBox::critical(this, "Error", QString::fromStdString(response.metainfo().message()));
        }
    } else {
        QMessageBox::critical(this, "Error", QString::fromStdString(status.error_message()));
    }
}

void EditorSwcVersionControl::getSwcIncrementRecord(proto::SwcSnapshotMetaInfoV1 snapshot, int64_t endTime) {
    grpc::ClientContext context;

    proto::GetAllIncrementOperationMetaInfoRequest request;
    proto::GetAllIncrementOperationMetaInfoResponse response;
    WrappedCall::setCommonRequestField(request);

    request.set_swcname(m_SwcName);

    proto::SwcIncrementOperationMetaInfoV1 currentIncrement;

    if (auto status = RpcCall::getInstance().Stub()->GetAllIncrementOperationMetaInfo(&context, request, &response);
            status.ok()) {
        if (response.metainfo().status()) {
            for (auto &increment: response.swcincrementoperationmetainfo()) {
                if (increment.startsnapshot() == snapshot.swcsnapshotcollectionname()) {
                    currentIncrement.CopyFrom(increment);
                    break;
                }
            }
            if (currentIncrement.startsnapshot().empty()) {
                QMessageBox::critical(this, "Error", "Cannot found the corresponding increment record!");
            } else {
                proto::GetSnapshotResponse snapResponse;
                if (!WrappedCall::getSwcSnapshot(snapshot.swcsnapshotcollectionname(), snapResponse, this)) {
                    return;
                }
                std::vector<proto::SwcNodeDataV1> nodeData;
                for (auto &node: snapResponse.swcnodedata().swcdata()) {
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

                for (auto &increment: incrementResponse.swcincrementoperationlist().swcincrementoperation()) {
                    promoteOperation(nodeData, increment, endTime);
                }

                proto::GetSwcMetaInfoResponse swc_meta_info_response;
                if (!WrappedCall::getSwcMetaInfoByName(m_SwcName, swc_meta_info_response, this)) {
                    return;
                }

                proto::SwcDataV1 exportSwcData;

                for (auto &node: nodeData) {
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
        } else {
            QMessageBox::critical(this, "Error", QString::fromStdString(response.metainfo().message()));
        }
    } else {
        QMessageBox::critical(this, "Error", QString::fromStdString(status.error_message()));
    }
}

void EditorSwcVersionControl::promoteOperation(std::vector<proto::SwcNodeDataV1> &nodeData,
                                               const proto::SwcIncrementOperationV1 &op, int64_t endTime) {
    int a = op.createtime().seconds();
    if (op.createtime().seconds() <= endTime) {
        switch (op.incrementoperation()) {
            case proto::Unknown:
                break;
            case proto::Create: {
                for (auto &newData: op.swcdata().swcdata()) {
                    nodeData.push_back(newData);
                }
                break;
            }
            case proto::Delete: {
                for (auto &newData: op.swcdata().swcdata()) {
                    std::erase_if(nodeData, [&](proto::SwcNodeDataV1 &node) {
                        if (node.base().uuid() == newData.base().uuid()) {
                            return true;
                        }
                        return false;
                    });
                }

                break;
            }
            case proto::Update: {
                for (auto &newData: op.swcdata().swcdata()) {
                    for (auto &node: nodeData) {
                        if (node.base().uuid() == newData.base().uuid()) {
                            node.CopyFrom(newData);
                        }
                    }
                }
                break;
            }
            default:;
        }
    }
}
