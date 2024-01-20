#include "createswcsnapshotview.h"

#include <QMessageBox>
#include <grpcpp/client_context.h>
#include <Message/Request.pb.h>
#include <Message/Response.pb.h>

#include "ui_CreateSwcSnapshotView.h"
#include "src/framework/service/WrappedCall.h"

CreateSwcSnapshotView::CreateSwcSnapshotView(const std::string&swcName, QWidget* parent) : QDialog(parent),
                                                                                           ui(new Ui::CreateSwcSnapshotView), m_SwcName(swcName) {
    ui->setupUi(this);

    ui->SwcName->setText(QString::fromStdString(m_SwcName));

    connect(ui->OKBtn, &QPushButton::clicked, this, [&]() {
        auto result = QMessageBox::information(this, "Warning",
                                               "Are you sure to create a snapshot for this swc?",
                                               QMessageBox::StandardButton::Ok, QMessageBox::StandardButton::Cancel);
        if (result == QMessageBox::StandardButton::Ok) {
            grpc::ClientContext context;
            proto::CreateSwcSnapshotRequest request;
            proto::CreateSwcSnapshotResponse response;
            WrappedCall::setCommonRequestField(request);
            request.set_swcname(m_SwcName);

            auto status = RpcCall::getInstance().Stub()->CreateSwcSnapshot(&context,request,&response);
            if(status.ok()) {
                if(response.has_metainfo()&&response.metainfo().status() == true) {
                    QMessageBox::information(this, "Warning","Create Snapshot successfully!",
                    QMessageBox::StandardButton::Ok, QMessageBox::StandardButton::Cancel);
                }else {
                    QMessageBox::critical(this,"Error",QString::fromStdString(response.metainfo().message()));
                }
            }else {
                QMessageBox::critical(this,"Error",QString::fromStdString(status.error_message()));
            }
        }
    });

    connect(ui->CancelBtn, &QPushButton::clicked, this, [&]() {
        reject();
    });
}

CreateSwcSnapshotView::~CreateSwcSnapshotView() {
    delete ui;
}
