#include "leftclientview.h"

#include <QMessageBox>

#include "MainWindow.h"
#include "MainWindow.h"
#include "mainwindow.h"
#include "ui_LeftClientView.h"
#include "src/framework/service/CachedProtoData.h"
#include "src/framework/service/RpcCall.h"
#include "src/framework/defination/TypeDef.h"
#include "src/framework/defination/ImageDefination.h"
#include "src/framework/service/WrappedCall.h"
#include "src/ui/editorusersettings.h"
#include <QMenu>
#include <ui_ViewCreateSwc.h>

#include "viewcreateproject.h"
#include "viewcreateswc.h"
#include "src/framework/config/AppConfig.h"
#include "viewimportswcfromfile.h"
#include "vieweportswctofile.h"

LeftClientView::LeftClientView(MainWindow* mainWindow) : QWidget(mainWindow), ui(new Ui::LeftClientView) {
    ui->setupUi(this);
    m_MainWindow = mainWindow;

    m_ControlBtnLayout = new QHBoxLayout;

    m_UserSettingBtn = new QPushButton(this);
    m_UserSettingBtn->setText("User Settings");
    m_UserSettingBtn->setIcon(QIcon(Image::ImageUser));

    auto rawHeadPhotoBinData = CachedProtoData::getInstance().CachedUserMetaInfo.headphotobindata();
    auto headPhotoBinData = QByteArray::fromStdString(rawHeadPhotoBinData);

    connect(m_UserSettingBtn, &QPushButton::clicked, this, [this](bool checked) {
        EditorUserSettings editorUserSettings(this);
        editorUserSettings.exec();
    });

    m_AccountBtn = new QToolButton(this);
    m_AccountBtn->setPopupMode(QToolButton::InstantPopup);
    m_AccountBtn->setText("Account");
    m_AccountBtn->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_AccountBtn->setIconSize({32,32});

    if(headPhotoBinData.size() > 0) {
        QPixmap pixmap;
        pixmap.loadFromData(headPhotoBinData);
        pixmap = pixmap.scaled(QSize(128,128),Qt::KeepAspectRatioByExpanding);
        m_AccountBtn->setIcon(pixmap);
    }else {
        m_AccountBtn->setIcon(QIcon(Image::ImageDefaultUserHeadPhoto));
    }

    auto* logoutAction = new QAction(this);
    logoutAction->setText("Logout");
    connect(logoutAction, &QAction::triggered, this, [this](bool checked) {
        auto btn = QMessageBox::information(this, "Warning",
                                            "Are you sure to logout? Please save all your work before logout!",
                                            QMessageBox::StandardButton::Ok,
                                            QMessageBox::StandardButton::Cancel);
        if (btn == QMessageBox::StandardButton::Ok) {
            proto::UserLogoutRequest request;
            request.mutable_userinfo()->CopyFrom(CachedProtoData::getInstance().CachedUserMetaInfo);
            proto::UserLogoutResponse response;

            grpc::ClientContext context;
            auto status = RpcCall::getInstance().Stub()->UserLogout(&context,request,&response);
            if(status.ok()) {
                if(response.status()) {
                    AppConfig::getInstance().setSecurityConfig(AppConfig::SecurityConfigItem::eCachedUserName, "");
                    AppConfig::getInstance().setSecurityConfig(AppConfig::SecurityConfigItem::eCachedPassword, "");
                    AppConfig::getInstance().setSecurityConfig(AppConfig::SecurityConfigItem::eAccountExpiredTime, "");
                    AppConfig::getInstance().writeSecurityConfig();

                    qApp->exit(RestartCode);
                }
                QMessageBox::critical(this,"Error",QString::fromStdString(response.message()));
            }
            QMessageBox::critical(this,"Error",QString::fromStdString(status.error_message()));
        }
    });
    m_AccountBtn->addAction(logoutAction);

    m_RefreshBtn = new QPushButton(this);
    m_RefreshBtn->setText("Refresh");
    m_RefreshBtn->setIcon(QIcon(Image::ImageRefresh));
    connect(m_RefreshBtn, &QPushButton::clicked, this, &LeftClientView::onRefreshBtnClicked);

    m_ControlBtnLayout->addWidget(m_AccountBtn);
    m_ControlBtnLayout->addWidget(m_UserSettingBtn);
    m_ControlBtnLayout->addWidget(m_RefreshBtn);

    m_TreeWidget = new QTreeWidget(this);
    m_TreeWidget->setHeaderLabel("MetaInfo");
    connect(m_TreeWidget, &QTreeWidget::itemDoubleClicked, this, [this](QTreeWidgetItem* item, int column) {
        if (column == 0) {
            if (item) {
                auto metaInfo = item->data(0, Qt::UserRole).value<LeftClientViewTreeWidgetItemMetaInfo>();
                switch (metaInfo.type) {
                    case MetaInfoType::eProjectContainer:
                        break;
                    case MetaInfoType::eProject: {
                        m_MainWindow->getRightClientView().openProjectMetaInfo(metaInfo.name);
                        break;
                    }
                    case MetaInfoType::eSwcContainer:
                        break;
                    case MetaInfoType::eSwc: {
                        m_MainWindow->getRightClientView().openSwcMetaInfo(metaInfo.name);
                        break;
                    }
                    case MetaInfoType::eDailyStatisticsContainer:
                        break;
                    case MetaInfoType::eDailyStatistics: {
                        m_MainWindow->getRightClientView().openDailyStatisticsMetaInfo(metaInfo.name);
                        break;
                    }
                    case MetaInfoType::eUserMetaInfo:
                        break;
                    case MetaInfoType::ePermissionGroupMetaInfo:
                        break;
                    case MetaInfoType::eUserManagerMetaInfo:
                        break;
                    case MetaInfoType::eSwcData:
                        break;
                    case MetaInfoType::eUnknown:
                        break;
                }
            }
        }
    });

    m_TreeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_TreeWidget, &QTreeWidget::customContextMenuRequested, this, &LeftClientView::customTreeWidgetContentMenu);

    m_MainLayout = new QVBoxLayout(this);
    m_MainLayout->addLayout(m_ControlBtnLayout);
    m_MainLayout->addWidget(m_TreeWidget);
    this->setLayout(m_MainLayout);

    refreshTree();
}

