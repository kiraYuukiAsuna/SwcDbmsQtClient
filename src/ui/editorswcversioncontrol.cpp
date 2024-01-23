#include "editorswcversioncontrol.h"

#include <grpcpp/client_context.h>
#include <Message/Request.pb.h>
#include <Message/Response.pb.h>

#include "ui_EditorSwcVersionControl.h"
#include "src/framework/service/WrappedCall.h"


EditorSwcVersionControl::EditorSwcVersionControl(const std::string&swcName, QWidget* parent) : QDialog(parent),
                                                                                               ui(new Ui::EditorSwcVersionControl),m_SwcName(swcName) {
    ui->setupUi(this);

    connect(ui->ExportBtn,&QPushButton::clicked,this,[&]() {
       getSwcLastSnapshot();
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

    if(auto status = RpcCall::getInstance().Stub()->GetAllSnapshotMetaInfo(&context,request,&response); status.ok()) {
        if(response.metainfo().status()) {
            if(response.swcsnapshotlist_size()==0) {
                QMessageBox::critical(this, "Error", "You need to create at least one snapshot to enable version control system!");
            }else {
                auto queryEndTime = ui->dateTimeEdit->dateTime().toSecsSinceEpoch();
                int64_t currentSnapTime = 0;
                proto::SwcSnapshotMetaInfoV1 currentSnap;
                for(auto& snap : response.swcsnapshotlist()) {
                    if(snap.createtime().seconds() <= queryEndTime && snap.createtime().seconds()>=currentSnapTime) {
                        currentSnapTime = snap.createtime().seconds();
                        currentSnap.CopyFrom(snap);
                    }
                }
                if(currentSnapTime == 0) {
                    QMessageBox::critical(this, "Error", "No snapshot found based on the end time you provided! End time must based on one snapshot!");
                }else {
                    getSwcIncrementRecord(currentSnap);
                }
            }
        }else {
            QMessageBox::critical(this, "Error", QString::fromStdString(response.metainfo().message()));
        }
    }else {
        QMessageBox::critical(this, "Error", QString::fromStdString(status.error_message()));
    }
}

void EditorSwcVersionControl::getSwcIncrementRecord(proto::SwcSnapshotMetaInfoV1 snapshot) {
    grpc::ClientContext context;

    proto::GetAllIncrementOperationMetaInfoRequest request;
    proto::GetAllIncrementOperationMetaInfoResponse response;
    WrappedCall::setCommonRequestField(request);

    request.set_swcname(m_SwcName);

    proto::SwcIncrementOperationMetaInfoV1 currentIncrement;

    if(auto status = RpcCall::getInstance().Stub()->GetAllIncrementOperationMetaInfo(&context,request,&response); status.ok()) {
        if(response.metainfo().status()) {
            for(auto& increment : response.swcincrementoperationmetainfo()) {
                if(increment.startsnapshot() == snapshot.swcsnapshotcollectionname()) {
                    currentIncrement.CopyFrom(increment);
                    break;
                }
            }
            if(currentIncrement.startsnapshot().empty()) {
                QMessageBox::critical(this, "Error", "Cannot found the corresponding increment record!");
            }else {
                proto::GetSnapshotResponse snapResponse;
                if(WrappedCall::getSwcSnapshot(snapshot.swcsnapshotcollectionname(),snapResponse,this)) {
                    std::vector<proto::SwcNodeDataV1> nodeData;
                    for(auto& node : snapResponse.swcnodedata()) {
                        nodeData.push_back(node);
                    }

                    if(WrappedCall::getS(snapshot.swcsnapshotcollectionname(),snapResponse,this)) {

                    }
                }
            }
        }else {
            QMessageBox::critical(this, "Error", QString::fromStdString(response.metainfo().message()));
        }
    }else {
        QMessageBox::critical(this, "Error", QString::fromStdString(status.error_message()));
    }
}
