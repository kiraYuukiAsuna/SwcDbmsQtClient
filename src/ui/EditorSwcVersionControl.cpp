#include "EditorSwcVersionControl.h"

#include <Message/Request.pb.h>
#include <Message/Response.pb.h>

#include <QApplication>
#include <QDateTime>
#include <QMessageBox>
#include <QScrollArea>
#include <QSizePolicy>
#include <QStyle>
#include <QTimer>

#include "Renderer/SwcRenderer.h"
#include "ViewExportSwcToFile.h"
#include "src/GraphModel/Registry.hpp"
#include "src/framework/service/WrappedCall.h"

EditorSwcVersionControl::EditorSwcVersionControl(const std::string &swcUuid,
												 QWidget *parent)
	: QDialog(parent),
	  m_SwcUuid(swcUuid),
	  m_DataFlowGraphModel(registerDataModels()) {
	setWindowTitle("SWC Version Control Manager");
	setWindowIcon(QApplication::style()->standardIcon(QStyle::SP_ComputerIcon));
	resize(1500, 900);
	setMinimumSize(1400, 800);

	// 初始化UI组件
	setupUI();

	// 初始化版本图
	m_DataFlowGraphicsScene =
		new QtNodes::DataFlowGraphicsScene(m_DataFlowGraphModel);
	m_GraphicsView = new QtNodes::GraphicsView(m_DataFlowGraphicsScene);

	m_DataFlowGraphicsScene->setOrientation(Qt::Vertical);
	m_GraphicsView->setScene(m_DataFlowGraphicsScene);
	m_GraphicsView->setContextMenuPolicy(Qt::NoContextMenu);

	connect(&m_StatusTimer, &QTimer::timeout, this, [this]() {
		m_StatusLabel->setText("🟢 Ready");
		m_StatusLabel->setStyleSheet(
			"color: #28a745; font-weight: bold; padding: 2px 8px;");
	});
	m_StatusTimer.start(3000);

	// 将图形视图添加到布局中
	m_GraphLayout->addWidget(m_GraphicsView);

	// 设置连接（必须在创建场景之后）
	setupConnections();

	// 设置默认时间为当前时间
	m_EndTimeEdit->setDateTime(QDateTime::currentDateTime());

	// 刷新版本图
	refreshVersionGraph();

	// 初始化版本信息显示
	updateVersionInfo();

	updateStatusBar("Version control manager initialized successfully",
					StatusType::Success);
}

void EditorSwcVersionControl::setupUI() {
	// 设置主窗口样式
	setStyleSheet(
		"QDialog {"
		"  background-color: #f5f5f5;"
		"}"
		"QSplitter::handle {"
		"  background-color: #ddd;"
		"  width: 3px;"
		"}"
		"QSplitter::handle:hover {"
		"  background-color: #2196F3;"
		"}");

	// 创建主布局
	auto *mainLayout = new QVBoxLayout(this);
	mainLayout->setSpacing(12);
	mainLayout->setContentsMargins(12, 12, 12, 8);

	// 设置控制面板
	setupControlPanel();
	mainLayout->addWidget(m_ControlPanel);

	// 设置主分割器
	m_MainSplitter = new QSplitter(Qt::Horizontal, this);

	// 设置版本图区域
	setupVersionGraphArea();

	// 设置信息面板
	m_InfoPanel = new QGroupBox("📊 Version Information", this);
	// 注释掉固定宽度限制，让setStretchFactor生效
	// m_InfoPanel->setMinimumWidth(320);
	// m_InfoPanel->setMaximumWidth(450);
	m_InfoPanel->setStyleSheet(
		"QGroupBox {"
		"  font-weight: bold;"
		"  border: 2px solid #cccccc;"
		"  border-radius: 8px;"
		"  margin-top: 1ex;"
		"  padding-top: 10px;"
		"  background-color: #fafafa;"
		"}"
		"QGroupBox::title {"
		"  subcontrol-origin: margin;"
		"  left: 10px;"
		"  padding: 0 8px 0 8px;"
		"  color: #2196F3;"
		"}");

	// 设置时间点面板
	setupTimePointPanel();

	auto *infoPanelLayout = new QVBoxLayout(m_InfoPanel);
	infoPanelLayout->setSpacing(10);
	infoPanelLayout->setContentsMargins(15, 20, 15, 15);

	// 状态卡片容器
	auto *statusCard = new QFrame(this);
	statusCard->setFrameStyle(QFrame::Box);
	statusCard->setStyleSheet(
		"QFrame {"
		"  border: 1px solid #e0e0e0;"
		"  border-radius: 8px;"
		"  background-color: white;"
		"  padding: 10px;"
		"}");

	auto *statusLayout = new QVBoxLayout(statusCard);
	statusLayout->setSpacing(8);
	statusLayout->setContentsMargins(12, 12, 12, 12);

	// 选中版本信息
	m_SelectedVersionLabel = new QLabel("🎯 No version selected", this);
	m_SelectedVersionLabel->setStyleSheet(
		"font-weight: bold;"
		"font-size: 14px;"
		"color: #2196F3;"
		"padding: 5px;"
		"background-color: #e3f2fd;"
		"border-radius: 4px;");
	m_SelectedVersionLabel->setWordWrap(true);
	statusLayout->addWidget(m_SelectedVersionLabel);

	// 版本总数信息
	m_TotalVersionsLabel = new QLabel("📈 Total versions: 0", this);
	m_TotalVersionsLabel->setStyleSheet(
		"color: #666;"
		"font-size: 12px;"
		"padding: 3px;");
	statusLayout->addWidget(m_TotalVersionsLabel);

	infoPanelLayout->addWidget(statusCard);

	// 详细信息区域
	auto *detailsFrame = new QFrame(this);
	detailsFrame->setFrameStyle(QFrame::Box);
	detailsFrame->setStyleSheet(
		"QFrame {"
		"  border: 1px solid #e0e0e0;"
		"  border-radius: 8px;"
		"  background-color: white;"
		"}");

	auto *detailsLayout = new QVBoxLayout(detailsFrame);
	detailsLayout->setContentsMargins(0, 0, 0, 0);

	// 详细信息标题
	auto *detailsTitle = new QLabel("📋 Details", this);
	detailsTitle->setStyleSheet(
		"font-weight: bold;"
		"font-size: 13px;"
		"color: #333;"
		"padding: 10px 15px 5px 15px;"
		"background-color: #f8f9fa;"
		"border-bottom: 1px solid #e9ecef;");
	detailsLayout->addWidget(detailsTitle);

	// 版本详细信息文本框
	m_VersionInfoText = new QTextEdit(this);
	m_VersionInfoText->setReadOnly(true);
	m_VersionInfoText->setMinimumHeight(400);
	m_VersionInfoText->setStyleSheet(
		"QTextEdit {"
		"  border: none;"
		"  background-color: white;"
		"  padding: 15px;"
		"  font-family: 'Segoe UI', Arial, sans-serif;"
		"  font-size: 13px;"
		"  line-height: 1.4;"
		"}"
		"QScrollBar:vertical {"
		"  border: none;"
		"  background: #f1f1f1;"
		"  width: 8px;"
		"  border-radius: 4px;"
		"}"
		"QScrollBar::handle:vertical {"
		"  background: #c1c1c1;"
		"  border-radius: 4px;"
		"  min-height: 20px;"
		"}"
		"QScrollBar::handle:vertical:hover {"
		"  background: #a8a8a8;"
		"}");
	m_VersionInfoText->setPlaceholderText(
		"Select a version to view detailed information...");
	detailsLayout->addWidget(m_VersionInfoText);

	infoPanelLayout->addWidget(detailsFrame);

	// 添加弹性空间
	infoPanelLayout->addStretch();

	// 添加分割器
	m_MainSplitter->addWidget(m_GraphContainer);
	m_MainSplitter->addWidget(m_InfoPanel);
	m_MainSplitter->addWidget(m_TimePointPanel);

	// 设置组件的大小策略，确保可以伸缩
	m_GraphContainer->setSizePolicy(QSizePolicy::Expanding,
									QSizePolicy::Expanding);
	m_InfoPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_TimePointPanel->setSizePolicy(QSizePolicy::Expanding,
									QSizePolicy::Expanding);

	// 设置初始大小比例
	QList<int> sizes;
	int totalWidth = 1500;
	sizes << totalWidth * 5 / 10;
	sizes << totalWidth * 2 / 10;
	sizes << totalWidth * 3 / 10;
	m_MainSplitter->setSizes(sizes);

	mainLayout->addWidget(m_MainSplitter);

	// 设置状态栏
	setupStatusBar();
	mainLayout->addWidget(m_StatusBar);
}

