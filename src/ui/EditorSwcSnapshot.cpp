#include "EditorSwcSnapshot.h"
#include <Message/Request.pb.h>
#include "ui_EditorSwcSnapshot.h"
#include "ViewExportSwcToFile.h"
#include "src/framework/service/WrappedCall.h"

EditorSwcSnapshot::EditorSwcSnapshot(const std::string& swcUuid, QWidget *parent) :
    QDialog(parent), ui(new Ui::EditorSwcSnapshot),m_SwcUuid(swcUuid) {
    ui->setupUi(this);


    connect(ui->Export,&QPushButton::clicked,this,[&]() {
        QModelIndex currentIndex = ui->tableView->selectionModel()->currentIndex();
        int currentRow = currentIndex.row();
        if(currentRow<0){
            QMessageBox::information(this,"Info","You need to select one row first!");
            return;
        }

        proto::GetSwcMetaInfoResponse response;
        if(!WrappedCall::getSwcMetaInfoByUuid(m_SwcUuid,response,this)) {
            return;
        }

        std::vector<ExportSwcData> exportSwcData;
        exportSwcData.push_back({response.swcinfo(),{},true,m_SwcSnapshots.at(currentRow).swcsnapshotcollectionname()});
        ViewExportSwcToFile view(exportSwcData,true,nullptr);
        view.exec();
    });

    getAllSnapshot();
    refreshTableView();
}

EditorSwcSnapshot::~EditorSwcSnapshot() {
    delete ui;
}

void EditorSwcSnapshot::getAllSnapshot() {
    grpc::ClientContext context;
    proto::GetAllSnapshotMetaInfoRequest request;
    proto::GetAllSnapshotMetaInfoResponse response;
    WrappedCall::setCommonRequestField(request);
    request.set_swcuuid(m_SwcUuid);

    auto status = RpcCall::getInstance().Stub()->GetAllSnapshotMetaInfo(&context,request,&response);
    if(status.ok()) {
        if(response.has_metainfo()&&response.metainfo().status() == true) {
            for (auto& snapshot : response.swcsnapshotlist()) {
                m_SwcSnapshots.push_back(snapshot);
            }
        }else {
            QMessageBox::critical(this,"Error",QString::fromStdString(response.metainfo().message()));
        }
    }else {
        QMessageBox::critical(this,"Error",QString::fromStdString(status.error_message()));
    }
}

void EditorSwcSnapshot::refreshTableView() {
    auto *model = new SwcSnapshotTableModel(m_SwcSnapshots, this);
    ui->tableView->setModel(model);
    ui->tableView->resizeColumnsToContents();
}
