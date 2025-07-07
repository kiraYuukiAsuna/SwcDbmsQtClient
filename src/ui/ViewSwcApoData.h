#pragma once

#include <QDialog>
#include <QWidget>

#include "EditorBase.h"
#include "Message/Message.pb.h"
#include "Message/Response.pb.h"

QT_BEGIN_NAMESPACE
namespace Ui {
	class ViewSwcApoData;
}
QT_END_NAMESPACE

class ViewSwcApoData : public QDialog {
	Q_OBJECT

public:
	explicit ViewSwcApoData(bool modifyMode = false, QWidget *parent = nullptr);
	~ViewSwcApoData() override;

	void refresh(proto::GetSwcFullNodeDataResponse &response) {}

	void setSwcApoData(proto::SwcAttachmentApoV1 &swcApoData);

	proto::SwcAttachmentApoV1 getSwcApoData();

private:
	bool m_ModifyMode;
	proto::SwcAttachmentApoV1 m_SwcApoData;

	proto::SwcMetaInfoV1 m_SwcMetaInfo;
	std::string m_SwcName;

	Ui::ViewSwcApoData *ui;
};