void EditorSwcVersionControl::setupControlPanel() {
	m_ControlPanel = new QGroupBox("🔧 Version Control Operations", this);
	m_ControlPanel->setMinimumHeight(180);
	m_ControlPanel->setMaximumHeight(200);
	m_ControlPanel->setStyleSheet(
		"QGroupBox {"
		"  font-weight: bold;"
		"  border: 2px solid #cccccc;"
		"  border-radius: 8px;"
		"  margin-top: 1ex;"
		"  padding-top: 10px;"
		"}"
		"QGroupBox::title {"
		"  subcontrol-origin: margin;"
		"  left: 10px;"
		"  padding: 0 8px 0 8px;"
		"  color: #2196F3;"
		"}");

	auto *controlLayout = new QGridLayout(m_ControlPanel);
	controlLayout->setSpacing(12);
	controlLayout->setContentsMargins(15, 20, 15, 15);

	// 第一行：时间选择和搜索
	m_EndTimeLabel = new QLabel("📅 Target Time:", this);
	m_EndTimeLabel->setStyleSheet("font-weight: bold; color: #333;");

	m_EndTimeEdit = new QDateTimeEdit(this);
	m_EndTimeEdit->setDisplayFormat("yyyy-MM-dd hh:mm:ss");
	m_EndTimeEdit->setCalendarPopup(false);
	m_EndTimeEdit->setMinimumHeight(30);
	m_EndTimeEdit->setStyleSheet(
		"QDateTimeEdit {"
		"  border: 1px solid #ddd;"
		"  border-radius: 4px;"
		"  padding: 5px;"
		"}");
	m_EndTimeEdit->setToolTip("Select the target time for version operations");

	controlLayout->addWidget(m_EndTimeLabel, 0, 0);
	controlLayout->addWidget(m_EndTimeEdit, 0, 1);

	// 第三行：操作按钮
	auto *buttonLayout = new QHBoxLayout();
	buttonLayout->setSpacing(8);

	// 样式定义
	QString primaryButtonStyle =
		"QPushButton {"
		"  background-color: #2196F3;"
		"  color: white;"
		"  border: none;"
		"  border-radius: 6px;"
		"  padding: 8px 16px;"
		"  font-weight: bold;"
		"  min-height: 20px;"
		"}"
		"QPushButton:hover {"
		"  background-color: #1976D2;"
		"}"
		"QPushButton:pressed {"
		"  background-color: #0D47A1;"
		"}";

	QString secondaryButtonStyle =
		"QPushButton {"
		"  background-color: #f5f5f5;"
		"  color: #333;"
		"  border: 1px solid #ddd;"
		"  border-radius: 6px;"
		"  padding: 8px 16px;"
		"  font-weight: bold;"
		"  min-height: 20px;"
		"}"
		"QPushButton:hover {"
		"  background-color: #e8e8e8;"
		"  border-color: #bbb;"
		"}"
		"QPushButton:pressed {"
		"  background-color: #ddd;"
		"}";

	QString dangerButtonStyle =
		"QPushButton {"
		"  background-color: #f44336;"
		"  color: white;"
		"  border: none;"
		"  border-radius: 6px;"
		"  padding: 8px 16px;"
		"  font-weight: bold;"
		"  min-height: 20px;"
		"}"
		"QPushButton:hover {"
		"  background-color: #d32f2f;"
		"}"
		"QPushButton:pressed {"
		"  background-color: #b71c1c;"
		"}";

	// 时间相关操作
	m_ExportBtn = new QPushButton("💾 Export Current Query Time Version", this);
	m_ExportBtn->setStyleSheet(primaryButtonStyle);
	m_ExportBtn->setToolTip("Export version at current query time point");

	m_RevertBtn = new QPushButton("⏪ Revert To Query Time", this);
	m_RevertBtn->setStyleSheet(dangerButtonStyle);
	m_RevertBtn->setToolTip(
		"Revert to current query time point (requires backup)");

	// 版本图操作
	m_RefreshBtn = new QPushButton("🔄 Refresh", this);
	m_RefreshBtn->setStyleSheet(secondaryButtonStyle);
	m_RefreshBtn->setToolTip("Refresh version graph");

	m_VisualizeBtn = new QPushButton("👁️ Visualize Snapshot", this);
	m_VisualizeBtn->setStyleSheet(primaryButtonStyle);
	m_VisualizeBtn->setToolTip("Visualize selected snapshot");

	m_VisualizeDiffBtn = new QPushButton("🔍 Compare Two Snapshots", this);
	m_VisualizeDiffBtn->setStyleSheet(primaryButtonStyle);
	m_VisualizeDiffBtn->setToolTip("Compare two selected snapshots");

	m_ExportSelectedBtn = new QPushButton("📤 Export Selected Snapshot", this);
	m_ExportSelectedBtn->setStyleSheet(secondaryButtonStyle);
	m_ExportSelectedBtn->setToolTip("Export selected snapshot");

	m_RevertToSnapshotBtn = new QPushButton("⏮️ Revert to Snapshot", this);
	m_RevertToSnapshotBtn->setStyleSheet(dangerButtonStyle);
	m_RevertToSnapshotBtn->setToolTip(
		"Revert to selected snapshot (requires backup)");

	// 添加按钮到布局
	buttonLayout->addWidget(m_ExportBtn);
	buttonLayout->addWidget(m_RevertBtn);
	buttonLayout->addStretch();
	buttonLayout->addWidget(m_RefreshBtn);
	buttonLayout->addWidget(m_VisualizeBtn);
	buttonLayout->addWidget(m_VisualizeDiffBtn);
	buttonLayout->addWidget(m_ExportSelectedBtn);
	buttonLayout->addWidget(m_RevertToSnapshotBtn);

	controlLayout->addLayout(buttonLayout, 2, 0, 1, 4);

	// 设置列拉伸
	controlLayout->setColumnStretch(1, 1);
	controlLayout->setColumnStretch(3, 2);
}

void EditorSwcVersionControl::setupVersionGraphArea() {
	m_GraphContainer = new QWidget(this);
	m_GraphContainer->setStyleSheet(
		"QWidget {"
		"  background-color: #fafafa;"
		"  border: 1px solid #e0e0e0;"
		"  border-radius: 8px;"
		"}");

	m_GraphLayout = new QVBoxLayout(m_GraphContainer);
	m_GraphLayout->setContentsMargins(10, 10, 10, 10);
	m_GraphLayout->setSpacing(5);

	// 标题区域
	auto *titleFrame = new QFrame(this);
	titleFrame->setFrameStyle(QFrame::NoFrame);
	titleFrame->setStyleSheet(
		"QFrame {"
		"  background-color: white;"
		"  border-radius: 6px;"
		"  padding: 8px;"
		"}");

	auto *titleLayout = new QHBoxLayout(titleFrame);
	titleLayout->setContentsMargins(15, 10, 15, 10);

	// 添加标题
	auto *titleLabel = new QLabel("🕐 Version Timeline", this);
	titleLabel->setStyleSheet(
		"font-size: 16px;"
		"font-weight: bold;"
		"color: #333;"
		"padding: 0;");
	titleLayout->addWidget(titleLabel);

	titleLayout->addStretch();

	// 添加图例
	auto *legendLabel = new QLabel(
		"💡 Tip: Click nodes to view details, select multiple for comparison",
		this);
	legendLabel->setStyleSheet(
		"font-size: 11px;"
		"color: #666;"
		"font-style: italic;");
	titleLayout->addWidget(legendLabel);

	m_GraphLayout->addWidget(titleFrame);

	// 版本图视图将在构造函数中添加
}

void EditorSwcVersionControl::setupStatusBar() {
	m_StatusBar = new QStatusBar(this);
	m_StatusBar->setStyleSheet(
		"QStatusBar {"
		"  background-color: #f8f9fa;"
		"  border-top: 1px solid #e9ecef;"
		"  padding: 5px;"
		"}"
		"QStatusBar::item {"
		"  border: none;"
		"}");

	m_StatusLabel = new QLabel("🟢 Ready", this);
	m_StatusLabel->setStyleSheet(
		"color: #28a745;"
		"font-weight: bold;"
		"padding: 2px 8px;");
	m_StatusBar->addWidget(m_StatusLabel);

	m_ProgressBar = new QProgressBar(this);
	m_ProgressBar->setVisible(false);
	m_ProgressBar->setMaximumWidth(200);
	m_ProgressBar->setMaximumHeight(18);
	m_ProgressBar->setStyleSheet(
		"QProgressBar {"
		"  border: 1px solid #ddd;"
		"  border-radius: 9px;"
		"  text-align: center;"
		"  background-color: #f0f0f0;"
		"}"
		"QProgressBar::chunk {"
		"  background-color: #2196F3;"
		"  border-radius: 8px;"
		"}");
	m_StatusBar->addPermanentWidget(m_ProgressBar);
}

void EditorSwcVersionControl::setupConnections() {
	// 时间选择相关连接
	connect(m_EndTimeEdit, &QDateTimeEdit::dateTimeChanged, this,
			&EditorSwcVersionControl::onTimeSelectionChanged);
	connect(m_ExportBtn, &QPushButton::clicked, this,
			&EditorSwcVersionControl::onExportCurrentVersion);
	connect(m_RevertBtn, &QPushButton::clicked, this,
			&EditorSwcVersionControl::onRevertVersion);

	// 版本图操作相关连接
	connect(m_RefreshBtn, &QPushButton::clicked, this,
			&EditorSwcVersionControl::onRefreshVersionGraph);
	connect(m_ExportSelectedBtn, &QPushButton::clicked, this,
			&EditorSwcVersionControl::onExportSelectedVersion);
	connect(m_RevertToSnapshotBtn, &QPushButton::clicked, this,
			&EditorSwcVersionControl::onRevertToSnapshot);
	connect(m_VisualizeBtn, &QPushButton::clicked, this,
			&EditorSwcVersionControl::onVisualizeVersion);
	connect(m_VisualizeDiffBtn, &QPushButton::clicked, this,
			&EditorSwcVersionControl::onVisualizeDiffVersion);

	// 版本图选择变化连接 - 这是关键的连接，用于更新版本信息
	connect(m_DataFlowGraphicsScene,
			&QtNodes::DataFlowGraphicsScene::selectionChanged, this,
			&EditorSwcVersionControl::updateVersionInfo);

	// 时间点面板相关连接
	connect(m_TimePointList, &QListWidget::itemSelectionChanged, this,
			&EditorSwcVersionControl::onTimePointSelected);
	connect(m_RevertToTimePointBtn, &QPushButton::clicked, this,
			&EditorSwcVersionControl::onRevertToTimePoint);
	connect(m_ExportTimePointBtn, &QPushButton::clicked, this,
			&EditorSwcVersionControl::onExportTimePoint);
	connect(m_VisualizeTimePointBtn, &QPushButton::clicked, this,
			&EditorSwcVersionControl::onVisualizeTimePoint);

	// ...existing code...
}

