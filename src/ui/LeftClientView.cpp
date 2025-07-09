#include "LeftClientView.h"

#include <ui_ViewCreateSwc.h>

#include <QDebug>
#include <chrono>

#include "CreateSwcSnapshotView.h"
#include "EditorAttachmentView.h"
#include "EditorBatchManageSwc.h"
#include "EditorPermission.h"
#include "EditorSwcIncrementRecord.h"
#include "EditorSwcSnapshot.h"
#include "EditorSwcVersionControl.h"
#include "MainWindow.h"
#include "ViewCreateProject.h"
#include "ViewCreateSwc.h"
#include "ViewExportSwcToFile.h"
#include "ViewImportSwcFromFile.h"
#include "ProgressBar.h"
#include "src/framework/config/AppConfig.h"
#include "src/framework/defination/ImageDefination.h"
#include "src/framework/defination/TypeDef.h"
#include "src/framework/service/CachedProtoData.h"
#include "src/framework/service/RpcCall.h"
#include "src/framework/service/WrappedCall.h"
#include "src/ui/EditorUserSettings.h"
#include "ui_LeftClientView.h"

LeftClientView::LeftClientView(MainWindow* mainWindow)
	: QWidget(mainWindow), ui(new Ui::LeftClientView) {
	ui->setupUi(this);
	m_MainWindow = mainWindow;

	m_ControlBtnLayout = new QHBoxLayout;

	m_UserSettingBtn = new QPushButton(this);
	m_UserSettingBtn->setText("User Settings");
	m_UserSettingBtn->setIcon(QIcon(Image::ImageUser));

	auto rawHeadPhotoBinData =
		CachedProtoData::getInstance().CachedUserMetaInfo.headphotobindata();
	auto headPhotoBinData = QByteArray::fromStdString(rawHeadPhotoBinData);

	connect(m_UserSettingBtn, &QPushButton::clicked, this,
			[this](bool checked) {
				EditorUserSettings editorUserSettings(this);
				editorUserSettings.exec();
			});

	m_AccountBtn = new QToolButton(this);
	m_AccountBtn->setPopupMode(QToolButton::InstantPopup);
	m_AccountBtn->setText("Account");
	m_AccountBtn->setToolButtonStyle(Qt::ToolButtonIconOnly);
	m_AccountBtn->setIconSize({32, 32});

	if (headPhotoBinData.size() > 0) {
		QPixmap pixmap;
		pixmap.loadFromData(headPhotoBinData);
		pixmap = pixmap.scaled(QSize(128, 128), Qt::KeepAspectRatioByExpanding);
		m_AccountBtn->setIcon(pixmap);
	} else {
		m_AccountBtn->setIcon(QIcon(Image::ImageDefaultUserHeadPhoto));
	}

	auto* logoutAction = new QAction(this);
	logoutAction->setText("Logout");
	connect(logoutAction, &QAction::triggered, this, [this](bool checked) {
		auto btn = QMessageBox::information(
			this, "Warning",
			"Are you sure to logout? Please save all your work before logout!",
			QMessageBox::StandardButton::Ok,
			QMessageBox::StandardButton::Cancel);
		if (btn == QMessageBox::StandardButton::Ok) {
			proto::UserLogoutRequest request;
			request.mutable_metainfo()->set_apiversion(RpcCall::ApiVersion);
			request.mutable_userverifyinfo()->set_username(
				CachedProtoData::getInstance().UserName);
			request.mutable_userverifyinfo()->set_usertoken(
				CachedProtoData::getInstance().UserToken);
			proto::UserLogoutResponse response;

			grpc::ClientContext context;
			RpcCall::getInstance().Stub()->UserLogout(&context, request,
													  &response);
			AppConfig::getInstance().setSecurityConfig(
				AppConfig::SecurityConfigItem::eCachedUserName, "");
			AppConfig::getInstance().setSecurityConfig(
				AppConfig::SecurityConfigItem::eCachedPassword, "");
			AppConfig::getInstance().setSecurityConfig(
				AppConfig::SecurityConfigItem::eAccountExpiredTime, "");
			AppConfig::getInstance().writeSecurityConfig();

			qApp->exit(RestartCode);
		}
	});
	m_AccountBtn->addAction(logoutAction);

	m_RefreshBtn = new QPushButton(this);
	m_RefreshBtn->setText("Refresh");
	m_RefreshBtn->setIcon(QIcon(Image::ImageRefresh));
	connect(m_RefreshBtn, &QPushButton::clicked, this,
			&LeftClientView::onRefreshBtnClicked);

	m_ControlBtnLayout->addWidget(m_AccountBtn);
	m_ControlBtnLayout->addWidget(m_UserSettingBtn);
	m_ControlBtnLayout->addWidget(m_RefreshBtn);

	m_TreeWidget = new QTreeWidget(this);
	m_TreeWidget->setHeaderLabel("MetaInfo");
	connect(
		m_TreeWidget, &QTreeWidget::itemDoubleClicked, this,
		[this](QTreeWidgetItem* item, int column) {
			if (column == 0) {
				if (item) {
					auto metaInfo =
						item->data(0, Qt::UserRole)
							.value<LeftClientViewTreeWidgetItemMetaInfo>();
					switch (metaInfo.type) {
						case MetaInfoType::eProjectContainer:
							break;
						case MetaInfoType::eProject: {
							m_MainWindow->getRightClientView()
								.openProjectMetaInfo(metaInfo.uuid);
							break;
						}
						case MetaInfoType::eFreeSwcContainer:
							break;
						case MetaInfoType::eFreeSwc: {
							m_MainWindow->getRightClientView().openSwcMetaInfo(
								metaInfo.uuid);
							break;
						}
						case MetaInfoType::eProjectSwc: {
							m_MainWindow->getRightClientView().openSwcMetaInfo(
								metaInfo.uuid);
							break;
						}
						case MetaInfoType::eDailyStatisticsContainer:
							break;
						case MetaInfoType::eDailyStatistics: {
							m_MainWindow->getRightClientView()
								.openDailyStatisticsMetaInfo(metaInfo.name);
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
	connect(m_TreeWidget, &QTreeWidget::customContextMenuRequested, this,
			&LeftClientView::customTreeWidgetContentMenu);

	m_MainLayout = new QVBoxLayout(this);
	m_MainLayout->addLayout(m_ControlBtnLayout);
	m_MainLayout->addWidget(m_TreeWidget);
	this->setLayout(m_MainLayout);

	refreshTree();
}

LeftClientView::~LeftClientView() { delete ui; }

void LeftClientView::getProjectMetaInfo() {
	proto::GetAllProjectResponse response;
	WrappedCall::getAllProjectMetaInfo(response, this);
	auto projectInfoList = response.mutable_projectinfo();
	for (int i = 0; i < projectInfoList->size(); i++) {
		auto& projectInfo = projectInfoList->Get(i);
		auto* projectItem = new QTreeWidgetItem;
		projectItem->setText(0, QString::fromStdString(projectInfo.name()));
		projectItem->setIcon(0, QIcon(Image::ImageProject));
		LeftClientViewTreeWidgetItemMetaInfo metaInfo{};
		metaInfo.type = MetaInfoType::eProject;
		metaInfo.uuid = projectInfo.base().uuid();
		projectItem->setData(0, Qt::UserRole, QVariant::fromValue(metaInfo));
		m_TopProjectItem->addChild(projectItem);

		proto::GetProjectSwcNamesByProjectUuidResponse swcNamesResponse;
		std::sort(swcNamesResponse.mutable_swcuuidname()->begin(),
				  swcNamesResponse.mutable_swcuuidname()->end(),
				  [](const proto::SwcUuidName& a, const proto::SwcUuidName& b) {
					  return a.swcname() < b.swcname();
				  });
		if (WrappedCall::GetProjectSwcNamesByProjectUuid(
				projectInfo.base().uuid(), swcNamesResponse, this)) {
			for (auto& swcuuidname : swcNamesResponse.swcuuidname()) {
				auto* swcItem = new QTreeWidgetItem;
				swcItem->setText(0,
								 QString::fromStdString(swcuuidname.swcname()));
				swcItem->setIcon(0, QIcon(Image::ImageNode));
				metaInfo.type = MetaInfoType::eProjectSwc;
				metaInfo.uuid = swcuuidname.swcuuid();
				swcItem->setData(0, Qt::UserRole,
								 QVariant::fromValue(metaInfo));
				projectItem->addChild(swcItem);
			}
		}
	}
}

void LeftClientView::getFreeSwcMetaInfo() {
	proto::GetAllFreeSwcMetaInfoRequest request;
	WrappedCall::setCommonRequestField(request);
	proto::GetAllFreeSwcMetaInfoResponse response;

	grpc::ClientContext context;
	auto status = RpcCall::getInstance().Stub()->GetAllFreeSwcMetaInfo(
		&context, request, &response);
	WrappedCall::defaultErrorHandler(__func__, status, response.metainfo(),
									 this);

	auto swcMetaInfo = response.mutable_swcuuidname();
	for (int i = 0; i < swcMetaInfo->size(); i++) {
		auto& swcInfo = swcMetaInfo->Get(i);
		auto* item = new QTreeWidgetItem;
		item->setText(0, QString::fromStdString(swcInfo.swcname()));
		item->setIcon(0, QIcon(Image::ImageNode));
		LeftClientViewTreeWidgetItemMetaInfo metaInfo{};
		metaInfo.type = MetaInfoType::eFreeSwc;
		metaInfo.uuid = swcInfo.swcuuid();
		item->setData(0, Qt::UserRole, QVariant::fromValue(metaInfo));
		m_TopSwcItem->addChild(item);
	}
}

void LeftClientView::getAllDailyStatisticsMetaInfo() {
	proto::GetAllDailyStatisticsResponse response;
	WrappedCall::getAllDailyStatisticsMetaInfo(response, this);
	auto dailyStatisticsMetaInfoList = response.mutable_dailystatisticsinfo();
	for (int i = 0; i < dailyStatisticsMetaInfoList->size(); i++) {
		auto& dailyStatisticsMetaInfo = dailyStatisticsMetaInfoList->Get(i);
		auto* item = new QTreeWidgetItem;
		item->setText(0,
					  QString::fromStdString(dailyStatisticsMetaInfo.name()));
		item->setIcon(0, QIcon(Image::ImageDaily));
		LeftClientViewTreeWidgetItemMetaInfo metaInfo{};
		metaInfo.type = MetaInfoType::eDailyStatistics;
		metaInfo.uuid = dailyStatisticsMetaInfo.base().uuid();
		metaInfo.name = dailyStatisticsMetaInfo.name();
		item->setData(0, Qt::UserRole, QVariant::fromValue(metaInfo));
		m_TopDailyStatisticsItem->addChild(item);
	}
}

void LeftClientView::onRefreshBtnClicked(bool checked) { refreshTree(); }

void LeftClientView::customTreeWidgetContentMenu(const QPoint& pos) {
	auto* curItem = m_TreeWidget->currentItem();
	if (!curItem) {
		return;
	}

	auto data = curItem->data(0, Qt::UserRole)
					.value<LeftClientViewTreeWidgetItemMetaInfo>();

	auto* popMenu = new QMenu(this);

	auto* MenuCreateProject = new QAction(this);
	MenuCreateProject->setText("Create Project");
	MenuCreateProject->setIcon(QIcon(Image::ImageCreate));
	connect(MenuCreateProject, &QAction::triggered, this, [this](bool checked) {
		ViewCreateProject view;
		view.exec();
		refreshTree();
	});

	auto* MenuExportDefinedSomaSwc = new QAction(this);
	MenuExportDefinedSomaSwc->setText("Export Defined Soma Swc");
	MenuExportDefinedSomaSwc->setIcon(QIcon(Image::ImageExport));
	connect(
		MenuExportDefinedSomaSwc, &QAction::triggered, this,
		[this, data](bool checked) {
			auto result = QMessageBox::information(
				this, "Info",
				"Exporting defined soma SWC may take some time, please wait "
				"for export steps to finish!",
				QMessageBox::StandardButton::Ok,
				QMessageBox::StandardButton::Cancel);
			if (result == QMessageBox::StandardButton::Ok) {
				if (data.type == MetaInfoType::eProject) {
					// 创建进度条
					ProgressBar progressBar(this);
					progressBar.setText("Loading defined soma SWC data...");
					progressBar.setValue(0);
					progressBar.show();
					
					QApplication::processEvents(); // 确保进度条显示

					// 获取项目定义的soma SWC UUIDs
					progressBar.setText("Getting project defined soma SWC UUIDs...");
					progressBar.setValue(20);
					QApplication::processEvents();
					
					std::vector<std::string> projectUuids = {data.uuid};
					proto::GetProjectsDefinedSomaSwcResponse somaSwcResponse;
					if (!WrappedCall::getProjectsDefinedSomaSwc(
							projectUuids, somaSwcResponse, this)) {
						progressBar.finish();
						return;
					}

					if (somaSwcResponse.swcuuids_size() == 0) {
						progressBar.finish();
						QMessageBox::information(
							this, "Info",
							"No defined soma SWC found in this project.");
						return;
					}

					// 获取所有SWC元信息
					progressBar.setText("Getting SWC meta info...");
					progressBar.setValue(50);
					QApplication::processEvents();

					std::vector<ExportSwcData> dataList;

					// 直接获取每个定义的soma SWC的元信息
					int processedCount = 0;
					int totalCount = somaSwcResponse.swcuuids_size();

					for (const auto& swcUuid : somaSwcResponse.swcuuids()) {
						progressBar.setText(QString("Getting SWC meta info... (%1/%2)")
							.arg(processedCount + 1).arg(totalCount).toStdString());
						progressBar.setValue(50 + (processedCount * 30 / totalCount));
						QApplication::processEvents();

						proto::GetSwcMetaInfoResponse response;
						if (WrappedCall::getSwcMetaInfoByUuid(swcUuid, response, this)) {
							ExportSwcData exportSwcData;
							exportSwcData.swcMetaInfo = response.swcinfo();
							dataList.push_back(exportSwcData);
						}
						processedCount++;
					}

					if (dataList.empty()) {
						progressBar.finish();
						QMessageBox::information(
							this, "Info", "No valid defined soma SWC found.");
						return;
					}

					progressBar.setText("Preparing export dialog...");
					progressBar.setValue(100);
					QApplication::processEvents();
					
					progressBar.finish();

					ViewExportSwcToFile view(dataList, true, this);
					view.exec();
				}
			}
		});

	auto* MenuDeleteProject = new QAction(this);
	MenuDeleteProject->setText("Delete Project");
	MenuDeleteProject->setIcon(QIcon(Image::ImageDelete));
	connect(
		MenuDeleteProject, &QAction::triggered, this,
		[this, data, curItem](bool checked) {
			auto result =
				QMessageBox::information(this, "Warning",
										 "Are you sure to delete this project? "
										 "This operation cannot be revert!",
										 QMessageBox::StandardButton::Ok,
										 QMessageBox::StandardButton::Cancel);
			if (result == QMessageBox::StandardButton::Ok) {
				if (data.type == MetaInfoType::eProject) {
					proto::DeleteProjectRequest request;
					request.mutable_metainfo()->set_apiversion(
						RpcCall::ApiVersion);
					auto* userInfo = request.mutable_userverifyinfo();
					userInfo->set_username(
						CachedProtoData::getInstance().UserName);
					userInfo->set_usertoken(
						CachedProtoData::getInstance().UserToken);
					request.set_projectuuid(data.uuid);

					proto::DeleteProjectResponse response;
					grpc::ClientContext context;
					auto status = RpcCall::getInstance().Stub()->DeleteProject(
						&context, request, &response);
					if (status.ok()) {
						if (response.metainfo().status()) {
							QMessageBox::information(
								this, "Info", "Delete Project successfully!");
							m_MainWindow->getRightClientView()
								.closeWithoutSavingProject(data.uuid);
							refreshTree();
						} else {
							QMessageBox::critical(
								this, "Error",
								QString::fromStdString(
									response.metainfo().message()));
						}
					} else {
						QMessageBox::critical(
							this, "Error",
							QString::fromStdString(status.error_message()));
					}
				}
			}
		});

	auto* MenuEditProject = new QAction(this);
	MenuEditProject->setText("Edit Project");
	MenuEditProject->setIcon(QIcon(Image::ImageEdit));
	connect(
		MenuEditProject, &QAction::triggered, this, [this, data](bool checked) {
			m_MainWindow->getRightClientView().openProjectMetaInfo(data.uuid);
		});

	auto* MenuEditProjectPermission = new QAction(this);
	MenuEditProjectPermission->setText("Edit Permission");
	MenuEditProjectPermission->setIcon(QIcon(Image::ImageACL));
	connect(MenuEditProjectPermission, &QAction::triggered, this,
			[this, data](bool checked) {
				EditorPermission view(data.uuid, MetaInfoType::eProject, false,
									  this);
				view.exec();
			});

	auto* MenuImportSwcFile = new QAction(this);
	MenuImportSwcFile->setText("Import Swc File");
	MenuImportSwcFile->setIcon(QIcon(Image::ImageImport));
	connect(MenuImportSwcFile, &QAction::triggered, this,
			[this, data](bool checked) {
				if (data.type == MetaInfoType::eFreeSwcContainer) {
					ViewImportSwcFromFile view(m_MainWindow, "");
					view.exec();
					refreshTree();
				} else if (data.type == MetaInfoType::eProject) {
					ViewImportSwcFromFile view(m_MainWindow, data.uuid);
					view.exec();
					refreshTree();
				}
			});

	auto* MenuExportToSwcFile = new QAction(this);
	MenuExportToSwcFile->setText("Export Swc File");
	MenuExportToSwcFile->setIcon(QIcon(Image::ImageExport));
	connect(
		MenuExportToSwcFile, &QAction::triggered, this,
		[this, &data](bool checked) {
			auto result = QMessageBox::information(
				this, "Info",
				"Exporting action may takes few time please waiting for export "
				"steps finish!",
				QMessageBox::StandardButton::Ok,
				QMessageBox::StandardButton::Cancel);
			if (result == QMessageBox::StandardButton::Ok) {
				if (data.type == MetaInfoType::eFreeSwc ||
					data.type == MetaInfoType::eProjectSwc) {
					proto::GetSwcMetaInfoResponse response1;
					if (!WrappedCall::getSwcMetaInfoByUuid(data.uuid, response1,
														   this)) {
						return;
					}

					proto::GetSwcFullNodeDataResponse response2;
					if (!WrappedCall::getSwcFullNodeDataByUuid(
							data.uuid, response2, this)) {
						return;
					}

					std::vector<ExportSwcData> dataList;
					ExportSwcData exportSwcData;
					exportSwcData.swcData = response2.swcnodedata();
					exportSwcData.swcMetaInfo = response1.swcinfo();
					dataList.push_back(exportSwcData);

					ViewExportSwcToFile view(dataList, false, this);
					view.exec();
				} else if (data.type == MetaInfoType::eFreeSwcContainer) {
					proto::GetAllSwcMetaInfoResponse response;
					if (!WrappedCall::getAllSwcMetaInfo(response, this)) {
						return;
					}

					std::vector<ExportSwcData> dataList;
					for (int i = 0; i < response.swcinfo_size(); i++) {
						ExportSwcData exportSwcData;
						exportSwcData.swcMetaInfo = response.swcinfo(i);
						dataList.push_back(exportSwcData);
					}

					ViewExportSwcToFile view(dataList, true, this);
					view.exec();
				} else if (data.type == MetaInfoType::eProject) {
					proto::GetProjectResponse projectResponse;
					if (!WrappedCall::getProjectMetaInfoByUuid(
							data.uuid, projectResponse, this)) {
						return;
					}

					std::vector<proto::SwcMetaInfoV1> swcMetaInfos;

					proto::GetAllSwcMetaInfoResponse responseAllSwc;
					WrappedCall::getAllSwcMetaInfo(responseAllSwc, this);
					for (int i = 0; i < responseAllSwc.swcinfo_size(); i++) {
						auto& swcInfo = responseAllSwc.swcinfo(i);
						bool bFind = false;
						for (int j = 0;
							 j < projectResponse.projectinfo().swclist().size();
							 j++) {
							auto uuid =
								projectResponse.projectinfo().swclist(j);
							if (uuid == swcInfo.base().uuid()) {
								bFind = true;
							}
						}
						if (bFind) {
							swcMetaInfos.push_back(responseAllSwc.swcinfo(i));
						}
					}

					std::vector<ExportSwcData> dataList;
					for (const auto& swcMetaInfo : swcMetaInfos) {
						ExportSwcData exportSwcData;
						exportSwcData.swcMetaInfo = swcMetaInfo;
						dataList.push_back(exportSwcData);
					}

					ViewExportSwcToFile view(dataList, true, this);
					view.exec();
				}
			}
		});

	auto* MenuCreateSwc = new QAction(this);
	MenuCreateSwc->setText("Create Swc");
	MenuCreateSwc->setIcon(QIcon(Image::ImageCreate));
	connect(MenuCreateSwc, &QAction::triggered, this, [this](bool checked) {
		ViewCreateSwc view;
		view.exec();
		refreshTree();
	});

	auto* MenuBatchManageSwc = new QAction(this);
	MenuBatchManageSwc->setText("Batch Manage Swc");
	MenuBatchManageSwc->setIcon(QIcon(Image::ImageBatchManage));
	connect(MenuBatchManageSwc, &QAction::triggered, this,
			[this, data](bool checked) {
				if (data.type == MetaInfoType::eProject) {
					std::vector<std::string> uuids;

					proto::GetProjectSwcNamesByProjectUuidResponse responseSwc;
					if (!WrappedCall::GetProjectSwcNamesByProjectUuid(
							data.uuid, responseSwc, this)) {
						return;
					}

					for (auto& swcuuidname : responseSwc.swcuuidname()) {
						uuids.push_back(swcuuidname.swcuuid());
					}

					EditorBatchManageSwc view(uuids, this);
					view.exec();
					refreshTree();
				} else if (data.type == MetaInfoType::eFreeSwcContainer) {
					std::vector<std::string> uuids;
					proto::GetAllFreeSwcMetaInfoRequest request;
					WrappedCall::setCommonRequestField(request);
					proto::GetAllFreeSwcMetaInfoResponse response;

					grpc::ClientContext context;
					auto status =
						RpcCall::getInstance().Stub()->GetAllFreeSwcMetaInfo(
							&context, request, &response);
					WrappedCall::defaultErrorHandler(__func__, status,
													 response.metainfo(), this);

					for (auto& info : response.swcuuidname()) {
						uuids.push_back(info.swcuuid());
					}

					EditorBatchManageSwc view(uuids, this);
					view.exec();
					refreshTree();
				}
			});

	auto* MenuDeleteSwc = new QAction(this);
	MenuDeleteSwc->setText("Delete Swc");
	MenuDeleteSwc->setIcon(QIcon(Image::ImageDelete));
	connect(
		MenuDeleteSwc, &QAction::triggered, this, [this, data](bool checked) {
			auto result =
				QMessageBox::information(this, "Warning",
										 "Are you sure to delete this Swc? "
										 "This operation cannot be revert!",
										 QMessageBox::StandardButton::Ok,
										 QMessageBox::StandardButton::Cancel);
			if (result == QMessageBox::StandardButton::Ok) {
				if (data.type == MetaInfoType::eFreeSwc ||
					data.type == MetaInfoType::eProjectSwc) {
					proto::DeleteSwcRequest request;
					WrappedCall::setCommonRequestField(request);
					request.set_swcuuid(data.uuid);

					proto::DeleteSwcResponse response;
					grpc::ClientContext context;
					auto status = RpcCall::getInstance().Stub()->DeleteSwc(
						&context, request, &response);
					if (status.ok()) {
						if (response.metainfo().status()) {
							QMessageBox::information(
								this, "Info", "Delete Swc successfully!");
							m_MainWindow->getRightClientView()
								.closeWithoutSavingSwc(data.uuid);
							m_MainWindow->getRightClientView()
								.refreshAllOpenedProjectMetaInfo();
							refreshTree();
						} else {
							QMessageBox::critical(
								this, "Error",
								QString::fromStdString(
									response.metainfo().message()));
						}
					} else {
						QMessageBox::critical(
							this, "Error",
							QString::fromStdString(status.error_message()));
					}
				}
			}
		});

	auto* MenuEditSwc = new QAction(this);
	MenuEditSwc->setText("Edit Swc");
	MenuEditSwc->setIcon(QIcon(Image::ImageEdit));
	connect(MenuEditSwc, &QAction::triggered, this, [this, data](bool checked) {
		m_MainWindow->getRightClientView().openSwcMetaInfo(data.uuid);
	});

	auto* MenuEditSwcNodeData = new QAction(this);
	MenuEditSwcNodeData->setText("Edit SwcNodeData");
	MenuEditSwcNodeData->setIcon(QIcon(Image::ImageEdit));
	connect(MenuEditSwcNodeData, &QAction::triggered, this,
			[this, data](bool checked) {
				m_MainWindow->getRightClientView().openSwcNodeData(data.uuid);
			});

	auto* MenuEditSwcPermission = new QAction(this);
	MenuEditSwcPermission->setText("Edit Permission");
	MenuEditSwcPermission->setIcon(QIcon(Image::ImageACL));
	connect(MenuEditSwcPermission, &QAction::triggered, this,
			[this, data](bool checked) {
				EditorPermission view(data.uuid, MetaInfoType::eFreeSwc, false,
									  this);
				view.exec();
			});

	auto* MenuDeleteDailyStatistics = new QAction(this);
	MenuDeleteDailyStatistics->setText("Delete DailyStatistics");
	MenuDeleteDailyStatistics->setIcon(QIcon(Image::ImageDelete));
	connect(
		MenuDeleteDailyStatistics, &QAction::triggered, this,
		[this, data, curItem](bool checked) {
			auto result = QMessageBox::information(
				this, "Warning",
				"Are you sure to delete this DailyStatistics? This operation "
				"cannot be revert!",
				QMessageBox::StandardButton::Ok,
				QMessageBox::StandardButton::Cancel);
			if (result == QMessageBox::StandardButton::Ok) {
				if (data.type == MetaInfoType::eDailyStatistics) {
					proto::DeleteDailyStatisticsRequest request;
					WrappedCall::setCommonRequestField(request);
					request.set_dailystatisticsname(
						curItem->text(0).toStdString());

					proto::DeleteDailyStatisticsResponse response;
					grpc::ClientContext context;
					auto status =
						RpcCall::getInstance().Stub()->DeleteDailyStatistics(
							&context, request, &response);
					if (status.ok()) {
						if (response.metainfo().status()) {
							QMessageBox::information(
								this, "Info",
								"Delete DailyStatistics successfully!");
							m_MainWindow->getRightClientView()
								.closeWithoutSavingDailyStatistics(data.uuid);
							refreshTree();
						} else {
							QMessageBox::critical(
								this, "Error",
								QString::fromStdString(
									response.metainfo().message()));
						}
					} else {
						QMessageBox::critical(
							this, "Error",
							QString::fromStdString(status.error_message()));
					}
				}
			}
		});

	auto* MenuEditDailyStatistics = new QAction(this);
	MenuEditDailyStatistics->setText("Edit DailyStatistics");
	MenuEditDailyStatistics->setIcon(QIcon(Image::ImageDelete));
	connect(MenuEditDailyStatistics, &QAction::triggered, this,
			[this, data](bool checked) {
				m_MainWindow->getRightClientView().openDailyStatisticsMetaInfo(
					data.name);
			});

	auto* MenuEditorAttachment = new QAction(this);
	MenuEditorAttachment->setText("Edit Attachment");
	MenuEditorAttachment->setIcon(QIcon(Image::ImageAttachment));
	connect(MenuEditorAttachment, &QAction::triggered, this,
			[this, data](bool checked) {
				EditorAttachmentView view(data.uuid, this);
				view.exec();
			});

	auto* MenuCreateSnapshot = new QAction(this);
	MenuCreateSnapshot->setText("Create Snapshot");
	MenuCreateSnapshot->setIcon(QIcon(Image::ImageSnapshot));
	connect(MenuCreateSnapshot, &QAction::triggered, this,
			[this, data](bool checked) {
				CreateSwcSnapshotView view(data.uuid, this);
				view.exec();
			});

	auto* MenuEditorSwcSnapshot = new QAction(this);
	MenuEditorSwcSnapshot->setText("View Snapshot");
	MenuEditorSwcSnapshot->setIcon(QIcon(Image::ImageSnapshot));
	connect(MenuEditorSwcSnapshot, &QAction::triggered, this,
			[this, data](bool checked) {
				EditorSwcSnapshot view(data.uuid, this);
				view.exec();
			});

	auto* MenuEditorSwcIncrement = new QAction(this);
	MenuEditorSwcIncrement->setText("View Increment Record");
	MenuEditorSwcIncrement->setIcon(QIcon(Image::ImageIncrement));
	connect(MenuEditorSwcIncrement, &QAction::triggered, this,
			[this, data](bool checked) {
				EditorSwcIncrementRecord view(data.uuid, this);
				view.exec();
			});

	auto* MenuEditorVersionControl = new QAction(this);
	MenuEditorVersionControl->setText("Version Control");
	MenuEditorVersionControl->setIcon(QIcon(Image::ImageVersionControl));
	connect(MenuEditorVersionControl, &QAction::triggered, this,
			[data](bool checked) {
				auto view = new EditorSwcVersionControl(data.uuid);
				view->setAttribute(Qt::WA_DeleteOnClose);
				view->exec();
			});

	switch (data.type) {
		case MetaInfoType::eProjectContainer: {
			popMenu->addAction(MenuCreateProject);
			break;
		}
		case MetaInfoType::eProject: {
			popMenu->addAction(MenuEditProject);
			popMenu->addAction(MenuBatchManageSwc);
			popMenu->addSeparator();
			popMenu->addAction(MenuEditProjectPermission);
			popMenu->addSeparator();
			popMenu->addAction(MenuImportSwcFile);
			popMenu->addAction(MenuExportToSwcFile);
			popMenu->addAction(MenuExportDefinedSomaSwc);
			popMenu->addSeparator();
			popMenu->addAction(MenuDeleteProject);
			break;
		}
		case MetaInfoType::eFreeSwcContainer: {
			popMenu->addAction(MenuImportSwcFile);
			popMenu->addAction(MenuExportToSwcFile);
			popMenu->addSeparator();
			popMenu->addAction(MenuCreateSwc);
			popMenu->addSeparator();
			popMenu->addAction(MenuBatchManageSwc);
			break;
		}
		case MetaInfoType::eFreeSwc:
		case MetaInfoType::eProjectSwc: {
			popMenu->addAction(MenuEditSwc);
			popMenu->addAction(MenuEditSwcNodeData);
			popMenu->addAction(MenuEditSwcPermission);
			popMenu->addAction(MenuEditorAttachment);
			popMenu->addSeparator();
			popMenu->addAction(MenuExportToSwcFile);
			popMenu->addSeparator();
			popMenu->addAction(MenuCreateSnapshot);
			popMenu->addAction(MenuEditorSwcSnapshot);
			popMenu->addAction(MenuEditorSwcIncrement);
			popMenu->addAction(MenuEditorVersionControl);
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
	auto startTime = std::chrono::high_resolution_clock::now();

	// 清空所有数据
	auto clearStartTime = std::chrono::high_resolution_clock::now();
	clearAll();
	auto clearEndTime = std::chrono::high_resolution_clock::now();
	auto clearDuration = std::chrono::duration_cast<std::chrono::milliseconds>(
							 clearEndTime - clearStartTime)
							 .count();
	qDebug() << "clearAll() 用时:" << clearDuration << "ms";

	// 获取项目元信息
	auto projectStartTime = std::chrono::high_resolution_clock::now();
	getProjectMetaInfo();
	auto projectEndTime = std::chrono::high_resolution_clock::now();
	auto projectDuration =
		std::chrono::duration_cast<std::chrono::milliseconds>(projectEndTime -
															  projectStartTime)
			.count();
	qDebug() << "getProjectMetaInfo() 用时:" << projectDuration << "ms";

	// 获取自由SWC元信息
	auto freeSwcStartTime = std::chrono::high_resolution_clock::now();
	getFreeSwcMetaInfo();
	auto freeSwcEndTime = std::chrono::high_resolution_clock::now();
	auto freeSwcDuration =
		std::chrono::duration_cast<std::chrono::milliseconds>(freeSwcEndTime -
															  freeSwcStartTime)
			.count();
	qDebug() << "getFreeSwcMetaInfo() 用时:" << freeSwcDuration << "ms";

	// 获取每日统计元信息
	auto dailyStatsStartTime = std::chrono::high_resolution_clock::now();
	getAllDailyStatisticsMetaInfo();
	auto dailyStatsEndTime = std::chrono::high_resolution_clock::now();
	auto dailyStatsDuration =
		std::chrono::duration_cast<std::chrono::milliseconds>(
			dailyStatsEndTime - dailyStatsStartTime)
			.count();
	qDebug() << "getAllDailyStatisticsMetaInfo() 用时:" << dailyStatsDuration
			 << "ms";

	auto endTime = std::chrono::high_resolution_clock::now();
	auto totalDuration = std::chrono::duration_cast<std::chrono::milliseconds>(
							 endTime - startTime)
							 .count();
	qDebug() << "refreshTree() 总用时:" << totalDuration << "ms";
}

void LeftClientView::clearAll() {
	m_TreeWidget->clear();

	m_TopProjectItem = new QTreeWidgetItem(m_TreeWidget);
	m_TopProjectItem->setText(0, "Project");
	m_TopProjectItem->setIcon(0, QIcon(Image::ImageProject));
	LeftClientViewTreeWidgetItemMetaInfo metaInfoProject{};
	metaInfoProject.type = MetaInfoType::eProjectContainer;
	m_TopProjectItem->setData(0, Qt::UserRole,
							  QVariant::fromValue(metaInfoProject));

	m_TopSwcItem = new QTreeWidgetItem(m_TreeWidget);
	m_TopSwcItem->setText(0, "Free Swc (Does not belong to any project)");
	m_TopSwcItem->setIcon(0, QIcon(Image::ImageNode));
	LeftClientViewTreeWidgetItemMetaInfo metaInfoSwc{};
	metaInfoSwc.type = MetaInfoType::eFreeSwcContainer;
	m_TopSwcItem->setData(0, Qt::UserRole, QVariant::fromValue(metaInfoSwc));

	m_TopDailyStatisticsItem = new QTreeWidgetItem(m_TreeWidget);
	m_TopDailyStatisticsItem->setText(0, "Daily Statistics");
	m_TopDailyStatisticsItem->setIcon(0, QIcon(Image::ImageDaily));
	LeftClientViewTreeWidgetItemMetaInfo metaInfoDailyStatistic{};
	metaInfoDailyStatistic.type = MetaInfoType::eDailyStatisticsContainer;
	m_TopDailyStatisticsItem->setData(
		0, Qt::UserRole, QVariant::fromValue(metaInfoDailyStatistic));

	m_TreeWidget->addTopLevelItem(m_TopProjectItem);
	m_TreeWidget->addTopLevelItem(m_TopSwcItem);
	m_TreeWidget->addTopLevelItem(m_TopDailyStatisticsItem);

	auto rawHeadPhotoBinData =
		CachedProtoData::getInstance().CachedUserMetaInfo.headphotobindata();
	auto headPhotoBinData = QByteArray::fromStdString(rawHeadPhotoBinData);

	if (headPhotoBinData.size() > 0) {
		QPixmap pixmap;
		pixmap.loadFromData(headPhotoBinData);
		pixmap = pixmap.scaled(QSize(128, 128), Qt::KeepAspectRatioByExpanding);
		m_AccountBtn->setIcon(pixmap);
	} else {
		m_AccountBtn->setIcon(QIcon(Image::ImageDefaultUserHeadPhoto));
	}
}
