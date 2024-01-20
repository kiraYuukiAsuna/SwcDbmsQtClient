//
// Created by KiraY on 2024/1/21.
//

// You may need to build the project (run Qt uic code generator) to get "ui_EditorSwcVersionControl.h" resolved

#include "editorswcversioncontrol.h"
#include "ui_EditorSwcVersionControl.h"


EditorSwcVersionControl::EditorSwcVersionControl(const std::string& swcName, QWidget *parent) :
    QDialog(parent), ui(new Ui::EditorSwcVersionControl) {
    ui->setupUi(this);
}

EditorSwcVersionControl::~EditorSwcVersionControl() {
    delete ui;
}
