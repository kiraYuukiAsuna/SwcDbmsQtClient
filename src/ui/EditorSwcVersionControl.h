#pragma once

#include <Message/Request.pb.h>
#include <Message/Response.pb.h>
#include <google/protobuf/timestamp.pb.h>

#include <QCloseEvent>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QDialog>
#include <QFormLayout>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QProgressBar>
#include <QPushButton>
#include <QSpinBox>
#include <QSplitter>
#include <QStatusBar>
#include <QTextEdit>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>
#include <QtNodes/DataFlowGraphicsScene>
#include <QtNodes/GraphicsView>

// Forward declarations
class ViewExportSwcToFile;
struct ExportSwcData;

class EditorSwcVersionControl : public QDialog {
	Q_OBJECT

public:
	enum class StatusType { Ready, Working, Success, Warning, Error };

	explicit EditorSwcVersionControl(const std::string &swcUuid,
									 QWidget *parent = nullptr);

	~EditorSwcVersionControl() override;

	void refreshVersionGraph();

private slots:
	// 时间选择相关槽函数
	void onTimeSelectionChanged();
	void onExportCurrentVersion();
	void onRevertVersion();

	// 版本图操作相关槽函数
	void onRefreshVersionGraph();
	void onExportSelectedVersion();
	void onRevertToSnapshot();
	void onVisualizeVersion();
	void onVisualizeDiffVersion();

	// 增量操作时间点相关槽函数
	void onTimePointSelected();
	void onRevertToTimePoint();
	void onExportTimePoint();
	void onVisualizeTimePoint();

private:
	// UI组件
	void setupUI();
	void setupControlPanel();
	void setupVersionGraphArea();
	void setupStatusBar();
	void setupConnections();

	// 控制面板组件
	QGroupBox *m_ControlPanel;
	QLabel *m_EndTimeLabel;
	QDateTimeEdit *m_EndTimeEdit;

	// 操作按钮组
	QGroupBox *m_ActionGroup;
	QPushButton *m_ExportBtn;
	QPushButton *m_RevertBtn;
	QPushButton *m_RefreshBtn;
	QPushButton *m_VisualizeBtn;
	QPushButton *m_VisualizeDiffBtn;
	QPushButton *m_ExportSelectedBtn;
	QPushButton *m_RevertToSnapshotBtn;

	// 版本图区域
	QSplitter *m_MainSplitter;
	QWidget *m_GraphContainer;
	QVBoxLayout *m_GraphLayout;

	// 信息面板
	QGroupBox *m_InfoPanel;
	QTextEdit *m_VersionInfoText;
	QLabel *m_SelectedVersionLabel;
	QLabel *m_TotalVersionsLabel;

	// 增量操作时间点面板
	QGroupBox *m_TimePointPanel;
	QListWidget *m_TimePointList;
	QPushButton *m_RevertToTimePointBtn;
	QPushButton *m_ExportTimePointBtn;
	QPushButton *m_VisualizeTimePointBtn;
	QLabel *m_TimePointStatusLabel;

	// 状态栏
	QStatusBar *m_StatusBar;
	QProgressBar *m_ProgressBar;
	QLabel *m_StatusLabel;

	QTimer m_StatusTimer;

private:
	std::string m_SwcUuid;

	bool m_HasExportedStatus{false};
	int64_t m_EndTime;
	std::string m_SnapshotName;
	std::string m_IncremrntOpName;

	std::vector<proto::SwcSnapshotMetaInfoV1> m_SwcSnapshots;
	std::vector<proto::SwcIncrementOperationMetaInfoV1> m_SwcIncrements;

	// 当前选中的增量操作和时间点
	std::string m_CurrentIncrementOpName;
	proto::SwcIncrementOperationMetaInfoV1 m_CurrentSelectedIncrement;
	std::vector<proto::SwcIncrementOperationV1> m_CurrentTimePoints;

	// 核心功能函数
	void getSwcLastSnapshot();
	void getSwcIncrementRecord(proto::SwcSnapshotMetaInfoV1 snapshot,
							   int64_t endTime);
	void promoteOperation(std::vector<proto::SwcNodeDataV1> &nodeData,
						  const proto::SwcIncrementOperationV1 &op,
						  int64_t endTime);
	void getAllSnapshot();
	void getAllSwcIncrementRecord();

	// 辅助功能函数
	void updateVersionInfo();
	void updateStatusBar(const QString &message);
	void updateStatusBar(const QString &message, StatusType type);
	void setProgressBar(int value, int maximum = 100);
	QString formatDateTime(int64_t timestamp);

	// 增量操作时间点相关函数
	void setupTimePointPanel();
	void loadIncrementTimePoints(const std::string &incrementOpName);
	void updateTimePointDisplay();
	void updateIncrementOperationInfo(
		const proto::SwcIncrementOperationMetaInfoV1 &increment,
		bool isLoading = false);
	void exportTimePointData(int64_t timePoint);

	// 时间比较辅助函数
	bool isEarlier(const google::protobuf::Timestamp &time1,
				   const google::protobuf::Timestamp &time2) const;

	QtNodes::DataFlowGraphModel m_DataFlowGraphModel;
	QtNodes::GraphicsView *m_GraphicsView;
	QtNodes::DataFlowGraphicsScene *m_DataFlowGraphicsScene;
};
