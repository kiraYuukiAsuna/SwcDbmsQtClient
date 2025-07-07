#pragma once

#include <QDialog>
#include <QWidget>

#include "EditorBase.h"

QT_BEGIN_NAMESPACE
namespace Ui {
	class EditorUserSettings;
}
QT_END_NAMESPACE

class LeftClientView;

class EditorUserSettings : public QDialog, public EditorBase {
	Q_OBJECT

public:
	explicit EditorUserSettings(LeftClientView *leftClientView);
	~EditorUserSettings() override;

	std::string getName() override { return "EditorUserSettings"; }

	std::string getUuid() override { return ""; }

	MetaInfoType getMetaInfoType() override {
		return MetaInfoType::eUserMetaInfo;
	}

	bool save() override { return false; }

	void getUserMetaInfo();

private:
	Ui::EditorUserSettings *ui;
	LeftClientView *m_LeftClientView;
};
