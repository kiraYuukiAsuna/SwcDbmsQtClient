#include "EditorSwcIncrementRecord.h"

#include <Message/Request.pb.h>
#include <Message/Response.pb.h>
#include "ui_EditorSwcIncrementRecord.h"
#include "src/framework/service/WrappedCall.h"

EditorSwcIncrementRecord::EditorSwcIncrementRecord(const std::string&swcName, QWidget* parent) : QDialog(parent),
    ui(new Ui::EditorSwcIncrementRecord), m_SwcName(swcName) {
    ui->setupUi(this);

    connect(ui->DetailBtn,&QPushButton::clicked,this,[&]() {
        QMessageBox::information(this, "Info", "Using [Version Control] To Access History Swc Version at AnyTime!");
    });

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