LeftClientView::~LeftClientView() {
    delete ui;
}

void LeftClientView::getProjectMetaInfo() {
    proto::GetAllProjectResponse response;
    WrappedCall::getAllProjectMetaInfo(response, this);
    auto projectInfoList = response.mutable_projectinfo();
    for (int i = 0; i < projectInfoList->size(); i++) {
        auto&projectInfo = projectInfoList->Get(i);
        auto* item = new QTreeWidgetItem;
        item->setText(0, QString::fromStdString(projectInfo.name()));
        item->setIcon(0, QIcon(Image::ImageProject));
        LeftClientViewTreeWidgetItemMetaInfo metaInfo{};
        metaInfo.type = MetaInfoType::eProject;
        metaInfo.name = projectInfo.name();
        item->setData(0, Qt::UserRole, QVariant::fromValue(metaInfo));
        m_TopProjectItem->addChild(item);
    }
}

void LeftClientView::getSwcMetaInfo() {
    proto::GetAllSwcMetaInfoResponse response;
    WrappedCall::getAllSwcMetaInfo(response, this);
    auto swcMetaInfo = response.mutable_swcinfo();
    for (int i = 0; i < swcMetaInfo->size(); i++) {
        auto&swcInfo = swcMetaInfo->Get(i);
        auto* item = new QTreeWidgetItem;
        item->setText(0, QString::fromStdString(swcInfo.name()));
        item->setIcon(0, QIcon(Image::ImageNode));
        LeftClientViewTreeWidgetItemMetaInfo metaInfo{};
        metaInfo.type = MetaInfoType::eSwc;
        metaInfo.name = swcInfo.name();
        item->setData(0, Qt::UserRole, QVariant::fromValue(metaInfo));
        m_TopSwcItem->addChild(item);
    }
}

