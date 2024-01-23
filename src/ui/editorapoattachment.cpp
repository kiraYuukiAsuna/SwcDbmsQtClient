#include "editorapoattachment.h"
#include "ui_EditorApoAttachment.h"


EditorApoAttachment::EditorApoAttachment(const std::string& swcName,QWidget *parent) :
    QDialog(parent), ui(new Ui::EditorApoAttachment) ,m_SwcName(swcName){
    ui->setupUi(this);
}

EditorApoAttachment::~EditorApoAttachment() {
    delete ui;
}
