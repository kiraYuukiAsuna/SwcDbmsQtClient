#pragma once

#include <QDialog>
#include <QWidget>

#include "EditorBase.h"
#include "Message/Message.pb.h"
#include "Message/Response.pb.h"

QT_BEGIN_NAMESPACE
namespace Ui {
	class ViewSwcNodeData;
}
QT_END_NAMESPACE

class ViewSwcNodeData : public QDialog {
	Q_OBJECT

public:
	explicit ViewSwcNodeData(bool modifyMode = false,
							 QWidget* parent = nullptr);
	~ViewSwcNodeData() override;

	void refresh(proto::GetSwcFullNodeDataResponse& response) {}

	void setSwcNodeInternalData(
		proto::SwcNodeInternalDataV1& swcNodeInternalData);

	proto::SwcNodeInternalDataV1 getSwcNodeInternalData();

private:
	bool m_ModifyMode;
	proto::SwcNodeInternalDataV1 m_SwcNodeInternalData;

	proto::SwcMetaInfoV1 m_SwcMetaInfo;
	std::string m_SwcName;

	Ui::ViewSwcNodeData* ui;
};
