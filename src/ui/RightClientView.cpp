#include "RightClientView.h"

#include <QFileInfo>
#include <QMenu>

#include "EditorDailyStatisticsMetaInfo.h"
#include "EditorQualityControl.h"
#include "EditorSwcFeature.h"
#include "EditorProjectMetaInfo.h"
#include "EditorSwcMetaInfo.h"
#include "EditorSwcNode.h"
#include "MainWindow.h"
#include "ViewSwcNodeData.h"
#include "src/framework/defination/ImageDefination.h"
#include "src/framework/service/WrappedCall.h"

RightClientView::RightClientView(MainWindow *mainWindow)
	: QWidget(mainWindow) {
	m_MainWindow = mainWindow;

	m_TabWidget = new QTabWidget(this);
	m_TabWidget->setTabsClosable(true);
	connect(m_TabWidget, &QTabWidget::tabCloseRequested, this,
			[this, mainWindow](int index) {
				auto editor = m_TabWidget->widget(index);

				if (!editor) {
					return;
				}

				auto base = dynamic_cast<EditorBase *>(editor);
				if (base) {
					if (base->save()) {
						m_TabWidget->removeTab(index);
					}
				} else {
					m_TabWidget->removeTab(index);
				}
			});

	m_TabWidget->tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(
		m_TabWidget->tabBar(), &QTabBar::customContextMenuRequested, this,
		[this](const QPoint &pos) {
			auto editor =
				dynamic_cast<EditorBase *>(m_TabWidget->currentWidget());
			if (!editor) {
				return;
			}
			auto menu = new QMenu(this);
			auto saveAction = new QAction("Save", this);
			connect(saveAction, &QAction::triggered, this,
					[this, editor] { editor->save(); });

			auto closeAction = new QAction("Close", this);
			connect(closeAction, &QAction::triggered, this,
					[this, editor, pos] {
						editor->save();
						auto index = m_TabWidget->tabBar()->tabAt(pos);
						m_TabWidget->removeTab(index);
					});

			auto closeAllAction = new QAction("Close All", this);
			connect(closeAllAction, &QAction::triggered, this, [this, editor] {
				for (int i = m_TabWidget->count() - 1; i >= 0; i--) {
					auto editor1 =
						dynamic_cast<EditorBase *>(m_TabWidget->widget(i));
					if (editor1) {
						editor1->save();
						m_TabWidget->removeTab(i);
					}
				}
			});

			menu->addAction(saveAction);
			menu->addAction(closeAction);
			menu->addAction(closeAllAction);
			menu->exec(QCursor::pos());
		});

	m_MainLayout = new QVBoxLayout(this);
	m_MainLayout->setContentsMargins(2, 6, 6, 6);
	m_MainLayout->setSpacing(0);
	m_MainLayout->addWidget(m_TabWidget);
	this->setLayout(m_MainLayout);
}

RightClientView::~RightClientView() = default;

int RightClientView::findIfTabAlreadyOpened(const std::string &identifier,
											MetaInfoType metaInfoType) {
	for (int i = 0; i < m_TabWidget->count(); i++) {
		auto editorBase = dynamic_cast<EditorBase *>(m_TabWidget->widget(i));
		if (!editorBase) {
			continue;
		}

		if (editorBase->getUuid() == identifier &&
			editorBase->getMetaInfoType() == metaInfoType) {
			return i;
		}
	}
	return -1;
}

void RightClientView::closeTab(const std::string &identifier,
							   MetaInfoType type) {
	auto index = findIfTabAlreadyOpened(identifier, type);
	if (index != -1) {
		m_TabWidget->removeTab(index);
	}
}

EditorBase *RightClientView::findEditorBase(const std::string &identifier,
											MetaInfoType type) {
	auto index = findIfTabAlreadyOpened(identifier, type);
	if (index == -1) {
		return nullptr;
	}
	return dynamic_cast<EditorBase *>(m_TabWidget->widget(index));
}

void RightClientView::openProjectMetaInfo(const std::string &projectUuid) {
	auto index = findIfTabAlreadyOpened(projectUuid, MetaInfoType::eProject);

	proto::GetProjectResponse response;

	if (index != -1) {
		m_TabWidget->setCurrentIndex(index);
		if (WrappedCall::getProjectMetaInfoByUuid(projectUuid, response,
												  this)) {
			auto projectEditor = dynamic_cast<EditorProjectMetaInfo *>(
				findEditorBase(projectUuid, MetaInfoType::eProject));
			if (projectEditor) {
				projectEditor->refresh(response);
			}
		}
		return;
	}

	if (WrappedCall::getProjectMetaInfoByUuid(projectUuid, response, this)) {
		auto *editor = new EditorProjectMetaInfo(response, m_TabWidget);
		auto newIndex = m_TabWidget->addTab(
			editor, QIcon(Image::ImageProject),
			QString::fromStdString(response.projectinfo().name()));
		m_TabWidget->setCurrentIndex(newIndex);
	}
}

