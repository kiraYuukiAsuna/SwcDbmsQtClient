//
// Created by KiraY on 2023/11/23.
//

// You may need to build the project (run Qt uic code generator) to get "ui_EditorPermissionGroup.h" resolved

#include "editorpermissiongroup.h"
#include "ui_EditorPermissionGroup.h"


EditorPermissionGroup::EditorPermissionGroup(QWidget *parent) :
    QWidget(parent), ui(new Ui::EditorPermissionGroup) {
    ui->setupUi(this);
}

EditorPermissionGroup::~EditorPermissionGroup() {
    delete ui;
}
