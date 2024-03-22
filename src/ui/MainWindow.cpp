#include <QMessageBox>
#include <QTimer>

#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "src/framework/defination/ImageDefination.h"
#include <QTimer>
#include <grpcpp/client_context.h>
#include <Message/Request.pb.h>
#include <Message/Response.pb.h>

#include "ViewImportSwcFromFile.h"
#include "src/framework/service/CachedProtoData.h"
#include "src/framework/service/RpcCall.h"

MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    setWindowIcon(QIcon(Image::ImageAppIcon));
    setWindowState(Qt::WindowMaximized);

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
        ViewImportSwcFromFile view(this);
        view.exec();
        m_LeftClientView->refreshTree();
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

    });

    auto *menuOnlineHelp = new QAction(menuHelp);
    menuOnlineHelp->setText("Online Help");
    menuOnlineHelp->setIcon(QIcon(Image::ImageHelp));
    menuHelp->addAction(menuOnlineHelp);
    connect(menuOnlineHelp, &QAction::triggered, this, [&]() {

    });

    setMenuBar(menuBar);

    m_HeartBeatTimer = new QTimer;
    m_HeartBeatTimer->setInterval(15000);
    connect(m_HeartBeatTimer, &QTimer::timeout, this, [this]() {
        proto::UserOnlineHeartBeatNotification notification;
        notification.mutable_metainfo()->set_apiversion(RpcCall::ApiVersion);
        auto *userInfo = notification.mutable_userverifyinfo();
        userInfo->set_username(CachedProtoData::getInstance().UserName);
        userInfo->set_usertoken(CachedProtoData::getInstance().UserToken);
        notification.set_heartbeattime(std::chrono::system_clock::now().time_since_epoch().count());
        proto::UserOnlineHeartBeatResponse response;
        grpc::ClientContext context;
        auto status = RpcCall::getInstance().Stub()->UserOnlineHeartBeatNotifications(&context, notification,
                                                                                      &response);
        if (status.ok()) {
            CachedProtoData::getInstance().UserName = response.userverifyinfo().username();
            CachedProtoData::getInstance().UserToken = response.userverifyinfo().usertoken();
            CachedProtoData::getInstance().OnlineStatus = true;
        } else {
            QMessageBox::critical(this, "Error", QString::fromStdString(status.error_message()));
        }
    });
    m_HeartBeatTimer->start();

    m_OnlineStatusTimer = new QTimer;
    m_OnlineStatusTimer->setInterval(30000);
    connect(m_OnlineStatusTimer, &QTimer::timeout, this, [this]() {
        if (!CachedProtoData::getInstance().OnlineStatus) {
            QMessageBox::critical(this, "Error", "Timeout! You may have disconnected from server!");
        }
        CachedProtoData::getInstance().OnlineStatus = false;
    });
    m_OnlineStatusTimer->start();
}

MainWindow::~MainWindow() {
    delete ui;
}

LeftClientView &MainWindow::getLeftClientView() {
    return *m_LeftClientView;
}

RightClientView &MainWindow::getRightClientView() {
    return *m_RightClientView;
}
