#include "editorswcincrementrecord.h"

#include <Message/Request.pb.h>
#include <Message/Response.pb.h>
#include "ui_EditorSwcIncrementRecord.h"
#include "src/framework/service/WrappedCall.h"

EditorSwcIncrementRecord::EditorSwcIncrementRecord(const std::string& swcName, QWidget *parent) :
    QDialog(parent), ui(new Ui::EditorSwcIncrementRecord),m_SwcName(swcName) {
    ui->setupUi(this);

    getAllSnapshot();
    refreshTableView();
}

EditorSwcIncrementRecord::~EditorSwcIncrementRecord() {
    delete ui;
}

void EditorSwcIncrementRecord::getAllSnapshot() {
    grpc::ClientContext context;
    proto::GetAllIncrementOperationMetaInfoRequest request;
    proto::GetAllIncrementOperationMetaInfoResponse response;
    WrappedCall::setCommonRequestField(request);
    request.set_swcname(m_SwcName);

    auto status = RpcCall::getInstance().Stub()->GetAllIncrementOperationMetaInfo(&context,request,&response);
    if(status.ok()) {
        if(response.has_metainfo()&&response.metainfo().status() == true) {
            for (auto& incrementRecordCollection : response.swcincrementoperationmetainfo()) {
                m_SwcIncrements.push_back(incrementRecordCollection);
            }
        }else {
            QMessageBox::critical(this,"Error",QString::fromStdString(response.metainfo().message()));
        }
    }else {
        QMessageBox::critical(this,"Error",QString::fromStdString(status.error_message()));
    }
}

void EditorSwcIncrementRecord::refreshTableView() {
    auto *model = new SwcIncrementRecordTableModel(m_SwcIncrements, this);
    ui->tableView->setModel(model);
    ui->tableView->resizeColumnsToContents();
}
