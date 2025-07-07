#include "EditorSwcMetaInfo.h"

#include <Message/Request.pb.h>
#include <Message/Response.pb.h>
#include <grpcpp/client_context.h>

#include <QIcon>

#include "src/framework/defination/ImageDefination.h"
#include "src/framework/service/RpcCall.h"
#include "src/framework/service/WrappedCall.h"
#include "ui_EditorSwcMetaInfo.h"

EditorSwcMetaInfo::EditorSwcMetaInfo(proto::GetSwcMetaInfoResponse& response,
									 QWidget* parent)
	: QWidget(parent), ui(new Ui::EditorSwcMetaInfo) {
	ui->setupUi(this);
	setWindowIcon(QIcon(Image::ImageNode));

	refresh(response);
}

EditorSwcMetaInfo::~EditorSwcMetaInfo() { delete ui; }

bool EditorSwcMetaInfo::save() {
	proto::UpdateSwcResponse response;

	if (ui->Description->text().isEmpty()) {
		QMessageBox::warning(this, "Error", "Description cannot be empty!");
		return false;
	}

	m_SwcMetaInfo.set_description(ui->Description->text().toStdString());

	if (!WrappedCall::UpdateSwcMetaInfo(m_SwcMetaInfo, response, this)) {
		return false;
	}

	return true;
}

void EditorSwcMetaInfo::refresh(proto::GetSwcMetaInfoResponse& response) {
	m_SwcMetaInfo.CopyFrom(response.swcinfo());

	ui->Id->setText(QString::fromStdString(m_SwcMetaInfo.base()._id()));
	ui->Id->setReadOnly(true);
	ui->Uuid->setText(QString::fromStdString(m_SwcMetaInfo.base().uuid()));
	ui->Uuid->setReadOnly(true);
	ui->DataAccessModelVersion->setText(
		QString::fromStdString(m_SwcMetaInfo.base().dataaccessmodelversion()));
	ui->DataAccessModelVersion->setReadOnly(true);
	ui->Name->setText(QString::fromStdString(m_SwcMetaInfo.name()));
	ui->Name->setReadOnly(true);
	ui->Description->setText(
		QString::fromStdString(m_SwcMetaInfo.description()));
	ui->Creator->setText(QString::fromStdString(m_SwcMetaInfo.creator()));
	ui->Creator->setReadOnly(true);
	ui->CreateTime->setDateTime(
		QDateTime::fromSecsSinceEpoch(m_SwcMetaInfo.createtime().seconds()));
	ui->CreateTime->setReadOnly(true);
	ui->LastModifiedTime->setDateTime(QDateTime::fromSecsSinceEpoch(
		m_SwcMetaInfo.lastmodifiedtime().seconds()));
	ui->LastModifiedTime->setReadOnly(true);

	// std::cout<<m_SwcMetaInfo.DebugString()<<std::endl;
}
