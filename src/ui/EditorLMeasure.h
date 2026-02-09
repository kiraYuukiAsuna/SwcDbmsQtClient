#pragma once

#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QWidget>

#include "EditorBase.h"

class EditorLMeasure : public QWidget, public EditorBase {
	Q_OBJECT

public:
	// Constructor for server SWC data
	EditorLMeasure(const std::string& swcUuid, const std::string& swcName,
				   QWidget* parent);
	// Constructor for local file
	EditorLMeasure(const QString& filePath, QWidget* parent);
	~EditorLMeasure() override;

	std::string getUuid() override;
	MetaInfoType getMetaInfoType() override;
	bool save() override;

private:
	struct MetricInfo {
		int id;
		QString name;
		QCheckBox* checkbox;
	};

	void setupUi();
	void populateMetrics();
	void runAnalysis();
	bool writeSwcToTempFile(const QString& tempPath);

	std::string m_SwcUuid;
	QString m_FilePath;
	std::string m_Name;

	QTableWidget* m_ResultTable;
	QPushButton* m_RunButton;
	QPushButton* m_SelectAllButton;
	QPushButton* m_DeselectAllButton;
	QLabel* m_StatusLabel;

	std::vector<MetricInfo> m_Metrics;
};
