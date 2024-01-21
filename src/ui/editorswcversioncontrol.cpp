#include "editorswcversioncontrol.h"
#include "ui_EditorSwcVersionControl.h"


EditorSwcVersionControl::EditorSwcVersionControl(const std::string& swcName, QWidget *parent) :
    QDialog(parent), ui(new Ui::EditorSwcVersionControl) {
    ui->setupUi(this);
}

EditorSwcVersionControl::~EditorSwcVersionControl() {
    delete ui;
}
