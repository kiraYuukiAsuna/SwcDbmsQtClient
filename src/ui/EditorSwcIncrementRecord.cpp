#include "EditorSwcIncrementRecord.h"

#include <Message/Request.pb.h>
#include <Message/Response.pb.h>
#include "ui_EditorSwcIncrementRecord.h"
#include "src/framework/service/WrappedCall.h"

EditorSwcIncrementRecord::EditorSwcIncrementRecord(const std::string&swcUuid, QWidget* parent) : QDialog(parent),
    ui(new Ui::EditorSwcIncrementRecord), m_SwcUuid(swcUuid) {
    ui->setupUi(this);

    connect(ui->DetailBtn,&QPushButton::clicked,this,[&]() {
        QMessageBox::information(this, "Info", "Using [Version Control] To Access History Swc Version at AnyTime!");
    });

    getAllSwcIncrementRecord();
    refreshTableView();
}

EditorSwcIncrementRecord::~EditorSwcIncrementRecord() {
    delete ui;
}

void EditorSwcIncrementRecord::getAllSwcIncrementRecord() {
    proto::GetAllIncrementOperationMetaInfoResponse response;

    if (WrappedCall::getAllSwcIncrementRecordByUuid(m_SwcUuid, response, this)) {
        for (auto&incrementRecordCollection: response.swcincrementoperationmetainfo()) {
            m_SwcIncrements.push_back(incrementRecordCollection);
        }
    }
}

void EditorSwcIncrementRecord::refreshTableView() {
    auto* model = new SwcIncrementRecordTableModel(m_SwcIncrements, this);
    ui->tableView->setModel(model);
    ui->tableView->resizeColumnsToContents();
}
