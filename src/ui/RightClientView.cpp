#include "RightClientView.h"

#include <QMenu>

#include "EditorDailyStatisticsMetaInfo.h"
#include "EditorProjectMetaInfo.h"
#include "EditorSwcMetaInfo.h"
#include "EditorSwcNode.h"
#include "MainWindow.h"
#include "ViewSwcNodeData.h"
#include "src/framework/defination/ImageDefination.h"
#include "src/framework/service/WrappedCall.h"
#include "ui_RightClientView.h"

RightClientView::RightClientView(MainWindow *mainWindow)
	: QWidget(mainWindow), ui(new Ui::RightClientView) {
	ui->setupUi(this);
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
						// mainWindow->getLeftClientView().refreshTree();
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
	m_MainLayout->addWidget(m_TabWidget);
	this->setLayout(m_MainLayout);
}

RightClientView::~RightClientView() { delete ui; }

void RightClientView::openProjectMetaInfo(const std::string &projectUuid) {
	auto index = findIfTabAlreadOpenned(projectUuid, MetaInfoType::eProject);

	proto::GetProjectResponse response;

	if (index != -1) {
		m_TabWidget->setCurrentIndex(index);

		auto editor = m_TabWidget->widget(index);
		if (!editor) {
			return;
		}
		auto base = dynamic_cast<EditorBase *>(editor);
		if (!base) {
			return;
		}
		if (WrappedCall::getProjectMetaInfoByUuid(projectUuid, response,
												  this)) {
			auto projectEditor = dynamic_cast<EditorProjectMetaInfo *>(base);
			if (!projectEditor) {
				return;
			}
			projectEditor->refresh(response);
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
	auto index = findIfTabAlreadOpenned(projectUuid, MetaInfoType::eProject);
	if (index != -1) {
		m_TabWidget->removeTab(index);
	}
}

void RightClientView::refreshProjectMetaInfo(const std::string &projectUuid) {
	auto index = findIfTabAlreadOpenned(projectUuid, MetaInfoType::eProject);

	proto::GetProjectResponse response;

	if (index != -1) {
		auto editor = m_TabWidget->widget(index);
		if (!editor) {
			return;
		}
		auto base = dynamic_cast<EditorBase *>(editor);
		if (!base) {
			return;
		}
		if (WrappedCall::getProjectMetaInfoByUuid(projectUuid, response,
												  this)) {
			auto projectEditor = dynamic_cast<EditorProjectMetaInfo *>(base);
			if (!projectEditor) {
				return;
			}
			projectEditor->refresh(response);
		}
	}
}

void RightClientView::openSwcMetaInfo(const std::string &swcUuid) {
	auto index = findIfTabAlreadOpenned(swcUuid, MetaInfoType::eFreeSwc);

	proto::GetSwcMetaInfoResponse response;
	if (index != -1) {
		m_TabWidget->setCurrentIndex(index);

		auto editor = m_TabWidget->widget(index);
		if (!editor) {
			return;
		}
		auto base = dynamic_cast<EditorBase *>(editor);
		if (!base) {
			return;
		}
		if (WrappedCall::getSwcMetaInfoByUuid(swcUuid, response, this)) {
			auto swcEditor = dynamic_cast<EditorSwcMetaInfo *>(base);
			if (!swcEditor) {
				return;
			}
			swcEditor->refresh(response);
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

void RightClientView::closeWithoutSavingSwc(const std::string &projectUuid) {
	auto index = findIfTabAlreadOpenned(projectUuid, MetaInfoType::eFreeSwc);
	if (index != -1) {
		m_TabWidget->removeTab(index);
	}
}

void RightClientView::refreshSwcMetaInfo(const std::string &swcUuid) {
	auto index = findIfTabAlreadOpenned(swcUuid, MetaInfoType::eFreeSwc);

	proto::GetSwcMetaInfoResponse response;
	if (index != -1) {
		auto editor = m_TabWidget->widget(index);
		if (!editor) {
			return;
		}
		auto base = dynamic_cast<EditorBase *>(editor);
		if (!base) {
			return;
		}
		if (WrappedCall::getSwcMetaInfoByUuid(swcUuid, response, this)) {
			auto swcEditor = dynamic_cast<EditorSwcMetaInfo *>(base);
			if (!swcEditor) {
				return;
			}
			swcEditor->refresh(response);
		}
	}
}

void RightClientView::openDailyStatisticsMetaInfo(
	const std::string &dailyStatisticsName) {
	auto index = findIfTabAlreadOpenned(dailyStatisticsName,
										MetaInfoType::eDailyStatistics);

	proto::GetDailyStatisticsResponse response;

	if (index != -1) {
		m_TabWidget->setCurrentIndex(index);

		auto editor = m_TabWidget->widget(index);
		if (!editor) {
			return;
		}
		auto base = dynamic_cast<EditorBase *>(editor);
		if (!base) {
			return;
		}
		if (WrappedCall::getDailyStatisticsmMetaInfoByName(dailyStatisticsName,
														   response, this)) {
			auto dailyStatisticsEditor =
				dynamic_cast<EditorDailyStatisticsMetaInfo *>(base);
			if (!dailyStatisticsEditor) {
				return;
			}
			dailyStatisticsEditor->refresh(response);
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
	auto index = findIfTabAlreadOpenned(dailyStatisticsName,
										MetaInfoType::eDailyStatistics);
	if (index != -1) {
		m_TabWidget->removeTab(index);
	}
}

void RightClientView::refreshDailyStatisticsMetaInfo(
	const std::string &dailyStatisticsName) {
	auto index = findIfTabAlreadOpenned(dailyStatisticsName,
										MetaInfoType::eDailyStatistics);

	proto::GetDailyStatisticsResponse response;

	if (index != -1) {
		auto editor = m_TabWidget->widget(index);
		if (!editor) {
			return;
		}
		auto base = dynamic_cast<EditorBase *>(editor);
		if (!base) {
			return;
		}
		if (WrappedCall::getDailyStatisticsmMetaInfoByName(dailyStatisticsName,
														   response, this)) {
			auto dailyStatisticsEditor =
				dynamic_cast<EditorDailyStatisticsMetaInfo *>(base);
			if (!dailyStatisticsEditor) {
				return;
			}
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

int RightClientView::findIfTabAlreadOpenned(const std::string &swcUuid,
											MetaInfoType metaInfoType) {
	for (int i = 0; i < m_TabWidget->count(); i++) {
		auto editorBase = dynamic_cast<EditorBase *>(m_TabWidget->widget(i));
		if (!editorBase) {
			continue;
		}

		if (editorBase->getUuid() == swcUuid &&
			editorBase->getMetaInfoType() == metaInfoType) {
			return i;
		}
	}
	return -1;
}

void RightClientView::openSwcNodeData(const std::string &swcUuid) {
	auto index = findIfTabAlreadOpenned(swcUuid, MetaInfoType::eSwcData);

	if (index != -1) {
		m_TabWidget->setCurrentIndex(index);

		auto qeditor = m_TabWidget->widget(index);
		if (!qeditor) {
			return;
		}
		auto base = dynamic_cast<EditorBase *>(qeditor);
		if (!base) {
			return;
		}
		auto editor = dynamic_cast<EditorSwcNode *>(base);
		if (!editor) {
			return;
		}
		editor->refreshUserArea();
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
	auto index = findIfTabAlreadOpenned(swcUuid, MetaInfoType::eSwcData);
	if (index != -1) {
		m_TabWidget->removeTab(index);
	}
}
