#include "viewincrementrecord.h"
#include "ui_ViewIncrementRecord.h"


ViewIncrementRecord::ViewIncrementRecord(QWidget *parent) :
    QDialog(parent), ui(new Ui::ViewIncrementRecord) {
    ui->setupUi(this);
}

ViewIncrementRecord::~ViewIncrementRecord() {
    delete ui;
}
