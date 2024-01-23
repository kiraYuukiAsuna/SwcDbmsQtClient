#include "editorswcincrementrecord.h"

#include <Message/Request.pb.h>
#include <Message/Response.pb.h>
#include "ui_EditorSwcIncrementRecord.h"
#include "src/framework/service/WrappedCall.h"

EditorSwcIncrementRecord::EditorSwcIncrementRecord(const std::string&swcName, QWidget* parent) : QDialog(parent),
    ui(new Ui::EditorSwcIncrementRecord), m_SwcName(swcName) {
    ui->setupUi(this);

    getAllSnapshot();
    refreshTableView();
}

EditorSwcIncrementRecord::~EditorSwcIncrementRecord() {
    delete ui;
}

void EditorSwcIncrementRecord::getAllSnapshot() {
    proto::GetAllIncrementOperationMetaInfoResponse response;

    if (WrappedCall::getAllSwcIncrementRecord(m_SwcName, response, this)) {
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
