#include "EditorBatchManageSwc.h"
#include "ui_EditorBatchManageSwc.h"


EditorBatchManageSwc::EditorBatchManageSwc(QWidget *parent) :
    QDialog(parent), ui(new Ui::EditorBatchManageSwc) {
    ui->setupUi(this);
}

EditorBatchManageSwc::~EditorBatchManageSwc() {
    delete ui;
}
