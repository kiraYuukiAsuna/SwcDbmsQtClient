#include "editoranoattachment.h"
#include "ui_EditorAnoAttachment.h"
#include "src/framework/service/WrappedCall.h"


EditorAnoAttachment::EditorAnoAttachment(const std::string&swcName, QWidget* parent) : QDialog(parent),
    ui(new Ui::EditorAnoAttachment), m_SwcName(swcName) {
    ui->setupUi(this);

    proto::GetSwcMetaInfoResponse response;
    if(WrappedCall::getSwcMetaInfoByName(m_SwcName, response, this)) {

    }

    response.swcinfo().
}

EditorAnoAttachment::~EditorAnoAttachment() {
    delete ui;
}
