#include "LeftClientView.h"

#include <ui_ViewCreateSwc.h>

#include "CreateSwcSnapshotView.h"
#include "EditorAttachmentView.h"
#include "EditorBatchManageSwc.h"
#include "EditorPermission.h"
#include "EditorSwcIncrementRecord.h"
#include "EditorSwcSnapshot.h"
#include "EditorSwcVersionControl.h"
#include "MainWindow.h"
#include "ProgressBar.h"
#include "ViewCreateProject.h"
#include "ViewCreateSwc.h"
#include "ViewExportSwcToFile.h"
#include "ViewImportSwcFromFile.h"
#include "src/framework/config/AppConfig.h"
#include "src/framework/defination/ImageDefination.h"
#include "src/framework/service/CachedProtoData.h"
#include "src/framework/service/RpcCall.h"
#include "src/framework/service/WrappedCall.h"
#include "src/ui/EditorUserSettings.h"

LeftClientView::LeftClientView(MainWindow* mainWindow)
	: QWidget(mainWindow) {
	m_MainWindow = mainWindow;

	m_ControlBtnLayout = new QHBoxLayout;
	m_ControlBtnLayout->setContentsMargins(4, 8, 4, 4);
	m_ControlBtnLayout->setSpacing(6);

	m_UserSettingBtn = new QPushButton(this);
	m_UserSettingBtn->setText("User Settings");
	m_UserSettingBtn->setIcon(QIcon(Image::ImageUser));

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

	updateHeadPhoto();

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
	m_MainLayout->setContentsMargins(6, 6, 2, 6);
	m_MainLayout->setSpacing(8);
	m_MainLayout->addLayout(m_ControlBtnLayout);
	m_MainLayout->addWidget(m_TreeWidget);
	this->setLayout(m_MainLayout);

	refreshTree();
}

LeftClientView::~LeftClientView() = default;

