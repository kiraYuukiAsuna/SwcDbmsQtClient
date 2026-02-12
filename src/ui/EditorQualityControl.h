#pragma once

#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QWidget>

#include "EditorBase.h"

class EditorQualityControl : public QWidget, public EditorBase {
	Q_OBJECT

public:
	EditorQualityControl(const std::string& swcUuid,
						 const std::string& swcName, QWidget* parent);
	EditorQualityControl(const QString& filePath, QWidget* parent);
	~EditorQualityControl() override;

	std::string getUuid() override;
	MetaInfoType getMetaInfoType() override;
	bool save() override;

private:
	void setupUi();
	void runAnalysis();

	std::string m_SwcUuid;
	QString m_FilePath;
	std::string m_Name;

	QTableWidget* m_ResultTable;
	QPushButton* m_RunButton;
	QLabel* m_StatusLabel;

	QCheckBox* m_ChkLoop;
	QCheckBox* m_ChkTip;
	QCheckBox* m_ChkBranching;
	QCheckBox* m_ChkCrossing;
	QCheckBox* m_ChkSpecStructs;
};
