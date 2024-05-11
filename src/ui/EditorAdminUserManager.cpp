#include "EditorAdminUserManager.h"
#include "ui_EditorAdminUserManager.h"


EditorAdminUserManager::EditorAdminUserManager(QWidget* parent) : QWidget(parent), ui(new Ui::EditorAdminUserManager) {
    ui->setupUi(this);
}

EditorAdminUserManager::~EditorAdminUserManager() {
    delete ui;
}
