#pragma once

#include <QTabWidget>
#include <QVBoxLayout>
#include <QWidget>

#include "EditorBase.h"
#include "src/framework/defination/TypeDef.h"

class MainWindow;

class RightClientView : public QWidget {
	Q_OBJECT

public:
	explicit RightClientView(MainWindow* mainWindow);
	~RightClientView() override;

	void openProjectMetaInfo(const std::string& projectUuid);
	void closeWithoutSavingProject(const std::string& projectUuid);
	void refreshProjectMetaInfo(const std::string& projectUuid);
	void openSwcMetaInfo(const std::string& swcUuid);
	void closeWithoutSavingSwc(const std::string& swcUuid);
	void refreshSwcMetaInfo(const std::string& swcUuid);
	void openDailyStatisticsMetaInfo(const std::string& dailyStatisticsName);
	void closeWithoutSavingDailyStatistics(
		const std::string& dailyStatisticsName);
	void refreshDailyStatisticsMetaInfo(const std::string& dailyStatisticsName);

	void openSwcNodeData(const std::string& swcUuid);
	void closeWithoutSavingSwcNodeData(const std::string& swcUuid);

	void refreshAllOpenedProjectMetaInfo();

private:
	int findIfTabAlreadyOpened(const std::string& name,
							   MetaInfoType metaInfoType);
	void closeTab(const std::string& identifier, MetaInfoType type);
	EditorBase* findEditorBase(const std::string& identifier,
							   MetaInfoType type);

	QVBoxLayout* m_MainLayout;
	MainWindow* m_MainWindow;

	QTabWidget* m_TabWidget;
};