void LeftClientView::getAllDailyStatisticsMetaInfo() {
    proto::GetAllDailyStatisticsResponse response;
    WrappedCall::getAllDailyStatisticsMetaInfo(response, this);
    auto dailyStatisticsMetaInfoList = response.mutable_dailystatisticsinfo();
    for (int i = 0; i < dailyStatisticsMetaInfoList->size(); i++) {
        auto&dailyStatisticsMetaInfo = dailyStatisticsMetaInfoList->Get(i);
        auto* item = new QTreeWidgetItem;
        item->setText(0, QString::fromStdString(dailyStatisticsMetaInfo.name()));
        item->setIcon(0, QIcon(Image::ImageDaily));
        LeftClientViewTreeWidgetItemMetaInfo metaInfo{};
        metaInfo.type = MetaInfoType::eDailyStatistics;
        metaInfo.name = dailyStatisticsMetaInfo.name();
        item->setData(0, Qt::UserRole, QVariant::fromValue(metaInfo));
        m_TopDailyStatisticsItem->addChild(item);
    }
}

void LeftClientView::onRefreshBtnClicked(bool checked) {
    refreshTree();
}

void LeftClientView::customTreeWidgetContentMenu(const QPoint&pos) {
    auto* curItem = m_TreeWidget->currentItem();
    if (!curItem) {
        return;
    }

    auto data = curItem->data(0, Qt::UserRole).value<LeftClientViewTreeWidgetItemMetaInfo>();

    auto* popMenu = new QMenu(this);

    auto* MenuCreateProject = new QAction(this);
    MenuCreateProject->setText("Create Project");
    MenuCreateProject->setIcon(QIcon(Image::ImageCreate));
    connect(MenuCreateProject,&QAction::triggered,this,[this](bool checked) {
        ViewCreateProject view;
        if(view.exec() == QDialog::Accepted) {
            refreshTree();
        }
    });

    auto* MenuDeleteProject = new QAction(this);
    MenuDeleteProject->setText("Delete Project");
    MenuDeleteProject->setIcon(QIcon(Image::ImageDelete));
    connect(MenuDeleteProject,&QAction::triggered,this,[this,data,curItem](bool checked) {
        auto result = QMessageBox::information(this,"Warning","Are you sure to delete this project? This operation cannot be revert!",
            QMessageBox::StandardButton::Ok,QMessageBox::StandardButton::Cancel);
        if(result == QMessageBox::StandardButton::Ok) {
            if(data.type == MetaInfoType::eProject) {
                proto::DeleteProjectRequest request;
                request.mutable_userinfo()->CopyFrom(CachedProtoData::getInstance().CachedUserMetaInfo);
                request.mutable_projectinfo()->set_name(curItem->text(0).toStdString());

                proto::DeleteProjectResponse response;
                grpc::ClientContext context;
                auto status = RpcCall::getInstance().Stub()->DeleteProject(&context,request,&response);
                if(status.ok()) {
                    if(response.status()) {
                        QMessageBox::information(this,"Info","Delete Project successfully!");
                        m_MainWindow->getRightClientView().closeWithoutSavingProject(data.name);
                        refreshTree();
                    }else {
                        QMessageBox::critical(this,"Error",QString::fromStdString(response.message()));
                    }
                }else{
                    QMessageBox::critical(this,"Error",QString::fromStdString(status.error_message()));
                }
            }
        }
    });

    auto* MenuEditProject = new QAction(this);
    MenuEditProject->setText("Edit Project");
    MenuEditProject->setIcon(QIcon(Image::ImageEdit));
    connect(MenuEditProject,&QAction::triggered,this,[this,data](bool checked) {
        m_MainWindow->getRightClientView().openProjectMetaInfo(data.name);
    });

    auto* MenuImportSwcFile = new QAction(this);
    MenuImportSwcFile->setText("Import Swc File");
    MenuImportSwcFile->setIcon(QIcon(Image::ImageImport));
    connect(MenuImportSwcFile,&QAction::triggered,this,[this](bool checked) {
        ViewImportSwcFromFile view(m_MainWindow);
        view.exec();
        refreshTree();
    });

    auto* MenuExportToSwcFile = new QAction(this);
    MenuExportToSwcFile->setText("Export Swc File");
    MenuExportToSwcFile->setIcon(QIcon(Image::ImageExport));
    connect(MenuExportToSwcFile,&QAction::triggered,this,[this, &data](bool checked) {
        auto result = QMessageBox::information(this,"Info","Exporting action may takes few time please waiting for export steps finish!",
                                               QMessageBox::StandardButton::Ok,QMessageBox::StandardButton::Cancel);
        if(result == QMessageBox::StandardButton::Ok) {
            if(data.type == MetaInfoType::eSwc){
                proto::GetSwcMetaInfoResponse response1;
                if(!WrappedCall::getSwcMetaInfoByName(data.name,response1,this)){
                    return;
                }

                proto::GetSwcFullNodeDataResponse response2;
                if(!WrappedCall::getSwcFullNodeData(data.name, response2, this)){
                    return;
                }

                std::vector<ExportSwcData> dataList;
                ExportSwcData exportSwcData;
                exportSwcData.swcData = response2.swcnodedata();
                exportSwcData.swcMetaInfo = response1.swcinfo();
                dataList.push_back(exportSwcData);

                ViewEportSwcToFile view(dataList,false,this);
                view.exec();
            }else if(data.type == MetaInfoType::eSwcContainer){
                proto::GetAllSwcMetaInfoResponse response;
                if(!WrappedCall::getAllSwcMetaInfo(response,this)){
                    return;
                }

                std::vector<ExportSwcData> dataList;
                for(int i=0;i<response.swcinfo_size();i++){
                    ExportSwcData exportSwcData;
                    exportSwcData.swcMetaInfo = response.swcinfo(i);
                    dataList.push_back(exportSwcData);
                }

                ViewEportSwcToFile view(dataList,true,this);
                view.exec();
            }else if(data.type == MetaInfoType::eProject){
                proto::GetProjectResponse projectResponse;
                if(!WrappedCall::getProjectMetaInfoByName(data.name, projectResponse, this)){
                    return;
                }

                std::vector<proto::SwcMetaInfoV1> swcMetaInfos;

                proto::GetAllSwcMetaInfoResponse responseAllSwc;
                WrappedCall::getAllSwcMetaInfo(responseAllSwc, this);
                for (int i = 0; i < responseAllSwc.swcinfo_size(); i++) {
                    auto& swcInfo = responseAllSwc.swcinfo(i);
                    bool bFind = false;
                    for (int j = 0; j < projectResponse.projectinfo().swclist().size(); j++) {
                        auto name = projectResponse.projectinfo().swclist(j);
                        if (name == swcInfo.name()) {
                            bFind = true;
                        }
                    }
                    if(bFind){
                        swcMetaInfos.push_back(responseAllSwc.swcinfo(i));
                    }
                }

                std::vector<ExportSwcData> dataList;
                for(const auto & swcMetaInfo : swcMetaInfos){
                    ExportSwcData exportSwcData;
                    exportSwcData.swcMetaInfo = swcMetaInfo;
                    dataList.push_back(exportSwcData);
                }

                ViewEportSwcToFile view(dataList,true,this);
                view.exec();
            }
        }
    });

    auto* MenuCreateSwc = new QAction(this);
    MenuCreateSwc->setText("Create Swc");
    MenuCreateSwc->setIcon(QIcon(Image::ImageCreate));
    connect(MenuCreateSwc,&QAction::triggered,this,[this](bool checked) {
        ViewCreateSwc view;
        if(view.exec() == QDialog::Accepted) {
            refreshTree();
        }
    });

    auto* MenuDeleteSwc = new QAction(this);
    MenuDeleteSwc->setText("Delete Swc");
    MenuDeleteSwc->setIcon(QIcon(Image::ImageDelete));
    connect(MenuDeleteSwc,&QAction::triggered,this,[this,data,curItem](bool checked) {
        auto result = QMessageBox::information(this,"Warning","Are you sure to delete this Swc? This operation cannot be revert!",
                   QMessageBox::StandardButton::Ok,QMessageBox::StandardButton::Cancel);
               if(result == QMessageBox::StandardButton::Ok) {
                   if(data.type == MetaInfoType::eSwc) {
                       proto::DeleteSwcRequest request;
                       request.mutable_userinfo()->CopyFrom(CachedProtoData::getInstance().CachedUserMetaInfo);
                       request.mutable_swcinfo()->set_name(curItem->text(0).toStdString());

                       proto::DeleteSwcResponse response;
                       grpc::ClientContext context;
                       auto status = RpcCall::getInstance().Stub()->DeleteSwc(&context,request,&response);
                       if(status.ok()) {
                           if(response.status()) {
                               QMessageBox::information(this,"Info","Delete Swc successfully!");
                               m_MainWindow->getRightClientView().closeWithoutSavingSwc(data.name);
                               m_MainWindow->getRightClientView().refreshAllOpenedProjectMetaInfo();
                               refreshTree();
                           }else {
                               QMessageBox::critical(this,"Error",QString::fromStdString(response.message()));
                           }
                       }else{
                           QMessageBox::critical(this,"Error",QString::fromStdString(status.error_message()));
                       }
                   }
               }
    });

    auto* MenuEditSwc = new QAction(this);
    MenuEditSwc->setText("Edit Swc");
    MenuEditSwc->setIcon(QIcon(Image::ImageEdit));
    connect(MenuEditSwc,&QAction::triggered,this,[this,data](bool checked) {
        m_MainWindow->getRightClientView().openSwcMetaInfo(data.name);
    });

    auto* MenuEditSwcNodeData = new QAction(this);
    MenuEditSwcNodeData->setText("Edit SwcNodeData");
    MenuEditSwcNodeData->setIcon(QIcon(Image::ImageEdit));
    connect(MenuEditSwcNodeData,&QAction::triggered,this,[this,data](bool checked) {
        m_MainWindow->getRightClientView().openSwcNodeData(data.name);
    });

    auto* MenuDeleteDailyStatistics = new QAction(this);
    MenuDeleteDailyStatistics->setText("Delete DailyStatistics");
    MenuDeleteDailyStatistics->setIcon(QIcon(Image::ImageDelete));
    connect(MenuDeleteDailyStatistics,&QAction::triggered,this,[this,data,curItem](bool checked) {
        auto result = QMessageBox::information(this,"Warning","Are you sure to delete this DailyStatistics? This operation cannot be revert!",
                  QMessageBox::StandardButton::Ok,QMessageBox::StandardButton::Cancel);
              if(result == QMessageBox::StandardButton::Ok) {
                  if(data.type == MetaInfoType::eDailyStatistics) {
                      proto::DeleteDailyStatisticsRequest request;
                      request.mutable_userinfo()->CopyFrom(CachedProtoData::getInstance().CachedUserMetaInfo);
                      request.mutable_dailystatisticsinfo()->set_name(curItem->text(0).toStdString());

                      proto::DeleteDailyStatisticsResponse response;
                      grpc::ClientContext context;
                      auto status = RpcCall::getInstance().Stub()->DeleteDailyStatistics(&context,request,&response);
                      if(status.ok()) {
                          if(response.status()) {
                              QMessageBox::information(this,"Info","Delete DailyStatistics successfully!");
                              m_MainWindow->getRightClientView().closeWithoutSavingDailyStatistics(data.name);
                              refreshTree();
                          }else {
                              QMessageBox::critical(this,"Error",QString::fromStdString(response.message()));
                          }
                      }else{
                          QMessageBox::critical(this,"Error",QString::fromStdString(status.error_message()));
                      }
                  }
              }
    });

    auto* MenuEditDailyStatistics = new QAction(this);
    MenuEditDailyStatistics->setText("Edit DailyStatistics");
    MenuEditDailyStatistics->setIcon(QIcon(Image::ImageDelete));
    connect(MenuEditDailyStatistics,&QAction::triggered,this,[this,data](bool checked) {
        m_MainWindow->getRightClientView().openDailyStatisticsMetaInfo(data.name);
    });

    switch (data.type) {
        case MetaInfoType::eProjectContainer: {
            popMenu->addAction(MenuCreateProject);
            break;
        }
        case MetaInfoType::eProject: {
            popMenu->addAction(MenuEditProject);
            popMenu->addSeparator();
            popMenu->addAction(MenuExportToSwcFile);
            popMenu->addSeparator();
            popMenu->addAction(MenuDeleteProject);
            break;
        }
        case MetaInfoType::eSwcContainer: {
            popMenu->addAction(MenuImportSwcFile);
            popMenu->addAction(MenuExportToSwcFile);
            popMenu->addSeparator();
            popMenu->addAction(MenuCreateSwc);
            break;
        }
        case MetaInfoType::eSwc: {
            popMenu->addAction(MenuEditSwc);
            popMenu->addAction(MenuEditSwcNodeData);
            popMenu->addSeparator();
            popMenu->addAction(MenuExportToSwcFile);
            popMenu->addSeparator();
            popMenu->addAction(MenuDeleteSwc);
            break;
        }
        case MetaInfoType::eDailyStatisticsContainer:
            break;
        case MetaInfoType::eDailyStatistics:
            popMenu->addAction(MenuEditDailyStatistics);
            popMenu->addSeparator();
            popMenu->addAction(MenuDeleteDailyStatistics);
            break;
        case MetaInfoType::eUserMetaInfo:
            break;
        case MetaInfoType::ePermissionGroupMetaInfo:
            break;
        case MetaInfoType::eUserManagerMetaInfo:
            break;
        case MetaInfoType::eSwcData:
            break;
        case MetaInfoType::eUnknown:
            break;
    }

    auto* triggeredAction = popMenu->exec(QCursor::pos());
    if (!triggeredAction) {
        return;
    }
}

