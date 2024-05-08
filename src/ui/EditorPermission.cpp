#include "EditorPermission.h"
#include "ui_EditorPermission.h"


EditorPermission::EditorPermission(QWidget *parent) :
    QDialog(parent), ui(new Ui::EditorPermission) {
    ui->setupUi(this);
}

EditorPermission::~EditorPermission() {
    delete ui;
}
