#pragma once

#include <QWidget>
#include <QTabWidget>
#include <QVBoxLayout>

#include "src/framework/defination/TypeDef.h"

class MainWindow;

QT_BEGIN_NAMESPACE
namespace Ui { class RightClientView; }
QT_END_NAMESPACE

class RightClientView : public QWidget {
Q_OBJECT

public:
    explicit RightClientView(MainWindow *mainWindow);
    ~RightClientView() override;

    void openProjectMetaInfo(const std::string& projectName);
    void closeWithoutSavingProject(const std::string& projectName);
    void refreshProjectMetaInfo(const std::string& projectName);
    void openSwcMetaInfo(const std::string& swcName);
    void closeWithoutSavingSwc(const std::string& swcName);
    void refreshSwcMetaInfo(const std::string& swcName);
    void openDailyStatisticsMetaInfo(const std::string& dailyStatisticsName);
    void closeWithoutSavingDailyStatistics(const std::string& dailyStatisticsName);
    void refreshDailyStatisticsMetaInfo(const std::string& dailyStatisticsName);

    void openSwcNodeData(const std::string& swcName);
    void closeWithoutSavingSwcNodeData(const std::string& swcName);

    void refreshAllOpenedProjectMetaInfo();

private:
    int findIfTabAlreadOpenned(const std::string& name, MetaInfoType metaInfoType);

    Ui::RightClientView *ui;
    QVBoxLayout* m_MainLayout;
    MainWindow *m_MainWindow;

    QTabWidget* m_TabWidget;
};
