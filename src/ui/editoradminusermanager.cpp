//
// Created by KiraY on 2023/11/22.
//

// You may need to build the project (run Qt uic code generator) to get "ui_EditorAdminUserManager.h" resolved

#include "editoradminusermanager.h"
#include "ui_EditorAdminUserManager.h"


EditorAdminUserManager::EditorAdminUserManager(QWidget *parent) :
    QWidget(parent), ui(new Ui::EditorAdminUserManager) {
    ui->setupUi(this);
}

EditorAdminUserManager::~EditorAdminUserManager() {
    delete ui;
}