void LeftClientView::updateHeadPhoto() {
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

void LeftClientView::getProjectMetaInfo() {
	proto::GetAllProjectResponse response;
	WrappedCall::getAllProjectMetaInfo(response, this);
	auto projectInfoList = response.mutable_projectinfo();
	std::sort(
		projectInfoList->begin(), projectInfoList->end(),
		[](const proto::ProjectMetaInfoV1& a,
		   const proto::ProjectMetaInfoV1& b) { return a.name() < b.name(); });
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
		if (WrappedCall::GetProjectSwcNamesByProjectUuid(
				projectInfo.base().uuid(), swcNamesResponse, this)) {
			std::sort(
				swcNamesResponse.mutable_swcuuidname()->begin(),
				swcNamesResponse.mutable_swcuuidname()->end(),
				[](const proto::SwcUuidName& a, const proto::SwcUuidName& b) {
					return a.swcname() < b.swcname();
				});
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
	proto::GetAllFreeSwcMetaInfoResponse response;
	WrappedCall::getAllFreeSwcMetaInfo(response, this);

	auto swcMetaInfo = response.mutable_swcuuidname();
	std::sort(swcMetaInfo->begin(), swcMetaInfo->end(),
			  [](const proto::SwcUuidName& a, const proto::SwcUuidName& b) {
				  return a.swcname() < b.swcname();
			  });
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
	std::sort(dailyStatisticsMetaInfoList->begin(),
			  dailyStatisticsMetaInfoList->end(),
			  [](const proto::DailyStatisticsMetaInfoV1& a,
				 const proto::DailyStatisticsMetaInfoV1& b) {
				  return a.name() < b.name();
			  });
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

	switch (data.type) {
		case MetaInfoType::eProjectContainer:
			buildProjectContainerMenu(popMenu, data);
			break;
		case MetaInfoType::eProject:
			buildProjectMenu(popMenu, data, curItem);
			break;
		case MetaInfoType::eFreeSwcContainer:
			buildFreeSwcContainerMenu(popMenu, data);
			break;
		case MetaInfoType::eFreeSwc:
		case MetaInfoType::eProjectSwc:
			buildSwcItemMenu(popMenu, data);
			break;
		case MetaInfoType::eDailyStatistics:
			buildDailyStatisticsMenu(popMenu, data, curItem);
			break;
		default:
			break;
	}

	popMenu->exec(QCursor::pos());
}

void LeftClientView::buildProjectContainerMenu(
	QMenu* menu, const LeftClientViewTreeWidgetItemMetaInfo& data) {
	auto* action = new QAction("Create Project", this);
	action->setIcon(QIcon(Image::ImageCreate));
	connect(action, &QAction::triggered, this, [this](bool checked) {
		ViewCreateProject view;
		view.exec();
		refreshTree();
	});
	menu->addAction(action);
}

void LeftClientView::buildProjectMenu(
	QMenu* menu, const LeftClientViewTreeWidgetItemMetaInfo& data,
	QTreeWidgetItem* curItem) {
	auto* editProject = new QAction("Edit Project", this);
	editProject->setIcon(QIcon(Image::ImageEdit));
	connect(editProject, &QAction::triggered, this,
			[this, data](bool checked) {
				m_MainWindow->getRightClientView().openProjectMetaInfo(
					data.uuid);
			});

	auto* batchManage = new QAction("Batch Manage Swc", this);
	batchManage->setIcon(QIcon(Image::ImageBatchManage));
	connect(batchManage, &QAction::triggered, this,
			[this, data](bool checked) {
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
			});

	auto* editPermission = new QAction("Edit Permission", this);
	editPermission->setIcon(QIcon(Image::ImageACL));
	connect(editPermission, &QAction::triggered, this,
			[this, data](bool checked) {
				EditorPermission view(data.uuid, MetaInfoType::eProject, false,
									  this);
				view.exec();
			});

	auto* importSwc = new QAction("Import Swc File", this);
	importSwc->setIcon(QIcon(Image::ImageImport));
	connect(importSwc, &QAction::triggered, this, [this, data](bool checked) {
		ViewImportSwcFromFile view(m_MainWindow, data.uuid);
		view.exec();
		refreshTree();
	});

	auto* exportSwc = new QAction("Export Swc File", this);
	exportSwc->setIcon(QIcon(Image::ImageExport));
	connect(
		exportSwc, &QAction::triggered, this, [this, data](bool checked) {
			auto result = QMessageBox::information(
				this, "Info",
				"Exporting action may takes few time please waiting for export "
				"steps finish!",
				QMessageBox::StandardButton::Ok,
				QMessageBox::StandardButton::Cancel);
			if (result == QMessageBox::StandardButton::Ok) {
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

				std::sort(
					dataList.begin(), dataList.end(),
					[](const ExportSwcData& a, const ExportSwcData& b) {
						return a.swcMetaInfo.name() < b.swcMetaInfo.name();
					});

				ViewExportSwcToFile view(dataList, true, this);
				view.exec();
			}
		});

	auto* exportDefinedSoma = new QAction("Export Defined Soma Swc", this);
	exportDefinedSoma->setIcon(QIcon(Image::ImageExport));
	connect(
		exportDefinedSoma, &QAction::triggered, this,
		[this, data](bool checked) {
			auto result = QMessageBox::information(
				this, "Info",
				"Exporting defined soma SWC may take some time, please wait "
				"for export steps to finish!",
				QMessageBox::StandardButton::Ok,
				QMessageBox::StandardButton::Cancel);
			if (result == QMessageBox::StandardButton::Ok) {
				ProgressBar progressBar(this);
				progressBar.setText("Loading defined soma SWC data...");
				progressBar.setValue(0);
				progressBar.show();
				QApplication::processEvents();

				progressBar.setText(
					"Getting project defined soma SWC UUIDs...");
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

				progressBar.setText("Getting SWC meta info...");
				progressBar.setValue(50);
				QApplication::processEvents();

				std::vector<ExportSwcData> dataList;

				int processedCount = 0;
				int totalCount = somaSwcResponse.swcuuids_size();
				for (const auto& swcUuid : somaSwcResponse.swcuuids()) {
					progressBar.setText(
						QString("Getting SWC meta info... (%1/%2)")
							.arg(processedCount + 1)
							.arg(totalCount)
							.toStdString());
					progressBar.setValue(
						50 + (processedCount * 30 / totalCount));
					QApplication::processEvents();

					proto::GetSwcMetaInfoResponse response;

					if (WrappedCall::getSwcMetaInfoByUuid(swcUuid, response,
														  this)) {
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
				std::sort(
					dataList.begin(), dataList.end(),
					[](const ExportSwcData& a, const ExportSwcData& b) {
						return a.swcMetaInfo.name() < b.swcMetaInfo.name();
					});

				progressBar.setText("Preparing export dialog...");
				progressBar.setValue(100);
				QApplication::processEvents();

				progressBar.finish();

				ViewExportSwcToFile view(dataList, true, this);
				view.exec();
			}
		});

	auto* deleteProject = new QAction("Delete Project", this);
	deleteProject->setIcon(QIcon(Image::ImageDelete));
	connect(deleteProject, &QAction::triggered, this,
			[this, data](bool checked) {
				auto result = QMessageBox::information(
					this, "Warning",
					"Are you sure to delete this project? "
					"This operation cannot be revert!",
					QMessageBox::StandardButton::Ok,
					QMessageBox::StandardButton::Cancel);
				if (result == QMessageBox::StandardButton::Ok) {
					proto::DeleteProjectResponse response;
					if (WrappedCall::DeleteProject(data.uuid, response, this)) {
						QMessageBox::information(this, "Info",
												 "Delete Project successfully!");
						m_MainWindow->getRightClientView()
							.closeWithoutSavingProject(data.uuid);
						refreshTree();
					}
				}
			});

	menu->addAction(editProject);
	menu->addAction(batchManage);
	menu->addSeparator();
	menu->addAction(editPermission);
	menu->addSeparator();
	menu->addAction(importSwc);
	menu->addAction(exportSwc);
	menu->addAction(exportDefinedSoma);
	menu->addSeparator();
	menu->addAction(deleteProject);
}

void LeftClientView::buildFreeSwcContainerMenu(
	QMenu* menu, const LeftClientViewTreeWidgetItemMetaInfo& data) {
	auto* importSwc = new QAction("Import Swc File", this);
	importSwc->setIcon(QIcon(Image::ImageImport));
	connect(importSwc, &QAction::triggered, this, [this](bool checked) {
		ViewImportSwcFromFile view(m_MainWindow, "");
		view.exec();
		refreshTree();
	});

	auto* exportSwc = new QAction("Export Swc File", this);
	exportSwc->setIcon(QIcon(Image::ImageExport));
	connect(exportSwc, &QAction::triggered, this, [this](bool checked) {
		auto result = QMessageBox::information(
			this, "Info",
			"Exporting action may takes few time please waiting for export "
			"steps finish!",
			QMessageBox::StandardButton::Ok,
			QMessageBox::StandardButton::Cancel);
		if (result == QMessageBox::StandardButton::Ok) {
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

			std::sort(dataList.begin(), dataList.end(),
					  [](const ExportSwcData& a, const ExportSwcData& b) {
						  return a.swcMetaInfo.name() < b.swcMetaInfo.name();
					  });

			ViewExportSwcToFile view(dataList, true, this);
			view.exec();
		}
	});

	auto* createSwc = new QAction("Create Swc", this);
	createSwc->setIcon(QIcon(Image::ImageCreate));
	connect(createSwc, &QAction::triggered, this, [this](bool checked) {
		ViewCreateSwc view;
		view.exec();
		refreshTree();
	});

	auto* batchManage = new QAction("Batch Manage Swc", this);
	batchManage->setIcon(QIcon(Image::ImageBatchManage));
	connect(batchManage, &QAction::triggered, this, [this](bool checked) {
		std::vector<std::string> uuids;
		proto::GetAllFreeSwcMetaInfoResponse response;
		WrappedCall::getAllFreeSwcMetaInfo(response, this);

		for (auto& info : response.swcuuidname()) {
			uuids.push_back(info.swcuuid());
		}

		EditorBatchManageSwc view(uuids, this);
		view.exec();
		refreshTree();
	});

	menu->addAction(importSwc);
	menu->addAction(exportSwc);
	menu->addSeparator();
	menu->addAction(createSwc);
	menu->addSeparator();
	menu->addAction(batchManage);
}

void LeftClientView::buildSwcItemMenu(
	QMenu* menu, const LeftClientViewTreeWidgetItemMetaInfo& data) {
	auto* editSwc = new QAction("Edit Swc", this);
	editSwc->setIcon(QIcon(Image::ImageEdit));
	connect(editSwc, &QAction::triggered, this, [this, data](bool checked) {
		m_MainWindow->getRightClientView().openSwcMetaInfo(data.uuid);
	});

	auto* editSwcNodeData = new QAction("Edit SwcNodeData", this);
	editSwcNodeData->setIcon(QIcon(Image::ImageEdit));
	connect(editSwcNodeData, &QAction::triggered, this,
			[this, data](bool checked) {
				m_MainWindow->getRightClientView().openSwcNodeData(data.uuid);
			});

	auto* editPermission = new QAction("Edit Permission", this);
	editPermission->setIcon(QIcon(Image::ImageACL));
	connect(editPermission, &QAction::triggered, this,
			[this, data](bool checked) {
				EditorPermission view(data.uuid, MetaInfoType::eFreeSwc, false,
									  this);
				view.exec();
			});

	auto* editAttachment = new QAction("Edit Attachment", this);
	editAttachment->setIcon(QIcon(Image::ImageAttachment));
	connect(editAttachment, &QAction::triggered, this,
			[this, data](bool checked) {
				EditorAttachmentView view(data.uuid, this);
				view.exec();
			});

	auto* exportSwc = new QAction("Export Swc File", this);
	exportSwc->setIcon(QIcon(Image::ImageExport));
	connect(exportSwc, &QAction::triggered, this, [this, data](bool checked) {
		auto result = QMessageBox::information(
			this, "Info",
			"Exporting action may takes few time please waiting for export "
			"steps finish!",
			QMessageBox::StandardButton::Ok,
			QMessageBox::StandardButton::Cancel);
		if (result == QMessageBox::StandardButton::Ok) {
			proto::GetSwcMetaInfoResponse response1;
			if (!WrappedCall::getSwcMetaInfoByUuid(data.uuid, response1,
												   this)) {
				return;
			}

			proto::GetSwcFullNodeDataResponse response2;
			if (!WrappedCall::getSwcFullNodeDataByUuid(data.uuid, response2,
													   this)) {
				return;
			}

			std::vector<ExportSwcData> dataList;
			ExportSwcData exportSwcData;
			exportSwcData.swcData = response2.swcnodedata();
			exportSwcData.swcMetaInfo = response1.swcinfo();
			dataList.push_back(exportSwcData);

			std::sort(dataList.begin(), dataList.end(),
					  [](const ExportSwcData& a, const ExportSwcData& b) {
						  return a.swcMetaInfo.name() < b.swcMetaInfo.name();
					  });

			ViewExportSwcToFile view(dataList, false, this);
			view.exec();
		}
	});

	auto* createSnapshot = new QAction("Create Snapshot", this);
	createSnapshot->setIcon(QIcon(Image::ImageSnapshot));
	connect(createSnapshot, &QAction::triggered, this,
			[this, data](bool checked) {
				CreateSwcSnapshotView view(data.uuid, this);
				view.exec();
			});

	auto* viewSnapshot = new QAction("View Snapshot", this);
	viewSnapshot->setIcon(QIcon(Image::ImageSnapshot));
	connect(viewSnapshot, &QAction::triggered, this,
			[this, data](bool checked) {
				EditorSwcSnapshot view(data.uuid, this);
				view.exec();
			});

	auto* viewIncrement = new QAction("View Increment Record", this);
	viewIncrement->setIcon(QIcon(Image::ImageIncrement));
	connect(viewIncrement, &QAction::triggered, this,
			[this, data](bool checked) {
				EditorSwcIncrementRecord view(data.uuid, this);
				view.exec();
			});

	auto* versionControl = new QAction("Version Control", this);
	versionControl->setIcon(QIcon(Image::ImageVersionControl));
	connect(versionControl, &QAction::triggered, this,
			[data](bool checked) {
				auto view = new EditorSwcVersionControl(data.uuid);
				view->setAttribute(Qt::WA_DeleteOnClose);
				view->exec();
			});

	auto* deleteSwc = new QAction("Delete Swc", this);
	deleteSwc->setIcon(QIcon(Image::ImageDelete));
	connect(deleteSwc, &QAction::triggered, this, [this, data](bool checked) {
		auto result =
			QMessageBox::information(this, "Warning",
									 "Are you sure to delete this Swc? "
									 "This operation cannot be revert!",
									 QMessageBox::StandardButton::Ok,
									 QMessageBox::StandardButton::Cancel);
		if (result == QMessageBox::StandardButton::Ok) {
			proto::DeleteSwcResponse response;
			if (WrappedCall::DeleteSwc(data.uuid, response, this)) {
				QMessageBox::information(this, "Info",
										 "Delete Swc successfully!");
				m_MainWindow->getRightClientView().closeWithoutSavingSwc(
					data.uuid);
				m_MainWindow->getRightClientView()
					.refreshAllOpenedProjectMetaInfo();
				refreshTree();
			}
		}
	});

	auto* swcFeatureAnalysis = new QAction("SWC Feature Analysis", this);
	swcFeatureAnalysis->setIcon(QIcon(Image::ImageTool));
	connect(swcFeatureAnalysis, &QAction::triggered, this,
			[this, data](bool checked) {
				m_MainWindow->getRightClientView().openSwcFeatureAnalysis(
					data.uuid, data.name);
			});

	auto* qualityControl = new QAction("Quality Control", this);
	qualityControl->setIcon(QIcon(Image::ImageTool));
	connect(qualityControl, &QAction::triggered, this,
			[this, data](bool checked) {
				m_MainWindow->getRightClientView().openQualityControl(
					data.uuid, data.name);
			});

	menu->addAction(editSwc);
	menu->addAction(editSwcNodeData);
	menu->addAction(editPermission);
	menu->addAction(editAttachment);
	menu->addSeparator();
	menu->addAction(exportSwc);
	menu->addSeparator();
	menu->addAction(createSnapshot);
	menu->addAction(viewSnapshot);
	menu->addAction(viewIncrement);
	menu->addAction(versionControl);
	menu->addSeparator();
	menu->addAction(swcFeatureAnalysis);
	menu->addAction(qualityControl);
	menu->addSeparator();
	menu->addAction(deleteSwc);
}

void LeftClientView::buildDailyStatisticsMenu(
	QMenu* menu, const LeftClientViewTreeWidgetItemMetaInfo& data,
	QTreeWidgetItem* curItem) {
	auto* editDailyStatistics = new QAction("Edit DailyStatistics", this);
	editDailyStatistics->setIcon(QIcon(Image::ImageDelete));
	connect(editDailyStatistics, &QAction::triggered, this,
			[this, data](bool checked) {
				m_MainWindow->getRightClientView().openDailyStatisticsMetaInfo(
					data.name);
			});

	auto* deleteDailyStatistics =
		new QAction("Delete DailyStatistics", this);
	deleteDailyStatistics->setIcon(QIcon(Image::ImageDelete));
	connect(
		deleteDailyStatistics, &QAction::triggered, this,
		[this, data, curItem](bool checked) {
			auto result = QMessageBox::information(
				this, "Warning",
				"Are you sure to delete this DailyStatistics? This operation "
				"cannot be revert!",
				QMessageBox::StandardButton::Ok,
				QMessageBox::StandardButton::Cancel);
			if (result == QMessageBox::StandardButton::Ok) {
				proto::DeleteDailyStatisticsResponse response;
				if (WrappedCall::DeleteDailyStatistics(
						curItem->text(0).toStdString(), response, this)) {
					QMessageBox::information(
						this, "Info",
						"Delete DailyStatistics successfully!");
					m_MainWindow->getRightClientView()
						.closeWithoutSavingDailyStatistics(data.uuid);
					refreshTree();
				}
			}
		});

	menu->addAction(editDailyStatistics);
	menu->addSeparator();
	menu->addAction(deleteDailyStatistics);
}

void LeftClientView::refreshTree() {
	clearAll();
	getProjectMetaInfo();
	getFreeSwcMetaInfo();
	getAllDailyStatisticsMetaInfo();
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

	updateHeadPhoto();
}
