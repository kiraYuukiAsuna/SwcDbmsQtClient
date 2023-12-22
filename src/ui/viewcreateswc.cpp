#include "viewcreateswc.h"

#include <QMessageBox>
#include <Message/Request.pb.h>
#include <Message/Response.pb.h>
#include "src/framework/service/WrappedCall.h"
#include "ui_ViewCreateSwc.h"

ViewCreateSwc::ViewCreateSwc(QWidget* parent) : QDialog(parent), ui(new Ui::ViewCreateSwc) {
    ui->setupUi(this);

    connect(ui->CancelBtn, &QPushButton::clicked, this, [this]() {
        reject();
    });

    connect(ui->OKBtn, &QPushButton::clicked, this, [this]() {
        proto::CreateSwcResponse response;

        if (ui->Name->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Error", "Name cannot be empty!");
            return false;
        }
        if (ui->Description->text().isEmpty()) {
            QMessageBox::warning(this, "Error", "Description cannot be empty!");
            return false;
        }

        if(WrappedCall::createSwcMeta(ui->Name->text().trimmed().toStdString(),ui->Description->text().toStdString(),response,this)){
            accept();
        }

    });
}

ViewCreateSwc::~ViewCreateSwc() {
    delete ui;
}
