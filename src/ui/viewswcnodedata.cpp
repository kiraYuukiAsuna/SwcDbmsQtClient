#include "viewswcnodedata.h"
#include "ui_ViewSwcNodeData.h"
#include "Message/Response.pb.h"
#include "src/framework/service/WrappedCall.h"

ViewSwcNodeData::ViewSwcNodeData(bool modifyMode, QWidget *parent) :
        QDialog(parent), ui(new Ui::ViewSwcNodeData) {
    ui->setupUi(this);

    m_ModifyMode = modifyMode;

    connect(ui->CancelBtn,&QPushButton::clicked,this,[this](){
        reject();
    });

    connect(ui->OKBtn,&QPushButton::clicked,this,[this](){
        m_SwcNodeInternalData.set_n(ui->N->value());
        m_SwcNodeInternalData.set_type(ui->Type->value());
        m_SwcNodeInternalData.set_x(ui->X->value());
        m_SwcNodeInternalData.set_y(ui->Y->value());
        m_SwcNodeInternalData.set_z(ui->Z->value());
        m_SwcNodeInternalData.set_radius(ui->Radius->value());
        m_SwcNodeInternalData.set_parent(ui->Parent->value());
        m_SwcNodeInternalData.set_seg_id(ui->Seg_id->value());
        m_SwcNodeInternalData.set_level(ui->Level->value());
        m_SwcNodeInternalData.set_mode(ui->Mode->value());
        m_SwcNodeInternalData.set_timestamp(ui->Timestamp->value());
        m_SwcNodeInternalData.set_feature_value(ui->Feature_value->value());

        accept();
    });
}

ViewSwcNodeData::~ViewSwcNodeData() {
    delete ui;
}

void ViewSwcNodeData::setSwcNodeInternalData(proto::SwcNodeInternalDataV1 &swcNodeInternalData) {
    m_SwcNodeInternalData = swcNodeInternalData;

    ui->N->setValue(swcNodeInternalData.n());
    ui->Type->setValue(swcNodeInternalData.type());
    ui->X->setValue(swcNodeInternalData.x());
    ui->Y->setValue(swcNodeInternalData.y());
    ui->Z->setValue(swcNodeInternalData.z());
    ui->Radius->setValue(swcNodeInternalData.radius());
    ui->Parent->setValue(swcNodeInternalData.parent());
    ui->Seg_id->setValue(swcNodeInternalData.seg_id());
    ui->Level->setValue(swcNodeInternalData.level());
    ui->Mode->setValue(swcNodeInternalData.mode());
    ui->Timestamp->setValue(swcNodeInternalData.timestamp());
    ui->Feature_value->setValue(swcNodeInternalData.feature_value());
}

proto::SwcNodeInternalDataV1 ViewSwcNodeData::getSwcNodeInternalData(){
    return m_SwcNodeInternalData;
}
