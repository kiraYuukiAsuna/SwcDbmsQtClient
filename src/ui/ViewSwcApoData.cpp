#include "ViewSwcApoData.h"

#include "Message/Response.pb.h"
#include "src/framework/service/WrappedCall.h"
#include "ui_ViewSwcApoData.h"

ViewSwcApoData::ViewSwcApoData(bool modifyMode, QWidget* parent)
	: QDialog(parent), ui(new Ui::ViewSwcApoData) {
	ui->setupUi(this);

	m_ModifyMode = modifyMode;

	connect(ui->CancelBtn, &QPushButton::clicked, this, [this]() { reject(); });

	connect(ui->OKBtn, &QPushButton::clicked, this, [this]() {
		m_SwcApoData.set_n(ui->N->value());
		m_SwcApoData.set_orderinfo(ui->orderinfo->text().toStdString());
		m_SwcApoData.set_name(ui->name->text().toStdString());
		m_SwcApoData.set_comment(ui->comment->text().toStdString());
		m_SwcApoData.set_z(ui->Z->value());
		m_SwcApoData.set_x(ui->X->value());
		m_SwcApoData.set_y(ui->Y->value());
		m_SwcApoData.set_pixmax(ui->pixmax->value());
		m_SwcApoData.set_intensity(ui->intensity->value());
		m_SwcApoData.set_volsize(ui->volsize->value());
		m_SwcApoData.set_mass(ui->mass->value());
		m_SwcApoData.set_colorr(ui->color_r->value());
		m_SwcApoData.set_colorg(ui->color_g->value());
		m_SwcApoData.set_colorb(ui->color_b->value());

		accept();
	});
}

ViewSwcApoData::~ViewSwcApoData() { delete ui; }

void ViewSwcApoData::setSwcApoData(proto::SwcAttachmentApoV1& swcApoData) {
	m_SwcApoData = swcApoData;

	ui->N->setValue(m_SwcApoData.n());
	ui->orderinfo->setText(QString::fromStdString(m_SwcApoData.orderinfo()));
	ui->name->setText(QString::fromStdString(m_SwcApoData.name()));
	ui->comment->setText(QString::fromStdString(m_SwcApoData.comment()));
	ui->Z->setValue(m_SwcApoData.z());
	ui->X->setValue(m_SwcApoData.x());
	ui->Y->setValue(m_SwcApoData.y());
	ui->pixmax->setValue(m_SwcApoData.pixmax());
	ui->intensity->setValue(m_SwcApoData.intensity());
	ui->volsize->setValue(m_SwcApoData.volsize());
	ui->mass->setValue(m_SwcApoData.mass());
	ui->color_r->setValue(m_SwcApoData.colorr());
	ui->color_g->setValue(m_SwcApoData.colorg());
	ui->color_b->setValue(m_SwcApoData.colorb());
}

proto::SwcAttachmentApoV1 ViewSwcApoData::getSwcApoData() {
	return m_SwcApoData;
}
