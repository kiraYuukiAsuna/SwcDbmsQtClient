//
// Created by KiraY on 2024/1/21.
//

// You may need to build the project (run Qt uic code generator) to get "ui_EditorSwcIncrementRecord.h" resolved

#include "editorswcincrementrecord.h"
#include "ui_EditorSwcIncrementRecord.h"


EditorSwcIncrementRecord::EditorSwcIncrementRecord(const std::string& swcName, QWidget *parent) :
    QDialog(parent), ui(new Ui::EditorSwcIncrementRecord) {
    ui->setupUi(this);
}

EditorSwcIncrementRecord::~EditorSwcIncrementRecord() {
    delete ui;
}