EditorSwcVersionControl::~EditorSwcVersionControl() {
	// 停止状态计时器
	if (m_StatusTimer.isActive()) {
		m_StatusTimer.stop();
	}

	// 断开所有信号连接，防止在销毁过程中触发回调
	disconnect();

	// 清理图形视图相关资源
	if (m_GraphicsView) {
		// 停止视图的所有渲染操作
		m_GraphicsView->setUpdatesEnabled(false);

		// 如果不是Qt管理的子对象，手动删除
		if (m_GraphicsView->parent() != this) {
			delete m_GraphicsView;
		}
		m_GraphicsView = nullptr;
	}

	// 清理图形场景
	if (m_DataFlowGraphicsScene) {
		// 清理场景中的所有图形对象
		m_DataFlowGraphicsScene->clearScene();

		// 如果不是Qt管理的子对象，手动删除
		if (m_DataFlowGraphicsScene->parent() != this) {
			delete m_DataFlowGraphicsScene;
		}
		m_DataFlowGraphicsScene = nullptr;
	}
}

// 槽函数实现
void EditorSwcVersionControl::onTimeSelectionChanged() {
	// 时间选择变化时的处理
	updateStatusBar("Time selection changed to " +
					m_EndTimeEdit->dateTime().toString());
}

void EditorSwcVersionControl::onExportCurrentVersion() {
	// 导出当前版本
	updateStatusBar("Exporting current version...", StatusType::Working);
	setProgressBar(30, 100);

	getSwcLastSnapshot();

	setProgressBar(0, 100);
	m_ProgressBar->setVisible(false);
}

