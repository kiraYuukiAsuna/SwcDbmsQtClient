#include "MainWindow.h"

#include <Message/Request.pb.h>
#include <Message/Response.pb.h>
#include <grpcpp/client_context.h>
#include <qdesktopservices.h>
#include <qurl.h>

#include <QMessageBox>
#include <QTimer>

#include "About.h"
#include "EditorAdminUserManager.h"
#include "EditorPermissionGroup.h"
#include "ViewImportSwcFromFile.h"
#include "src/framework/defination/ImageDefination.h"
#include "src/framework/service/CachedProtoData.h"
#include "src/framework/service/RpcCall.h"
#include "ui_MainWindow.h"
#include "version.h"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent), ui(new Ui::MainWindow) {
	ui->setupUi(this);
	setWindowIcon(QIcon(Image::ImageAppIcon));
	setWindowState(Qt::WindowMaximized);

	QString appTitle =
		QString("SWC DBMS Qt Client") +
		QString(" - Version: %1")
			.arg(QString::fromStdString(version::VERSION_STRING)) +
		QString(" - Git Commit: %1")
			.arg(QString::fromStdString(version::GIT_COMMIT_HASH)) +
		QString(" - Git Branch: %1")
			.arg(QString::fromStdString(version::GIT_BRANCH)) +
		QString(" - Git Commit Date: %1")
			.arg(QString::fromStdString(version::GIT_COMMIT_DATE));

	setWindowTitle(appTitle);

	m_Splitter = new QSplitter(this);

	m_LeftClientView = new LeftClientView(this);
	m_RightClientView = new RightClientView(this);

	m_Splitter->addWidget(m_LeftClientView);
	m_Splitter->addWidget(m_RightClientView);
	m_Splitter->setSizes(QList<int>() << 100000000 << 400000000);
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
		proto::UserOnlineHeartBeatNotification notification;
		notification.mutable_metainfo()->set_apiversion(RpcCall::ApiVersion);
		auto *userInfo = notification.mutable_userverifyinfo();
		userInfo->set_username(CachedProtoData::getInstance().UserName);
		userInfo->set_usertoken(CachedProtoData::getInstance().UserToken);
		notification.set_heartbeattime(
			std::chrono::system_clock::now().time_since_epoch().count());
		proto::UserOnlineHeartBeatResponse response;
		grpc::ClientContext context;
		auto status =
			RpcCall::getInstance().Stub()->UserOnlineHeartBeatNotifications(
				&context, notification, &response);
		if (status.ok()) {
			CachedProtoData::getInstance().UserName =
				response.userverifyinfo().username();
			CachedProtoData::getInstance().UserToken =
				response.userverifyinfo().usertoken();
			CachedProtoData::getInstance().OnlineStatus = true;
		} else {
			CachedProtoData::getInstance().OnlineStatus = false;
			QMessageBox::critical(
				this, "Error", QString::fromStdString(status.error_message()));
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

MainWindow::~MainWindow() { delete ui; }

LeftClientView &MainWindow::getLeftClientView() { return *m_LeftClientView; }

RightClientView &MainWindow::getRightClientView() { return *m_RightClientView; }
