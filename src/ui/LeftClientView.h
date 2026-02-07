#pragma once

#include <QHBoxLayout>
#include <QMenu>
#include <QPushButton>
#include <QToolButton>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QWidget>

#include "src/framework/defination/TypeDef.h"

class MainWindow;

class LeftClientView : public QWidget {
	Q_OBJECT

public:
	explicit LeftClientView(MainWindow* mainWindow);
	~LeftClientView() override;

	void clearAll();
	void getProjectMetaInfo();
	void getFreeSwcMetaInfo();
	void getAllDailyStatisticsMetaInfo();
	void refreshTree();

public slots:
	void onRefreshBtnClicked(bool checked);
	void customTreeWidgetContentMenu(const QPoint& pos);

private:
	void updateHeadPhoto();
	void buildProjectContainerMenu(
		QMenu* menu, const LeftClientViewTreeWidgetItemMetaInfo& data);
	void buildProjectMenu(QMenu* menu,
						  const LeftClientViewTreeWidgetItemMetaInfo& data,
						  QTreeWidgetItem* curItem);
	void buildFreeSwcContainerMenu(
		QMenu* menu, const LeftClientViewTreeWidgetItemMetaInfo& data);
	void buildSwcItemMenu(QMenu* menu,
						  const LeftClientViewTreeWidgetItemMetaInfo& data);
	void buildDailyStatisticsMenu(
		QMenu* menu, const LeftClientViewTreeWidgetItemMetaInfo& data,
		QTreeWidgetItem* curItem);

	QVBoxLayout* m_MainLayout;
	QHBoxLayout* m_ControlBtnLayout;
	MainWindow* m_MainWindow;

	QPushButton* m_UserSettingBtn;
	QToolButton* m_AccountBtn;
	QPushButton* m_RefreshBtn;

	QTreeWidget* m_TreeWidget;
	QTreeWidgetItem* m_TopProjectItem;
	QTreeWidgetItem* m_TopSwcItem;
	QTreeWidgetItem* m_TopDailyStatisticsItem;
};