void EditorSwcVersionControl::onRevertVersion() {
	// 版本回退功能
	m_EndTime = m_EndTimeEdit->dateTime().toSecsSinceEpoch();

	auto result = QMessageBox::question(
		this, "Confirm Revert",
		QString("Are you sure you want to revert this SWC to %1?\n\n"
				"⚠️ WARNING: You may lose all changes after that time!\n"
				"Please ensure you have exported a backup before continuing.")
			.arg(QDateTime::fromSecsSinceEpoch(m_EndTime).toString()),
		QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

	if (result == QMessageBox::Yes) {
		if (!m_HasExportedStatus) {
			QMessageBox::warning(
				this, "Backup Required",
				"Please export a backup before reverting this SWC!");
			return;
		}

		if (m_SnapshotName.empty()) {
			QMessageBox::warning(
				this, "No Snapshot Found",
				"No snapshot found for the selected time!\n"
				"The end time must correspond to an existing snapshot.");
			return;
		}

		updateStatusBar("Reverting version...", StatusType::Working);
		setProgressBar(50, 100);

		proto::RevertSwcVersionResponse response;
		google::protobuf::Timestamp endTime;
		endTime.set_seconds(m_EndTime);

		if (!WrappedCall::RevertSwcVersionByUuid(m_SwcUuid, endTime, response,
												 this)) {
			setProgressBar(0, 100);
			m_ProgressBar->setVisible(false);
			return;
		}

		setProgressBar(0, 100);
		m_ProgressBar->setVisible(false);
		updateStatusBar("Version reverted successfully!", StatusType::Success);

		QMessageBox::information(this, "Success",
								 "SWC version reverted successfully!");

		// 刷新版本图
		refreshVersionGraph();
	}
}

void EditorSwcVersionControl::onRefreshVersionGraph() {
	// 刷新版本图
	updateStatusBar("Refreshing version graph...");
	setProgressBar(25, 100);

	refreshVersionGraph();

	setProgressBar(0, 100);
	m_ProgressBar->setVisible(false);
	updateStatusBar("Version graph refreshed successfully");
}

void EditorSwcVersionControl::onExportSelectedVersion() {
	// 导出选中版本
	if (!m_DataFlowGraphicsScene) {
		return;
	}

	auto selectedNodes = m_DataFlowGraphicsScene->selectedNodes();
	if (selectedNodes.empty()) {
		QMessageBox::information(this, "No Selection",
								 "Please select a snapshot to export.");
		return;
	}

	auto selectedNode = selectedNodes[0];
	auto internalData =
		m_DataFlowGraphModel
			.nodeData(selectedNode, QtNodes::NodeRole::InternalData)
			.value<std::any>();
	QString snapShotCName;
	if (internalData.has_value()) {
		// check if snapshot not increment
		try {
			auto snapshot =
				std::any_cast<proto::SwcSnapshotMetaInfoV1>(internalData);
			snapShotCName =
				QString::fromStdString(snapshot.swcsnapshotcollectionname());
		} catch (const std::bad_any_cast &e) {
			QMessageBox::warning(this, "Invalid Selection",
								 "Please select a valid snapshot node.");
			return;
		}
	}

	auto result = QMessageBox::question(
		this, "Confirm Export",
		QString("Export snapshot: %1?").arg(snapShotCName),
		QMessageBox::Yes | QMessageBox::No);

	if (result != QMessageBox::Yes) {
		return;
	}

	updateStatusBar("Exporting selected version...");
	setProgressBar(40, 100);

	proto::GetSwcMetaInfoResponse response;
	if (!WrappedCall::getSwcMetaInfoByUuid(m_SwcUuid, response, this)) {
		setProgressBar(0, 100);
		m_ProgressBar->setVisible(false);
		return;
	}

	std::vector<ExportSwcData> exportSwcData;
	std::string name = snapShotCName.toStdString();
	exportSwcData.push_back({response.swcinfo(), {}, true, name});

	setProgressBar(0, 100);
	m_ProgressBar->setVisible(false);
	updateStatusBar("Opening export dialog...");

	ViewExportSwcToFile view(exportSwcData, true, this);
	view.exec();

	updateStatusBar("Export completed");
}

void EditorSwcVersionControl::onVisualizeVersion() {
	// 可视化选中版本
	if (!m_DataFlowGraphicsScene) {
		return;
	}

	auto selectedNodes = m_DataFlowGraphicsScene->selectedNodes();
	if (selectedNodes.empty()) {
		QMessageBox::information(this, "No Selection",
								 "Please select a snapshot to visualize.");
		return;
	}

	QString snapShotCName;
	auto selectedNode = selectedNodes[0];
	auto internalData =
		m_DataFlowGraphModel
			.nodeData(selectedNode, QtNodes::NodeRole::InternalData)
			.value<std::any>();

	if (internalData.has_value()) {
		try {
			auto snapshot =
				std::any_cast<proto::SwcSnapshotMetaInfoV1>(internalData);
			snapShotCName =
				QString::fromStdString(snapshot.swcsnapshotcollectionname());
		} catch (const std::bad_any_cast &e) {
			QMessageBox::warning(this, "Invalid Selection",
								 "Please select a valid snapshot node.");
			return;
		}
	}

	auto result = QMessageBox::question(
		this, "Confirm Visualization",
		QString("Visualize snapshot: %1?").arg(snapShotCName),
		QMessageBox::Yes | QMessageBox::No);

	if (result != QMessageBox::Yes) {
		return;
	}

	updateStatusBar("Loading snapshot data...");
	setProgressBar(60, 100);

	proto::GetSnapshotResponse snapResponse;
	if (!WrappedCall::getSwcSnapshot(snapShotCName.toStdString(), snapResponse,
									 this)) {
		setProgressBar(0, 100);
		m_ProgressBar->setVisible(false);
		return;
	}

	setProgressBar(0, 100);
	m_ProgressBar->setVisible(false);
	updateStatusBar("Opening visualization...");

	SwcRendererCreateInfo createInfo;
	createInfo.swcData = snapResponse.swcnodedata();
	auto *renderer = new SwcRendererDailog(createInfo);
	renderer->setAttribute(Qt::WA_DeleteOnClose);
	renderer->exec();

	updateStatusBar("Visualization opened");
}

void EditorSwcVersionControl::onVisualizeDiffVersion() {
	// 比较两个版本
	if (!m_DataFlowGraphicsScene) {
		return;
	}

	auto selectedNodes = m_DataFlowGraphicsScene->selectedNodes();
	if (selectedNodes.size() != 2) {
		QMessageBox::information(
			this, "Invalid Selection",
			"Please select exactly two snapshots to compare.");
		return;
	}

	std::string snapShotCName1, snapShotCName2;

	// 获取第一个选中的快照
	auto selectedNode1 = selectedNodes[0];
	auto internalData1 =
		m_DataFlowGraphModel
			.nodeData(selectedNode1, QtNodes::NodeRole::InternalData)
			.value<std::any>();
	if (internalData1.has_value()) {
		try {
			auto snapshot =
				std::any_cast<proto::SwcSnapshotMetaInfoV1>(internalData1);
			snapShotCName1 = snapshot.swcsnapshotcollectionname();
		} catch (const std::bad_any_cast &e) {
			QMessageBox::warning(this, "Invalid Selection",
								 "Please select a valid snapshot node.");
			return;
		}
	}

	// 获取第二个选中的快照
	auto selectedNode2 = selectedNodes[1];
	auto internalData2 =
		m_DataFlowGraphModel
			.nodeData(selectedNode2, QtNodes::NodeRole::InternalData)
			.value<std::any>();
	if (internalData2.has_value()) {
		try {
			auto snapshot =
				std::any_cast<proto::SwcSnapshotMetaInfoV1>(internalData2);
			snapShotCName2 = snapshot.swcsnapshotcollectionname();
		} catch (const std::bad_any_cast &e) {
			QMessageBox::warning(this, "Invalid Selection",
								 "Please select a valid snapshot node.");
			return;
		}
	}

	// 确定时间顺序
	google::protobuf::Timestamp selectedNode1Time, selectedNode2Time;
	for (auto &snap : m_SwcSnapshots) {
		if (snap.swcsnapshotcollectionname() == snapShotCName1) {
			selectedNode1Time = snap.createtime();
		}
		if (snap.swcsnapshotcollectionname() == snapShotCName2) {
			selectedNode2Time = snap.createtime();
		}
	}

	std::string oldCName, newCName;
	if (isEarlier(selectedNode1Time, selectedNode2Time)) {
		oldCName = snapShotCName1;
		newCName = snapShotCName2;
	} else {
		newCName = snapShotCName1;
		oldCName = snapShotCName2;
	}

	auto result =
		QMessageBox::question(this, "Confirm Comparison",
							  QString("Compare versions?\n\nOld: %1\nNew: %2")
								  .arg(QString::fromStdString(oldCName))
								  .arg(QString::fromStdString(newCName)),
							  QMessageBox::Yes | QMessageBox::No);

	if (result != QMessageBox::Yes) {
		return;
	}

	updateStatusBar("Loading snapshots for comparison...");
	setProgressBar(30, 100);

	proto::GetSnapshotResponse oldSnapResponse, newSnapResponse;
	if (!WrappedCall::getSwcSnapshot(oldCName, oldSnapResponse, this)) {
		setProgressBar(0, 100);
		m_ProgressBar->setVisible(false);
		return;
	}

	setProgressBar(60, 100);
	if (!WrappedCall::getSwcSnapshot(newCName, newSnapResponse, this)) {
		setProgressBar(0, 100);
		m_ProgressBar->setVisible(false);
		return;
	}

	setProgressBar(0, 100);
	m_ProgressBar->setVisible(false);
	updateStatusBar("Opening comparison view...");

	SwcRendererCreateInfo createInfo;
	createInfo.mode = SwcRendererMode::eVisualizeDiffSwc;
	createInfo.swcData = oldSnapResponse.swcnodedata();
	createInfo.newSwcData = newSnapResponse.swcnodedata();
	auto *renderer = new SwcRendererDailog(createInfo);
	renderer->setAttribute(Qt::WA_DeleteOnClose);
	renderer->exec();

	updateStatusBar("Comparison view opened");
}

// 辅助功能函数实现
void EditorSwcVersionControl::updateVersionInfo() {
	// 更新版本信息显示
	m_TotalVersionsLabel->setText(
		QString("Total versions: %1 snapshots, %2 operations")
			.arg(m_SwcSnapshots.size())
			.arg(m_SwcIncrements.size()));

	if (!m_DataFlowGraphicsScene) {
		m_SelectedVersionLabel->setText("No version selected");
		m_VersionInfoText->setHtml(
			"<p style='color: #666; font-style: italic;'>"
			"Graphics scene not available...</p>");
		return;
	}

	auto selectedNodes = m_DataFlowGraphicsScene->selectedNodes();
	if (selectedNodes.empty()) {
		m_SelectedVersionLabel->setText("No version selected");
		m_VersionInfoText->setHtml(
			"<p style='color: #666; font-style: italic;'>"
			"Select a version node to view detailed information...</p>");

		// 清空时间点显示
		m_TimePointList->clear();
		m_TimePointStatusLabel->setText(
			"Select an increment operation to view time points");
		m_RevertToTimePointBtn->setEnabled(false);
		m_ExportTimePointBtn->setEnabled(false);
		m_VisualizeTimePointBtn->setEnabled(false);

		return;
	}

	if (selectedNodes.size() == 1) {
		auto selectedNode = selectedNodes[0];
		auto internalData =
			m_DataFlowGraphModel
				.nodeData(selectedNode, QtNodes::NodeRole::InternalData)
				.value<std::any>();

		if (internalData.has_value()) {
			// 尝试解析为快照节点
			try {
				auto snapshot =
					std::any_cast<proto::SwcSnapshotMetaInfoV1>(internalData);
				m_SelectedVersionLabel->setText(
					QString("Selected: Snapshot %1")
						.arg(QString::fromStdString(
							snapshot.swcsnapshotcollectionname())));

				// 显示快照详细信息
				QString info =
					QString(
						"<h3 style='color: #2196F3; margin-bottom: 10px;'>📸 "
						"Snapshot "
						"Information</h3>"
						"<table style='width: 100%; border-spacing: 8px;'>"
						"<tr><td style='font-weight: bold; color: "
						"#333;'>Collection "
						"Name:</td>"
						"<td style='font-family: monospace; background: "
						"#f5f5f5; "
						"padding: 2px 4px;'>%1</td></tr>"
						"<tr><td style='font-weight: bold; color: "
						"#333;'>Created "
						"Time:</td>"
						"<td>%2</td></tr>"
						"<tr><td style='font-weight: bold; color: "
						"#333;'>Timestamp:</td>"
						"<td style='font-family: monospace;'>%3</td></tr>"
						"<tr><td style='font-weight: bold; color: "
						"#333;'>Type:</td>"
						"<td style='color: #4CAF50; font-weight: "
						"bold;'>Version "
						"Snapshot</td></tr>"
						"</table>"
						"<p style='margin-top: 15px; padding: 8px; background: "
						"#e8f5e8; border-left: 3px solid #4CAF50;'>"
						"<strong>💡 Actions Available:</strong><br>"
						"• Click 'Visualize' to view this snapshot in 3D<br>"
						"• Click 'Export Selected' to save this snapshot<br>"
						"• Select another snapshot and click 'Compare' for "
						"diff view"
						"</p>")
						.arg(QString::fromStdString(
							snapshot.swcsnapshotcollectionname()))
						.arg(formatDateTime(snapshot.createtime().seconds()))
						.arg(QString::number(snapshot.createtime().seconds()));

				m_VersionInfoText->setHtml(info);
				updateStatusBar(QString("Selected snapshot: %1")
									.arg(QString::fromStdString(
										snapshot.swcsnapshotcollectionname())));

				// 清空时间点显示
				m_TimePointList->clear();
				m_TimePointStatusLabel->setText(
					"Select an increment operation to view time points");
				m_RevertToTimePointBtn->setEnabled(false);
				m_ExportTimePointBtn->setEnabled(false);
				m_VisualizeTimePointBtn->setEnabled(false);

				return;
			} catch (const std::bad_any_cast &) {
				// 不是快照节点，尝试解析为增量操作节点
			}

			// 尝试解析为增量操作节点
			try {
				auto increment =
					std::any_cast<proto::SwcIncrementOperationMetaInfoV1>(
						internalData);
				m_SelectedVersionLabel->setText(
					QString("Selected: Increment Operation"));

				// 存储当前选中的increment信息
				m_CurrentSelectedIncrement = increment;

				// 使用新的函数显示增量操作信息（带加载状态）
				updateIncrementOperationInfo(increment, true);

				updateStatusBar(
					QString("Selected increment operation: %1")
						.arg(QString::fromStdString(
							increment.incrementoperationcollectionname())));

				// 加载增量操作的时间点数据
				loadIncrementTimePoints(
					increment.incrementoperationcollectionname());

				return;
			} catch (const std::bad_any_cast &) {
				// 都不是，显示未知节点
			}
		}

		// 如果无法解析节点数据
		m_SelectedVersionLabel->setText("Selected: Unknown node type");
		m_VersionInfoText->setHtml(
			"<p style='color: #f44336;'>⚠️ Unable to parse node "
			"information</p>");

	} else if (selectedNodes.size() == 2) {
		m_SelectedVersionLabel->setText("Two versions selected for comparison");
		m_VersionInfoText->setHtml(
			"<h3 style='color: #9C27B0; margin-bottom: 10px;'>🔍 Comparison "
			"Mode</h3>"
			"<p style='padding: 8px; background: #f3e5f5; border-left: 3px "
			"solid "
			"#9C27B0;'>"
			"<strong>Ready to compare versions!</strong><br><br>"
			"You have selected two nodes. Click the <strong>'Compare'</strong> "
			"button to:"
			"</p>"
			"<ul style='margin: 10px 0; padding-left: 20px;'>"
			"<li>View differences between the two versions</li>"
			"<li>See added, modified, and deleted elements</li>"
			"<li>Understand how your SWC evolved</li>"
			"</ul>"
			"<p style='color: #666; font-size: 12px; margin-top: 10px;'>"
			"💡 Tip: The system will automatically determine which version is "
			"older."
			"</p>");
		updateStatusBar("Two versions selected - ready for comparison");

	} else {
		m_SelectedVersionLabel->setText(
			QString("%1 versions selected").arg(selectedNodes.size()));
		m_VersionInfoText->setHtml(
			"<p style='color: #ff9800;'>⚠️ Multiple versions selected</p>"
			"<p>Some operations may not be available with multiple selections. "
			"Please select 1-2 versions for optimal functionality.</p>");
		updateStatusBar(
			QString("%1 versions selected").arg(selectedNodes.size()));
	}
}

void EditorSwcVersionControl::updateStatusBar(const QString &message) {
	updateStatusBar(message, StatusType::Ready);
}

void EditorSwcVersionControl::updateStatusBar(const QString &message,
											  StatusType type) {
	// 根据状态类型设置不同的图标和颜色
	QString statusText;
	QString styleSheet;

	switch (type) {
		case StatusType::Ready:
			statusText = "🟢 " + message;
			styleSheet = "color: #28a745; font-weight: bold; padding: 2px 8px;";
			break;
		case StatusType::Working:
			statusText = "🔄 " + message;
			styleSheet = "color: #007bff; font-weight: bold; padding: 2px 8px;";
			break;
		case StatusType::Success:
			statusText = "✅ " + message;
			styleSheet = "color: #28a745; font-weight: bold; padding: 2px 8px;";
			break;
		case StatusType::Warning:
			statusText = "⚠️ " + message;
			styleSheet = "color: #ffc107; font-weight: bold; padding: 2px 8px;";
			break;
		case StatusType::Error:
			statusText = "❌ " + message;
			styleSheet = "color: #dc3545; font-weight: bold; padding: 2px 8px;";
			break;
	}

	m_StatusLabel->setText(statusText);
	m_StatusLabel->setStyleSheet(styleSheet);
}

void EditorSwcVersionControl::setProgressBar(int value, int maximum) {
	// 设置进度条
	if (value <= 0) {
		m_ProgressBar->setVisible(false);
		return;
	}

	m_ProgressBar->setMaximum(maximum);
	m_ProgressBar->setValue(value);
	m_ProgressBar->setVisible(true);
}

QString EditorSwcVersionControl::formatDateTime(int64_t timestamp) {
	// 格式化时间戳
	return QDateTime::fromSecsSinceEpoch(timestamp).toString(
		"yyyy-MM-dd hh:mm:ss");
}

void EditorSwcVersionControl::refreshVersionGraph() {
	// 如果图形场景已经被销毁，直接返回
	if (!m_DataFlowGraphicsScene) {
		return;
	}

	// 获取所有快照和增量记录
	getAllSnapshot();
	getAllSwcIncrementRecord();

	// 清除现有节点
	for (auto &nodeId : m_DataFlowGraphModel.allNodeIds()) {
		m_DataFlowGraphModel.deleteNode(nodeId);
	}

	std::map<std::string, QtNodes::NodeId> snapshotNodeMap;

	// 创建快照节点
	for (int idx = 0; idx < m_SwcSnapshots.size(); idx++) {
		QtNodes::NodeId const newId =
			m_DataFlowGraphModel.addNode("SnapshotDelegateModel");
		m_DataFlowGraphModel.setNodeData(
			newId, QtNodes::NodeRole::Position,
			QPointF{static_cast<qreal>(idx * 900), 0});

		QString caption = QString("Snapshot #%1\nCreated: %2")
							  .arg(idx + 1)
							  .arg(formatDateTime(
								  m_SwcSnapshots[idx].createtime().seconds()));

		m_DataFlowGraphModel.setNodeData(newId, QtNodes::NodeRole::Caption,
										 caption);
		m_DataFlowGraphModel.setNodeData(
			newId, QtNodes::NodeRole::InternalData,
			QVariant::fromValue(std::any(m_SwcSnapshots[idx])));
		snapshotNodeMap.insert(
			{m_SwcSnapshots[idx].swcsnapshotcollectionname(), newId});
	}

	// 创建增量操作节点
	for (int idx = 0; idx < m_SwcIncrements.size(); idx++) {
		QtNodes::NodeId const newId =
			m_DataFlowGraphModel.addNode("IncrementOperationDelegateModel");
		m_DataFlowGraphModel.setNodeData(
			newId, QtNodes::NodeRole::Position,
			QPointF{static_cast<qreal>(400 + idx * 900), 300});

		QString caption = QString("Increment Operation\nFrom v%1 to v%2")
							  .arg(idx + 1)
							  .arg(idx + 2);

		m_DataFlowGraphModel.setNodeData(newId, QtNodes::NodeRole::Caption,
										 caption);
		m_DataFlowGraphModel.setNodeData(
			newId, QtNodes::NodeRole::InternalData,
			QVariant::fromValue(std::any(m_SwcIncrements[idx])));

		// 连接节点
		for (int idx2 = 0; idx2 < m_SwcSnapshots.size(); idx2++) {
			auto snap = m_SwcSnapshots[idx2];
			if (m_SwcIncrements[idx].startsnapshot() ==
				snap.swcsnapshotcollectionname()) {
				m_DataFlowGraphModel.addConnection(
					{snapshotNodeMap[snap.swcsnapshotcollectionname()], 0,
					 newId, 0});
				if (idx2 + 1 < m_SwcSnapshots.size()) {
					m_DataFlowGraphModel.addConnection(
						{newId, 0,
						 snapshotNodeMap[m_SwcSnapshots[idx + 1]
											 .swcsnapshotcollectionname()],
						 0});
				}
				break;
			}
		}
	}

	// 更新版本信息
	updateVersionInfo();
	updateStatusBar("Version graph refreshed successfully");
}

void EditorSwcVersionControl::getSwcLastSnapshot() {
	grpc::ClientContext context;

	proto::GetAllSnapshotMetaInfoRequest request;
	proto::GetAllSnapshotMetaInfoResponse response;
	WrappedCall::setCommonRequestField(request);

	request.set_swcuuid(m_SwcUuid);

	if (auto status = RpcCall::getInstance().Stub()->GetAllSnapshotMetaInfo(
			&context, request, &response);
		status.ok()) {
		if (response.metainfo().status()) {
			if (response.swcsnapshotlist_size() == 0) {
				QMessageBox::critical(
					this, "Error",
					"You need to create at least one snapshot to "
					"enable version control system!");
			} else {
				auto queryEndTime =
					m_EndTimeEdit->dateTime().toSecsSinceEpoch();

				int64_t currentSnapTime = 0;
				proto::SwcSnapshotMetaInfoV1 currentSnap;
				for (auto &snap : response.swcsnapshotlist()) {
					if (snap.createtime().seconds() <= queryEndTime &&
						snap.createtime().seconds() >= currentSnapTime) {
						currentSnapTime = snap.createtime().seconds();
						currentSnap.CopyFrom(snap);
					}
				}
				if (currentSnapTime == 0) {
					QMessageBox::critical(
						this, "Error",
						"No snapshot found based on the end time you provided! "
						"End "
						"time must be based on one snapshot!");
				} else {
					getSwcIncrementRecord(currentSnap, queryEndTime);
				}
			}
		} else {
			QMessageBox::critical(
				this, "Error",
				QString::fromStdString(response.metainfo().message()));
		}
	} else {
		QMessageBox::critical(this, "Error",
							  QString::fromStdString(status.error_message()));
	}
}

void EditorSwcVersionControl::getSwcIncrementRecord(
	proto::SwcSnapshotMetaInfoV1 snapshot, int64_t endTime) {
	grpc::ClientContext context;

	proto::GetAllIncrementOperationMetaInfoRequest request;
	proto::GetAllIncrementOperationMetaInfoResponse response;
	WrappedCall::setCommonRequestField(request);

	request.set_swcuuid(m_SwcUuid);

	proto::SwcIncrementOperationMetaInfoV1 currentIncrement;

	if (auto status =
			RpcCall::getInstance().Stub()->GetAllIncrementOperationMetaInfo(
				&context, request, &response);
		status.ok()) {
		if (response.metainfo().status()) {
			for (auto &increment : response.swcincrementoperationmetainfo()) {
				if (increment.startsnapshot() ==
					snapshot.swcsnapshotcollectionname()) {
					currentIncrement.CopyFrom(increment);
					break;
				}
			}
			if (currentIncrement.startsnapshot().empty()) {
				QMessageBox::critical(
					this, "Error",
					"Cannot found the corresponding increment record!");
			} else {
				proto::GetSnapshotResponse snapResponse;
				if (!WrappedCall::getSwcSnapshot(
						snapshot.swcsnapshotcollectionname(), snapResponse,
						this)) {
					return;
				}
				std::vector<proto::SwcNodeDataV1> nodeData;
				for (auto &node : snapResponse.swcnodedata().swcdata()) {
					nodeData.push_back(node);
				}

				proto::GetIncrementOperationResponse incrementResponse;
				if (!WrappedCall::getSwcIncrementRecord(
						currentIncrement.incrementoperationcollectionname(),
						incrementResponse, this)) {
					return;
				}

				m_EndTime = endTime;
				m_SnapshotName = snapshot.swcsnapshotcollectionname();
				m_IncremrntOpName =
					currentIncrement.incrementoperationcollectionname();

				for (auto &increment :
					 incrementResponse.swcincrementoperationlist()
						 .swcincrementoperation()) {
					promoteOperation(nodeData, increment, endTime);
				}

				proto::GetSwcMetaInfoResponse swc_meta_info_response;
				if (!WrappedCall::getSwcMetaInfoByUuid(
						m_SwcUuid, swc_meta_info_response, this)) {
					return;
				}

				proto::SwcDataV1 exportSwcData;

				for (auto &node : nodeData) {
					auto data = exportSwcData.add_swcdata();
					data->CopyFrom(node);
				}

				// export data
				ExportSwcData exportData{
					.swcMetaInfo = swc_meta_info_response.swcinfo(),
					.swcData = exportSwcData,
					.isSnapshot = true,
					.swcSnapshotCollectionName = ""};

				std::vector<ExportSwcData> exportDataVec;
				exportDataVec.push_back(exportData);

				ViewExportSwcToFile view(exportDataVec, false, this);
				view.exec();
				m_HasExportedStatus = true;
			}
		} else {
			QMessageBox::critical(
				this, "Error",
				QString::fromStdString(response.metainfo().message()));
		}
	} else {
		QMessageBox::critical(this, "Error",
							  QString::fromStdString(status.error_message()));
	}
}

void EditorSwcVersionControl::promoteOperation(
	std::vector<proto::SwcNodeDataV1> &nodeData,
	const proto::SwcIncrementOperationV1 &op, int64_t endTime) {
	if (op.createtime().seconds() <= endTime) {
		switch (op.incrementoperation()) {
			case proto::Unknown:
				break;
			case proto::Create: {
				for (auto &newData : op.swcdata().swcdata()) {
					nodeData.push_back(newData);
				}
				break;
			}
			case proto::Delete: {
				for (auto &deletedData : op.swcdata().swcdata()) {
					auto it = std::find_if(nodeData.begin(), nodeData.end(),
										   [&](proto::SwcNodeDataV1 &node) {
											   return node.base().uuid() ==
													  deletedData.base().uuid();
										   });
					if (it != nodeData.end()) {
						nodeData.erase(it);
					}
				}

				int counter = 1;
				proto::SwcNodeDataV1 *lastNode = nullptr;
				for (auto &node : nodeData) {
					node.mutable_swcnodeinternaldata()->set_n(counter++);
					if (lastNode != nullptr) {
						if (lastNode->swcnodeinternaldata().parent() != -1) {
							lastNode->mutable_swcnodeinternaldata()->set_parent(
								node.swcnodeinternaldata().n());
						}
					}
					lastNode = &node;
				}
				break;
			}
			case proto::Update: {
				for (auto &updateData : op.swcdata().swcdata()) {
					for (auto &node : nodeData) {
						if (node.base().uuid() == updateData.base().uuid()) {
							node.CopyFrom(updateData);
						}
					}
				}
				break;
			}
			case proto::UpdateNParent: {
				std::unordered_map<std::string, int> indexMap;
				for (int i = 0; i < nodeData.size(); i++) {
					indexMap[nodeData[i].base().uuid()] = i;
				}

				for (auto &updateData : op.nodenparent()) {
					auto iter = indexMap.find(updateData.nodeuuid());
					if (iter != indexMap.end()) {
						nodeData.at(iter->second)
							.mutable_swcnodeinternaldata()
							->set_n(updateData.n());
						nodeData.at(iter->second)
							.mutable_swcnodeinternaldata()
							->set_parent(updateData.parent());
					}
				}
				break;
			}
			case proto::ClearAll: {
				nodeData.clear();
				break;
			}
			case proto::OverwriteAll: {
				nodeData.clear();
				for (auto &newData : op.swcdata().swcdata()) {
					nodeData.push_back(newData);
				}
				break;
			}
			default:;
		}
	}
}

void EditorSwcVersionControl::getAllSnapshot() {
	grpc::ClientContext context;
	proto::GetAllSnapshotMetaInfoRequest request;
	proto::GetAllSnapshotMetaInfoResponse response;
	WrappedCall::setCommonRequestField(request);
	request.set_swcuuid(m_SwcUuid);

	auto status = RpcCall::getInstance().Stub()->GetAllSnapshotMetaInfo(
		&context, request, &response);
	if (status.ok()) {
		if (response.has_metainfo() && response.metainfo().status() == true) {
			m_SwcSnapshots.clear();
			for (auto &snapshot : response.swcsnapshotlist()) {
				m_SwcSnapshots.push_back(snapshot);
			}
		} else {
			QMessageBox::critical(
				this, "Error",
				QString::fromStdString(response.metainfo().message()));
		}
	} else {
		QMessageBox::critical(this, "Error",
							  QString::fromStdString(status.error_message()));
	}
}

void EditorSwcVersionControl::getAllSwcIncrementRecord() {
	proto::GetAllIncrementOperationMetaInfoResponse response;

	if (WrappedCall::getAllSwcIncrementRecordByUuid(m_SwcUuid, response,
													this)) {
		m_SwcIncrements.clear();
		for (auto &incrementRecordCollection :
			 response.swcincrementoperationmetainfo()) {
			m_SwcIncrements.push_back(incrementRecordCollection);
		}
	}
}

bool EditorSwcVersionControl::isEarlier(
	const google::protobuf::Timestamp &time1,
	const google::protobuf::Timestamp &time2) const {
	// 比较两个时间戳，如果time1早于time2则返回true
	if (time1.seconds() < time2.seconds()) {
		return true;
	} else if (time1.seconds() == time2.seconds()) {
		return time1.nanos() < time2.nanos();
	}
	return false;
}

// 时间点面板相关函数实现
void EditorSwcVersionControl::setupTimePointPanel() {
	m_TimePointPanel = new QGroupBox("⏰ Recoverable Time Points", this);
	// 注释掉固定宽度限制，让setStretchFactor生效
	// m_TimePointPanel->setMinimumWidth(300);
	// m_TimePointPanel->setMaximumWidth(400);
	m_TimePointPanel->setStyleSheet(
		"QGroupBox {"
		"  font-weight: bold;"
		"  border: 2px solid #cccccc;"
		"  border-radius: 8px;"
		"  margin-top: 1ex;"
		"  padding-top: 10px;"
		"  background-color: #fafafa;"
		"}"
		"QGroupBox::title {"
		"  subcontrol-origin: margin;"
		"  left: 10px;"
		"  padding: 0 8px 0 8px;"
		"  color: #FF9800;"
		"}");

	auto *timePointLayout = new QVBoxLayout(m_TimePointPanel);
	timePointLayout->setSpacing(10);
	timePointLayout->setContentsMargins(15, 20, 15, 15);

	// 状态标签
	m_TimePointStatusLabel =
		new QLabel("Select an increment operation to view time points", this);
	m_TimePointStatusLabel->setStyleSheet(
		"color: #666;"
		"font-style: italic;"
		"padding: 5px;"
		"background-color: #f8f9fa;"
		"border-radius: 4px;");
	m_TimePointStatusLabel->setWordWrap(true);
	timePointLayout->addWidget(m_TimePointStatusLabel);

	// 时间点列表
	m_TimePointList = new QListWidget(this);
	m_TimePointList->setStyleSheet(
		"QListWidget {"
		"  border: 1px solid #e0e0e0;"
		"  border-radius: 6px;"
		"  background-color: white;"
		"  padding: 5px;"
		"  selection-background-color: #e3f2fd;"
		"}"
		"QListWidget::item {"
		"  padding: 8px;"
		"  border-bottom: 1px solid #f0f0f0;"
		"  border-radius: 4px;"
		"  margin: 2px;"
		"}"
		"QListWidget::item:selected {"
		"  background-color: #2196F3;"
		"  color: white;"
		"}"
		"QListWidget::item:hover {"
		"  background-color:rgb(136, 136, 136);"
		"}");
	m_TimePointList->setMinimumHeight(400);
	timePointLayout->addWidget(m_TimePointList);

	// 操作按钮
	auto *buttonLayout = new QVBoxLayout();
	buttonLayout->setSpacing(8);

	QString primaryButtonStyle =
		"QPushButton {"
		"  background-color: #FF9800;"
		"  color: white;"
		"  border: none;"
		"  border-radius: 6px;"
		"  padding: 8px 12px;"
		"  font-weight: bold;"
		"  min-height: 16px;"
		"}"
		"QPushButton:hover {"
		"  background-color: #F57C00;"
		"}"
		"QPushButton:pressed {"
		"  background-color: #E65100;"
		"}"
		"QPushButton:disabled {"
		"  background-color: #ccc;"
		"  color: #888;"
		"}";

	QString secondaryButtonStyle =
		"QPushButton {"
		"  background-color: #f5f5f5;"
		"  color: #333;"
		"  border: 1px solid #ddd;"
		"  border-radius: 6px;"
		"  padding: 8px 12px;"
		"  font-weight: bold;"
		"  min-height: 16px;"
		"}"
		"QPushButton:hover {"
		"  background-color: #e8e8e8;"
		"  border-color: #bbb;"
		"}"
		"QPushButton:pressed {"
		"  background-color: #ddd;"
		"}"
		"QPushButton:disabled {"
		"  background-color: #f8f8f8;"
		"  color: #ccc;"
		"  border-color: #eee;"
		"}";

	m_RevertToTimePointBtn = new QPushButton("⏪ Revert to Time Point", this);
	m_RevertToTimePointBtn->setStyleSheet(primaryButtonStyle);
	m_RevertToTimePointBtn->setToolTip("Revert SWC to selected time point");
	m_RevertToTimePointBtn->setEnabled(false);

	m_ExportTimePointBtn = new QPushButton("📤 Export Time Point", this);
	m_ExportTimePointBtn->setStyleSheet(secondaryButtonStyle);
	m_ExportTimePointBtn->setToolTip("Export SWC data at selected time point");
	m_ExportTimePointBtn->setEnabled(false);

	m_VisualizeTimePointBtn = new QPushButton("👁️ Visualize Time Point", this);
	m_VisualizeTimePointBtn->setStyleSheet(secondaryButtonStyle);
	m_VisualizeTimePointBtn->setToolTip(
		"Visualize SWC data at selected time point");
	m_VisualizeTimePointBtn->setEnabled(false);

	buttonLayout->addWidget(m_RevertToTimePointBtn);
	buttonLayout->addWidget(m_ExportTimePointBtn);
	buttonLayout->addWidget(m_VisualizeTimePointBtn);
	timePointLayout->addLayout(buttonLayout);

	timePointLayout->addStretch();
}

void EditorSwcVersionControl::loadIncrementTimePoints(
	const std::string &incrementOpName) {
	// 显示加载状态
	updateStatusBar("Loading increment operation time points...",
					StatusType::Working);
	setProgressBar(0, 100);
	m_ProgressBar->setVisible(true);

	// 清空当前时间点数据
	m_CurrentTimePoints.clear();
	m_CurrentIncrementOpName = incrementOpName;

	// 更新UI状态，显示正在加载
	m_TimePointList->clear();
	m_TimePointStatusLabel->setText("🔄 Loading time points, please wait...");
	m_RevertToTimePointBtn->setEnabled(false);
	m_ExportTimePointBtn->setEnabled(false);
	m_VisualizeTimePointBtn->setEnabled(false);

	// 设置初始进度
	setProgressBar(10, 100);

	// 使用QTimer来分步骤执行，避免阻塞UI
	QTimer::singleShot(50, this, [this, incrementOpName]() {
		setProgressBar(30, 100);

		// 获取增量操作数据
		proto::GetIncrementOperationResponse response;

		// 在这里执行实际的网络调用
		QTimer::singleShot(
			100, this, [this, incrementOpName, response]() mutable {
				setProgressBar(50, 100);

				if (!WrappedCall::getSwcIncrementRecord(incrementOpName,
														response, this)) {
					setProgressBar(0, 100);
					m_ProgressBar->setVisible(false);
					m_TimePointStatusLabel->setText(
						"❌ Failed to load increment operation data");

					// 也要更新increment operation信息显示，移除加载状态
					updateIncrementOperationInfo(m_CurrentSelectedIncrement,
												 false);

					updateStatusBar("Failed to load increment operation",
									StatusType::Error);
					return;
				}

				// 更新进度到70%
				setProgressBar(70, 100);

				// 使用另一个定时器来处理数据解析，继续保持UI响应
				QTimer::singleShot(50, this, [this, response]() {
					setProgressBar(85, 100);

					// 收集所有带有UpdateNParent操作的时间点
					int totalOperations = response.swcincrementoperationlist()
											  .swcincrementoperation_size();
					int processedOperations = 0;

					for (const auto &operation :
						 response.swcincrementoperationlist()
							 .swcincrementoperation()) {
						if (operation.incrementoperation() ==
							proto::UpdateNParent) {
							m_CurrentTimePoints.push_back(operation);
						}
						processedOperations++;

						// 每处理100个操作更新一次进度条（避免过于频繁的UI更新）
						if (processedOperations % 100 == 0 ||
							processedOperations == totalOperations) {
							int progress = 85 + (10 * processedOperations /
												 totalOperations);
							setProgressBar(progress, 100);
							QApplication::processEvents();	// 让UI保持响应
						}
					}

					// 完成数据处理，更新显示
					setProgressBar(100, 100);

					// 最后一步：更新显示
					QTimer::singleShot(100, this, [this]() {
						updateTimePointDisplay();
						setProgressBar(0, 100);
						m_ProgressBar->setVisible(false);

						// 更新increment operation信息显示，移除加载状态
						updateIncrementOperationInfo(m_CurrentSelectedIncrement,
													 false);

						updateStatusBar(QString("✅ Loaded %1 time points for "
												"increment operation")
											.arg(m_CurrentTimePoints.size()),
										StatusType::Success);
					});
				});
			});
	});
}

void EditorSwcVersionControl::updateTimePointDisplay() {
	m_TimePointList->clear();

	if (m_CurrentTimePoints.empty()) {
		m_TimePointStatusLabel->setText(
			"ℹ️ No recoverable time points found in this increment operation");
		m_RevertToTimePointBtn->setEnabled(false);
		m_ExportTimePointBtn->setEnabled(false);
		m_VisualizeTimePointBtn->setEnabled(false);
		return;
	}

	m_TimePointStatusLabel->setText(
		QString("📋 Found %1 recoverable time points "
				"based on UpdateNParent operations")
			.arg(m_CurrentTimePoints.size()));

	// 按时间排序
	std::sort(m_CurrentTimePoints.begin(), m_CurrentTimePoints.end(),
			  [](const proto::SwcIncrementOperationV1 &a,
				 const proto::SwcIncrementOperationV1 &b) {
				  if (a.createtime().seconds() != b.createtime().seconds()) {
					  return a.createtime().seconds() <
							 b.createtime().seconds();
				  }
				  return a.createtime().nanos() < b.createtime().nanos();
			  });

	// 添加时间点到列表
	for (size_t i = 0; i < m_CurrentTimePoints.size(); ++i) {
		const auto &timePoint = m_CurrentTimePoints[i];
		QString itemText =
			QString("⏰ Time Point %1\n📅 %2\n🔄 Nodes Updated: %3")
				.arg(i + 1)
				.arg(formatDateTime(timePoint.createtime().seconds()))
				.arg(timePoint.nodenparent_size());

		auto *item = new QListWidgetItem(itemText);
		item->setData(Qt::UserRole, static_cast<int>(i));  // 存储索引

		// 为列表项添加一些样式
		item->setToolTip(
			QString("Created at: %1\nOperation count: %2 nodes updated")
				.arg(formatDateTime(timePoint.createtime().seconds()))
				.arg(timePoint.nodenparent_size()));

		m_TimePointList->addItem(item);
	}

	// 启用操作按钮
	m_RevertToTimePointBtn->setEnabled(false);	 // 需要选择才能启用
	m_ExportTimePointBtn->setEnabled(false);	 // 需要选择才能启用
	m_VisualizeTimePointBtn->setEnabled(false);	 // 需要选择才能启用
}

void EditorSwcVersionControl::onTimePointSelected() {
	auto selectedItems = m_TimePointList->selectedItems();
	bool hasSelection = !selectedItems.isEmpty();

	m_RevertToTimePointBtn->setEnabled(hasSelection);
	m_ExportTimePointBtn->setEnabled(hasSelection);
	m_VisualizeTimePointBtn->setEnabled(hasSelection);

	if (hasSelection) {
		int index = selectedItems[0]->data(Qt::UserRole).toInt();
		const auto &timePoint = m_CurrentTimePoints[index];
		updateStatusBar(
			QString("Selected time point: %1")
				.arg(formatDateTime(timePoint.createtime().seconds())));
	}
}

void EditorSwcVersionControl::onRevertToTimePoint() {
	auto selectedItems = m_TimePointList->selectedItems();
	if (selectedItems.isEmpty()) {
		QMessageBox::information(this, "No Selection",
								 "Please select a time point to revert to.");
		return;
	}

	int index = selectedItems[0]->data(Qt::UserRole).toInt();
	const auto &timePoint = m_CurrentTimePoints[index];
	int64_t targetTime = timePoint.createtime().seconds();

	auto result = QMessageBox::question(
		this, "Confirm Revert",
		QString("Are you sure you want to revert this SWC to the selected time "
				"point?\n\n"
				"Target Time: %1\n"
				"Operations affected: %2 node updates\n\n"
				"⚠️ WARNING: You may lose all changes after that time!\n"
				"Please ensure you have exported a backup before continuing.")
			.arg(formatDateTime(targetTime))
			.arg(timePoint.nodenparent_size()),
		QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

	if (result != QMessageBox::Yes) {
		return;
	}

	// 设置时间选择器并执行回退
	m_EndTimeEdit->setDateTime(QDateTime::fromSecsSinceEpoch(targetTime));
	onRevertVersion();
}

void EditorSwcVersionControl::onExportTimePoint() {
	auto selectedItems = m_TimePointList->selectedItems();
	if (selectedItems.isEmpty()) {
		QMessageBox::information(this, "No Selection",
								 "Please select a time point to export.");
		return;
	}

	int index = selectedItems[0]->data(Qt::UserRole).toInt();
	const auto &timePoint = m_CurrentTimePoints[index];
	int64_t targetTime = timePoint.createtime().seconds();

	auto result = QMessageBox::question(
		this, "Confirm Export",
		QString("Export SWC data at the selected time point?\n\n"
				"Target Time: %1\n"
				"Operations included: %2 node updates")
			.arg(formatDateTime(targetTime))
			.arg(timePoint.nodenparent_size()),
		QMessageBox::Yes | QMessageBox::No);

	if (result != QMessageBox::Yes) {
		return;
	}

	exportTimePointData(targetTime);
}

void EditorSwcVersionControl::exportTimePointData(int64_t timePoint) {
	updateStatusBar("Exporting time point data...", StatusType::Working);
	setProgressBar(30, 100);

	// 设置时间选择器并导出
	m_EndTimeEdit->setDateTime(QDateTime::fromSecsSinceEpoch(timePoint));

	setProgressBar(0, 100);
	m_ProgressBar->setVisible(false);

	// 调用现有的导出函数
	onExportCurrentVersion();
}

void EditorSwcVersionControl::updateIncrementOperationInfo(
	const proto::SwcIncrementOperationMetaInfoV1 &increment, bool isLoading) {
	// 显示增量操作详细信息
	QString loadingInfo =
		isLoading ? "<br><br><strong>⏳ Loading Time Points:</strong><br>"
					"Please wait while we load the recoverable time points..."
				  : "<br><br><strong>✅ Time Points Loaded:</strong><br>"
					"Time points are now available in the panel below.";

	QString info = QString(
					   "<h3 style='color: #FF9800; margin-bottom: 10px;'>⚡ "
					   "Increment Operation</h3>"
					   "<table style='width: 100%; border-spacing: 8px;'>"
					   "<tr><td style='font-weight: bold; color: "
					   "#333;'>Collection Name:</td>"
					   "<td style='font-family: monospace; background: "
					   "#f5f5f5; "
					   "padding: 2px 4px;'>%1</td></tr>"
					   "<tr><td style='font-weight: bold; color: #333;'>Start "
					   "Snapshot:</td>"
					   "<td style='font-family: monospace;'>%2</td></tr>"
					   "<tr><td style='font-weight: bold; color: "
					   "#333;'>Created "
					   "Time:</td>"
					   "<td>%3</td></tr>"
					   "<tr><td style='font-weight: bold; color: "
					   "#333;'>Type:</td>"
					   "<td style='color: #FF9800; font-weight: "
					   "bold;'>Increment "
					   "Operation</td></tr>"
					   "</table>"
					   "<p style='margin-top: 15px; padding: 8px; background: "
					   "#fff3e0; border-left: 3px solid #FF9800;'>"
					   "<strong>ℹ️ Information:</strong><br>"
					   "This node represents incremental changes between "
					   "snapshots.<br>"
					   "It contains the operations that transform one "
					   "snapshot "
					   "into another.%4"
					   "</p>")
					   .arg(QString::fromStdString(
						   increment.incrementoperationcollectionname()))
					   .arg(QString::fromStdString(increment.startsnapshot()))
					   .arg(formatDateTime(increment.createtime().seconds()))
					   .arg(loadingInfo);

	m_VersionInfoText->setHtml(info);
}

void EditorSwcVersionControl::onVisualizeTimePoint() {
	auto selectedItems = m_TimePointList->selectedItems();
	if (selectedItems.isEmpty()) {
		QMessageBox::information(this, "No Selection",
								 "Please select a time point to visualize.");
		return;
	}

	int index = selectedItems[0]->data(Qt::UserRole).toInt();
	const auto &timePoint = m_CurrentTimePoints[index];
	int64_t targetTime = timePoint.createtime().seconds();

	auto result = QMessageBox::question(
		this, "Confirm Visualization",
		QString("Visualize SWC data at time point: %1?\n\n"
				"Operations included: %2 node updates")

			.arg(formatDateTime(targetTime))
			.arg(timePoint.nodenparent_size()),
		QMessageBox::Yes | QMessageBox::No);

	if (result != QMessageBox::Yes) {
		return;
	}

	updateStatusBar("Generating SWC data for visualization...",
					StatusType::Working);
	setProgressBar(20, 100);
	m_ProgressBar->setVisible(true);

	// 获取基础快照数据
	proto::GetSnapshotResponse snapResponse;
	if (!WrappedCall::getSwcSnapshot(m_CurrentSelectedIncrement.startsnapshot(),
									 snapResponse, this)) {
		setProgressBar(0, 100);
		m_ProgressBar->setVisible(false);
		updateStatusBar("Failed to load base snapshot data", StatusType::Error);
		return;
	}

	setProgressBar(60, 100);

	// 获取增量操作数据
	proto::GetIncrementOperationResponse incrementResponse;
	if (!WrappedCall::getSwcIncrementRecord(m_CurrentIncrementOpName,
											incrementResponse, this)) {
		setProgressBar(0, 100);
		m_ProgressBar->setVisible(false);
		updateStatusBar("Failed to load increment operation data",
						StatusType::Error);
		return;
	}

	setProgressBar(80, 100);

	// 应用到指定时间点的所有操作
	std::vector<proto::SwcNodeDataV1> nodeData;
	for (auto &node : snapResponse.swcnodedata().swcdata()) {
		nodeData.push_back(node);
	}

	for (const auto &op : incrementResponse.swcincrementoperationlist()
							  .swcincrementoperation()) {
		// 只应用在目标时间点之前或等于的操作
		if (op.createtime().seconds() <= targetTime) {
			promoteOperation(nodeData, op, targetTime);
		}
	}

	setProgressBar(100, 100);
	m_ProgressBar->setVisible(false);
	updateStatusBar("Opening visualization...", StatusType::Success);

	// 创建渲染器并显示
	SwcRendererCreateInfo createInfo;
	proto::SwcDataV1 swcData;
	for (const auto &node : nodeData) {
		*swcData.add_swcdata() = node;
	}
	createInfo.swcData = swcData;
	auto *renderer = new SwcRendererDailog(createInfo);
	renderer->setAttribute(Qt::WA_DeleteOnClose);
	renderer->exec();

	updateStatusBar("Visualization completed successfully",
					StatusType::Success);
}

void EditorSwcVersionControl::onRevertToSnapshot() {
	// 回退到选中的快照
	if (!m_DataFlowGraphicsScene) {
		return;
	}

	auto selectedNodes = m_DataFlowGraphicsScene->selectedNodes();
	if (selectedNodes.empty()) {
		QMessageBox::information(this, "No Selection",
								 "Please select a snapshot to revert to.");
		return;
	}

	auto selectedNode = selectedNodes[0];
	auto internalData =
		m_DataFlowGraphModel
			.nodeData(selectedNode, QtNodes::NodeRole::InternalData)
			.value<std::any>();

	QString snapShotCName;
	proto::SwcSnapshotMetaInfoV1 snapshot;

	if (internalData.has_value()) {
		// 检查是否为快照节点（而非增量操作）
		try {
			snapshot =
				std::any_cast<proto::SwcSnapshotMetaInfoV1>(internalData);
			snapShotCName =
				QString::fromStdString(snapshot.swcsnapshotcollectionname());
		} catch (const std::bad_any_cast &e) {
			QMessageBox::warning(this, "Invalid Selection",
								 "Please select a valid snapshot node (not an "
								 "increment operation).");
			return;
		}
	}

	// 获取快照的创建时间
	int64_t snapshotTime = snapshot.createtime().seconds();
	QString timeStr = formatDateTime(snapshotTime);

	auto result = QMessageBox::question(
		this, "Confirm Revert to Snapshot",
		QString("Are you sure you want to revert this SWC to the selected "
				"snapshot?\n\n"
				"Target Snapshot: %1\n"
				"Created Time: %2\n\n"
				"⚠️ WARNING: You may lose all changes after that snapshot!\n"
				"Please ensure you have exported a backup before continuing.")
			.arg(snapShotCName)
			.arg(timeStr),
		QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

	if (result != QMessageBox::Yes) {
		return;
	}

	if (!m_HasExportedStatus) {
		QMessageBox::warning(
			this, "Backup Required",
			"Please export a backup before reverting this SWC!");
		return;
	}

	updateStatusBar("Reverting to snapshot...", StatusType::Working);
	setProgressBar(50, 100);

	proto::RevertSwcVersionResponse response;
	google::protobuf::Timestamp endTime;
	endTime.set_seconds(snapshotTime);

	if (!WrappedCall::RevertSwcVersionByUuid(m_SwcUuid, endTime, response,
											 this)) {
		setProgressBar(0, 100);
		m_ProgressBar->setVisible(false);
		updateStatusBar("Failed to revert to snapshot", StatusType::Error);
		return;
	}

	setProgressBar(0, 100);
	m_ProgressBar->setVisible(false);
	updateStatusBar("Successfully reverted to snapshot!", StatusType::Success);

	QMessageBox::information(
		this, "Success",
		QString("SWC successfully reverted to snapshot: %1")
			.arg(snapShotCName));

	// 刷新版本图以显示最新状态
	refreshVersionGraph();
}
