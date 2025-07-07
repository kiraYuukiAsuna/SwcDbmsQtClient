#pragma once

#include <QDialog>
#include <QWidget>

#include "EditorBase.h"
#include "Message/Message.pb.h"
#include "TreeWidget/TreeWidget.h"

QT_BEGIN_NAMESPACE

namespace Ui {
	class EditorPermissionGroup;
}

QT_END_NAMESPACE

class EditorPermissionGroup : public QDialog, public EditorBase {
	Q_OBJECT

public:
	explicit EditorPermissionGroup(QWidget* parent = nullptr);

	~EditorPermissionGroup() override;

	std::string getName() override { return ""; }

	std::string getUuid() override { return ""; }

	MetaInfoType getMetaInfoType() override {
		return MetaInfoType::ePermissionGroupMetaInfo;
	}

	bool save() override { return true; }

	void refresh();

private:
	Ui::EditorPermissionGroup* ui;

	TreeWidget* m_TreeWidget;

	google::protobuf::RepeatedPtrField<proto::PermissionGroupMetaInfoV1>
		m_PermissionGroupList;
};
