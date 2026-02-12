#pragma once

#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QWidget>

#include "EditorBase.h"

class EditorSwcFeature : public QWidget, public EditorBase {
	Q_OBJECT

public:
	EditorSwcFeature(const std::string& swcUuid, const std::string& swcName,
					 QWidget* parent);
	EditorSwcFeature(const QString& filePath, QWidget* parent);
	~EditorSwcFeature() override;

	std::string getUuid() override;
	MetaInfoType getMetaInfoType() override;
	bool save() override;

private:
	void setupUi();
	void runAnalysis();
	bool writeSwcToTempFile(const QString& tempPath);

	std::string m_SwcUuid;
	QString m_FilePath;
	std::string m_Name;

	QTableWidget* m_ResultTable;
	QPushButton* m_RunButton;
	QLabel* m_StatusLabel;

	QCheckBox* m_ChkSort;
	QCheckBox* m_ChkPrune;
	QDoubleSpinBox* m_SpinPruneThreshold;
	QCheckBox* m_ChkResample;
	QDoubleSpinBox* m_SpinResampleStep;
};
