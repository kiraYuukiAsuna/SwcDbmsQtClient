#pragma once

#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QWidget>

#include "EditorBase.h"
#include "Renderer/SwcRenderer.h"

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
	void onVisualize();
	bool writeSwcToTempFile(const QString& tempPath);

	std::string m_SwcUuid;
	QString m_FilePath;
	std::string m_Name;

	QTableWidget* m_ResultTable;
	QPushButton* m_RunButton;
	QPushButton* m_VisualizeButton;
	QLabel* m_StatusLabel;

	std::vector<SwcMarker> m_Markers;

	QCheckBox* m_ChkLoop;
	QCheckBox* m_ChkTip;
	QCheckBox* m_ChkBranching;
	QCheckBox* m_ChkCrossing;
	QCheckBox* m_ChkSpecStructs;
};