void RightClientView::closeWithoutSavingProject(
	const std::string &projectUuid) {
	closeTab(projectUuid, MetaInfoType::eProject);
}

void RightClientView::refreshProjectMetaInfo(const std::string &projectUuid) {
	auto *base = findEditorBase(projectUuid, MetaInfoType::eProject);
	if (!base) {
		return;
	}
	proto::GetProjectResponse response;
	if (WrappedCall::getProjectMetaInfoByUuid(projectUuid, response, this)) {
		auto projectEditor = dynamic_cast<EditorProjectMetaInfo *>(base);
		if (projectEditor) {
			projectEditor->refresh(response);
		}
	}
}

void RightClientView::openSwcMetaInfo(const std::string &swcUuid) {
	auto index = findIfTabAlreadyOpened(swcUuid, MetaInfoType::eFreeSwc);

	proto::GetSwcMetaInfoResponse response;
	if (index != -1) {
		m_TabWidget->setCurrentIndex(index);
		if (WrappedCall::getSwcMetaInfoByUuid(swcUuid, response, this)) {
			auto swcEditor = dynamic_cast<EditorSwcMetaInfo *>(
				findEditorBase(swcUuid, MetaInfoType::eFreeSwc));
			if (swcEditor) {
				swcEditor->refresh(response);
			}
		}
		return;
	}

	if (WrappedCall::getSwcMetaInfoByUuid(swcUuid, response, this)) {
		auto *editor = new EditorSwcMetaInfo(response, m_TabWidget);
		auto newIndex = m_TabWidget->addTab(
			editor, QIcon(Image::ImageNode),
			QString::fromStdString(response.swcinfo().name()));
		m_TabWidget->setCurrentIndex(newIndex);
	}
}

void RightClientView::closeWithoutSavingSwc(const std::string &swcUuid) {
	closeTab(swcUuid, MetaInfoType::eFreeSwc);
}

void RightClientView::refreshSwcMetaInfo(const std::string &swcUuid) {
	auto *base = findEditorBase(swcUuid, MetaInfoType::eFreeSwc);
	if (!base) {
		return;
	}
	proto::GetSwcMetaInfoResponse response;
	if (WrappedCall::getSwcMetaInfoByUuid(swcUuid, response, this)) {
		auto swcEditor = dynamic_cast<EditorSwcMetaInfo *>(base);
		if (swcEditor) {
			swcEditor->refresh(response);
		}
	}
}

void RightClientView::openDailyStatisticsMetaInfo(
	const std::string &dailyStatisticsName) {
	auto index = findIfTabAlreadyOpened(dailyStatisticsName,
										MetaInfoType::eDailyStatistics);

	proto::GetDailyStatisticsResponse response;

	if (index != -1) {
		m_TabWidget->setCurrentIndex(index);
		if (WrappedCall::getDailyStatisticsmMetaInfoByName(dailyStatisticsName,
														   response, this)) {
			auto dailyStatisticsEditor =
				dynamic_cast<EditorDailyStatisticsMetaInfo *>(findEditorBase(
					dailyStatisticsName, MetaInfoType::eDailyStatistics));
			if (dailyStatisticsEditor) {
				dailyStatisticsEditor->refresh(response);
			}
		}
		return;
	}

	if (WrappedCall::getDailyStatisticsmMetaInfoByName(dailyStatisticsName,
													   response, this)) {
		auto *editor = new EditorDailyStatisticsMetaInfo(response, m_TabWidget);
		auto newIndex = m_TabWidget->addTab(
			editor, QIcon(Image::ImageDaily),
			QString::fromStdString(response.dailystatisticsinfo().name()));
		m_TabWidget->setCurrentIndex(newIndex);
	}
}

void RightClientView::closeWithoutSavingDailyStatistics(
	const std::string &dailyStatisticsName) {
	closeTab(dailyStatisticsName, MetaInfoType::eDailyStatistics);
}