void LeftClientView::refreshTree() {
    clearAll();
    getProjectMetaInfo();
    getSwcMetaInfo();
    getAllDailyStatisticsMetaInfo();
}

void LeftClientView::clearAll() {
    m_TreeWidget->clear();

    m_TopProjectItem = new QTreeWidgetItem(m_TreeWidget);
    m_TopProjectItem->setText(0, "Project");
    m_TopProjectItem->setIcon(0, QIcon(Image::ImageProject));
    LeftClientViewTreeWidgetItemMetaInfo metaInfoProject{};
    metaInfoProject.type = MetaInfoType::eProjectContainer;
    metaInfoProject.name = "Project";
    m_TopProjectItem->setData(0, Qt::UserRole, QVariant::fromValue(metaInfoProject));

    m_TopSwcItem = new QTreeWidgetItem(m_TreeWidget);
    m_TopSwcItem->setText(0, "Swc");
    m_TopSwcItem->setIcon(0, QIcon(Image::ImageNode));
    LeftClientViewTreeWidgetItemMetaInfo metaInfoSwc{};
    metaInfoSwc.type = MetaInfoType::eSwcContainer;
    metaInfoProject.name = "Swc";
    m_TopSwcItem->setData(0, Qt::UserRole, QVariant::fromValue(metaInfoSwc));

    m_TopDailyStatisticsItem = new QTreeWidgetItem(m_TreeWidget);
    m_TopDailyStatisticsItem->setText(0, "DailyStatistics");
    m_TopDailyStatisticsItem->setIcon(0, QIcon(Image::ImageDaily));
    LeftClientViewTreeWidgetItemMetaInfo metaInfoDailyStatistic{};
    metaInfoDailyStatistic.type = MetaInfoType::eDailyStatisticsContainer;
    metaInfoProject.name = "DailyStatistics";
    m_TopDailyStatisticsItem->setData(0, Qt::UserRole, QVariant::fromValue(metaInfoDailyStatistic));

    m_TreeWidget->addTopLevelItem(m_TopProjectItem);
    m_TreeWidget->addTopLevelItem(m_TopSwcItem);
    m_TreeWidget->addTopLevelItem(m_TopDailyStatisticsItem);

    auto rawHeadPhotoBinData = CachedProtoData::getInstance().CachedUserMetaInfo.headphotobindata();
    auto headPhotoBinData = QByteArray::fromStdString(rawHeadPhotoBinData);

    if(headPhotoBinData.size() > 0) {
        QPixmap pixmap;
        pixmap.loadFromData(headPhotoBinData);
        pixmap = pixmap.scaled(QSize(128,128),Qt::KeepAspectRatioByExpanding);
        m_AccountBtn->setIcon(pixmap);
    }else {
        m_AccountBtn->setIcon(QIcon(Image::ImageDefaultUserHeadPhoto));
    }
}
