#include "MainWindow.h"

#include <Message/Response.pb.h>
#include <qdesktopservices.h>
#include <qurl.h>

#include <QFileDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QTimer>

#include "About.h"
#include "EditorAdminUserManager.h"
#include "EditorPermissionGroup.h"
#include "Renderer/SwcRenderer.h"
#include "ViewImportSwcFromFile.h"
#include "src/framework/defination/ImageDefination.h"
#include "src/framework/service/CachedProtoData.h"
#include "src/framework/service/WrappedCall.h"
#include "version.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
	setWindowIcon(QIcon(Image::ImageAppIcon));
	setWindowState(Qt::WindowMaximized);

	QString appTitle =
		QString("SWC DBMS Qt Client") +
		QString(" - Version: %1")
			.arg(QString::fromStdString(version::VERSION_STRING));

	setWindowTitle(appTitle);

	m_Splitter = new QSplitter(this);
	m_Splitter->setHandleWidth(4);

	m_LeftClientView = new LeftClientView(this);
	m_RightClientView = new RightClientView(this);

	m_Splitter->addWidget(m_LeftClientView);
	m_Splitter->addWidget(m_RightClientView);
	m_Splitter->setSizes(QList<int>() << 1 << 4);
	m_Splitter->setCollapsible(0, false);
	m_Splitter->setCollapsible(1, false);

	this->setCentralWidget(m_Splitter);

	auto menuBar = new QMenuBar(this);

	auto *menuFile = new QMenu(menuBar);
	menuFile->setTitle("File");
	menuFile->setIcon(QIcon(Image::ImageFileOpen));
	menuBar->addMenu(menuFile);

	auto *menuImportSwcFile = new QAction(menuFile);
	menuImportSwcFile->setText("Import Swc File");
	menuImportSwcFile->setIcon(QIcon(Image::ImageImport));
	menuFile->addAction(menuImportSwcFile);
	connect(menuImportSwcFile, &QAction::triggered, this, [&]() {
		ViewImportSwcFromFile view(this, "");
		view.exec();
		m_LeftClientView->refreshTree();
	});

	auto *menuTools = new QMenu(menuBar);
	menuTools->setTitle("Tools");
	menuTools->setIcon(QIcon(Image::ImageTool));
	menuBar->addMenu(menuTools);

	auto *menuManagePermissionGroup = new QAction(menuTools);
	menuManagePermissionGroup->setText("Manage Permission Group");
	menuManagePermissionGroup->setIcon(QIcon(Image::ImageUserPermission));
	menuTools->addAction(menuManagePermissionGroup);
	connect(menuManagePermissionGroup, &QAction::triggered, this, [&]() {
		EditorPermissionGroup view(this);
		view.exec();
	});

	auto *menuManageUser = new QAction(menuTools);
	menuManageUser->setText("Manage User");
	menuManageUser->setIcon(QIcon(Image::ImageUser));
	menuTools->addAction(menuManageUser);
	connect(menuManageUser, &QAction::triggered, this, [&]() {
		EditorAdminUserManager view(this);
		view.exec();
	});

	menuTools->addSeparator();

	auto *menuSwcFeature = new QAction(menuTools);
	menuSwcFeature->setText("SWC Feature (Local File)");
	menuSwcFeature->setIcon(QIcon(Image::ImageTool));
	menuTools->addAction(menuSwcFeature);
	connect(menuSwcFeature, &QAction::triggered, this, [this]() {
		auto filePath = QFileDialog::getOpenFileName(
			this, "Open SWC File", QString(), "SWC Files (*.swc *.SWC)");
		if (!filePath.isEmpty()) {
			m_RightClientView->openSwcFeatureLocalFile(filePath);
		}
	});

	auto *menuQualityControl = new QAction(menuTools);
	menuQualityControl->setText("Quality Control (Local File)");
	menuQualityControl->setIcon(QIcon(Image::ImageTool));
	menuTools->addAction(menuQualityControl);
	connect(menuQualityControl, &QAction::triggered, this, [this]() {
		auto filePath = QFileDialog::getOpenFileName(
			this, "Open SWC File", QString(), "SWC Files (*.swc *.SWC)");
		if (!filePath.isEmpty()) {
			m_RightClientView->openQualityControlLocalFile(filePath);
		}
	});

	auto *menuCompareSwc = new QAction(menuTools);
	menuCompareSwc->setText("Compare SWC Files (Local)");
	menuCompareSwc->setIcon(QIcon(Image::ImageTool));
	menuTools->addAction(menuCompareSwc);
	connect(menuCompareSwc, &QAction::triggered, this, [this]() {
		auto oldPath = QFileDialog::getOpenFileName(
			this, "Open Old SWC File", QString(),
			"SWC Files (*.swc *.eswc *.SWC *.ESWC)");
		if (oldPath.isEmpty()) return;

		auto newPath = QFileDialog::getOpenFileName(
			this, "Open New SWC File", QString(),
			"SWC Files (*.swc *.eswc *.SWC *.ESWC)");
		if (newPath.isEmpty()) return;

		SwcRendererCreateInfo createInfo;
		createInfo.mode = SwcRendererMode::eVisualizeDiffSwc;
		createInfo.dataSource = DataSource::eLoadFromFile;
		createInfo.swcPath = oldPath.toStdString();
		createInfo.newSwcPath = newPath.toStdString();
		createInfo.markerLegend.push_back(
			{"Added branches", {0.0f, 0.8f, 0.2f}});
		createInfo.markerLegend.push_back(
			{"Deleted branches", {1.0f, 0.2f, 0.2f}});
		createInfo.markerLegend.push_back(
			{"Modified attributes", {1.0f, 0.7f, 0.0f}});
		createInfo.markerLegend.push_back(
			{"Unchanged", {0.3f, 0.4f, 0.6f}});

		auto *renderer = new SwcRendererDailog(createInfo);
		renderer->setAttribute(Qt::WA_DeleteOnClose);
		renderer->exec();
	});

	auto *menuHelp = new QMenu(menuBar);
	menuHelp->setTitle("Help");
	menuHelp->setIcon(QIcon(Image::ImageHelp));
	menuBar->addMenu(menuHelp);

	auto *menuAbout = new QAction(menuHelp);
	menuAbout->setText("About");
	menuAbout->setIcon(QIcon(Image::ImageHelp));
	menuHelp->addAction(menuAbout);
	connect(menuAbout, &QAction::triggered, this, [&]() {
		About view(this);
		view.exec();
	});

	auto *menuOnlineHelp = new QAction(menuHelp);
	menuOnlineHelp->setText("Online Help");
	menuOnlineHelp->setIcon(QIcon(Image::ImageHelp));
	menuHelp->addAction(menuOnlineHelp);
	connect(menuOnlineHelp, &QAction::triggered, this, [&]() {
		QDesktopServices::openUrl(
			QUrl("https://github.com/Hi5App/SwcDbmsQtClient"));
	});

	setMenuBar(menuBar);

	m_HeartBeatTimer = new QTimer;
	m_HeartBeatTimer->setInterval(30000);
	connect(m_HeartBeatTimer, &QTimer::timeout, this, [this]() {
		proto::UserOnlineHeartBeatResponse response;
		if (WrappedCall::UserOnlineHeartBeat(response, this)) {
			CachedProtoData::getInstance().UserName =
				response.userverifyinfo().username();
			CachedProtoData::getInstance().UserToken =
				response.userverifyinfo().usertoken();
			CachedProtoData::getInstance().OnlineStatus = true;
		} else {
			CachedProtoData::getInstance().OnlineStatus = false;
		}
	});
	m_HeartBeatTimer->start();

	m_OnlineStatusTimer = new QTimer;
	m_OnlineStatusTimer->setInterval(60000);
	connect(m_OnlineStatusTimer, &QTimer::timeout, this, [this]() {
		if (!CachedProtoData::getInstance().OnlineStatus) {
			QMessageBox::critical(
				this, "Error",
				"Timeout! You may have disconnected from server!");
		}
		CachedProtoData::getInstance().OnlineStatus = false;
	});
	m_OnlineStatusTimer->start();
}

MainWindow::~MainWindow() = default;

LeftClientView &MainWindow::getLeftClientView() { return *m_LeftClientView; }

RightClientView &MainWindow::getRightClientView() { return *m_RightClientView; }