void RightClientView::refreshDailyStatisticsMetaInfo(
	const std::string &dailyStatisticsName) {
	auto *base =
		findEditorBase(dailyStatisticsName, MetaInfoType::eDailyStatistics);
	if (!base) {
		return;
	}
	proto::GetDailyStatisticsResponse response;
	if (WrappedCall::getDailyStatisticsmMetaInfoByName(dailyStatisticsName,
													   response, this)) {
		auto dailyStatisticsEditor =
			dynamic_cast<EditorDailyStatisticsMetaInfo *>(base);
		if (dailyStatisticsEditor) {
			dailyStatisticsEditor->refresh(response);
		}
	}
}

void RightClientView::refreshAllOpenedProjectMetaInfo() {
	for (int i = 0; i < m_TabWidget->count(); i++) {
		auto editorBase = dynamic_cast<EditorBase *>(m_TabWidget->widget(i));
		if (!editorBase) {
			continue;
		}

		if (editorBase->getMetaInfoType() == MetaInfoType::eProject) {
			refreshProjectMetaInfo(editorBase->getUuid());
		}
	}
}

void RightClientView::openSwcNodeData(const std::string &swcUuid) {
	auto index = findIfTabAlreadyOpened(swcUuid, MetaInfoType::eSwcData);

	if (index != -1) {
		m_TabWidget->setCurrentIndex(index);
		auto editor = dynamic_cast<EditorSwcNode *>(
			findEditorBase(swcUuid, MetaInfoType::eSwcData));
		if (editor) {
			editor->refreshUserArea();
		}
		return;
	}
	proto::GetSwcMetaInfoResponse response;
	if (WrappedCall::getSwcMetaInfoByUuid(swcUuid, response, this)) {
		auto *editor = new EditorSwcNode(swcUuid, m_TabWidget);
		auto newIndex = m_TabWidget->addTab(
			editor, QIcon(Image::ImageDaily),
			QString::fromStdString(response.swcinfo().name()));
		m_TabWidget->setCurrentIndex(newIndex);
	}
}

void RightClientView::closeWithoutSavingSwcNodeData(
	const std::string &swcUuid) {
	closeTab(swcUuid, MetaInfoType::eSwcData);
}

void RightClientView::openSwcFeatureAnalysis(const std::string &swcUuid,
											 const std::string &swcName) {
	auto index = findIfTabAlreadyOpened(swcUuid, MetaInfoType::eSwcFeature);
	if (index != -1) {
		m_TabWidget->setCurrentIndex(index);
		return;
	}

	auto *editor = new EditorSwcFeature(swcUuid, swcName, m_TabWidget);
	auto newIndex = m_TabWidget->addTab(
		editor, QString::fromStdString(swcName) + " [Feature]");
	m_TabWidget->setCurrentIndex(newIndex);
}

void RightClientView::openSwcFeatureLocalFile(const QString &filePath) {
	auto identifier = filePath.toStdString();
	auto index =
		findIfTabAlreadyOpened(identifier, MetaInfoType::eSwcFeature);
	if (index != -1) {
		m_TabWidget->setCurrentIndex(index);
		return;
	}

	auto *editor = new EditorSwcFeature(filePath, m_TabWidget);
	QFileInfo fi(filePath);
	auto newIndex =
		m_TabWidget->addTab(editor, fi.fileName() + " [Feature]");
	m_TabWidget->setCurrentIndex(newIndex);
}

void RightClientView::openQualityControl(const std::string &swcUuid,
										  const std::string &swcName) {
	auto index =
		findIfTabAlreadyOpened(swcUuid, MetaInfoType::eQualityControl);
	if (index != -1) {
		m_TabWidget->setCurrentIndex(index);
		return;
	}

	auto *editor = new EditorQualityControl(swcUuid, swcName, m_TabWidget);
	auto newIndex = m_TabWidget->addTab(
		editor, QString::fromStdString(swcName) + " [QC]");
	m_TabWidget->setCurrentIndex(newIndex);
}

void RightClientView::openQualityControlLocalFile(const QString &filePath) {
	auto identifier = filePath.toStdString();
	auto index =
		findIfTabAlreadyOpened(identifier, MetaInfoType::eQualityControl);
	if (index != -1) {
		m_TabWidget->setCurrentIndex(index);
		return;
	}

	auto *editor = new EditorQualityControl(filePath, m_TabWidget);
	QFileInfo fi(filePath);
	auto newIndex = m_TabWidget->addTab(editor, fi.fileName() + " [QC]");
	m_TabWidget->setCurrentIndex(newIndex);
}
