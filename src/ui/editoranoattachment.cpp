#include "editoranoattachment.h"
#include "ui_EditorAnoAttachment.h"


EditorAnoAttachment::EditorAnoAttachment(const std::string& swcName,QWidget *parent) :
    QDialog(parent), ui(new Ui::EditorAnoAttachment),m_SwcName(swcName) {
    ui->setupUi(this);
}

EditorAnoAttachment::~EditorAnoAttachment() {
    delete ui;
}
