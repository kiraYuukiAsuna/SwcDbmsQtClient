#include "rightclientview.h"

#include "editordailystatisticsmetainfo.h"
#include "ui_RightClientView.h"
#include "mainwindow.h"
#include "src/framework/service/WrappedCall.h"
#include "editorprojectmetainfo.h"
#include "editorswcmetainfo.h"
#include "src/framework/defination/ImageDefination.h"
#include "viewswcnodedata.h"
#include "editorswcnode.h"

RightClientView::RightClientView(MainWindow* mainWindow) : QWidget(mainWindow), ui(new Ui::RightClientView) {
    ui->setupUi(this);
    m_MainWindow = mainWindow;

    m_TabWidget = new QTabWidget(this);
    m_TabWidget->setTabsClosable(true);
    connect(m_TabWidget, &QTabWidget::tabCloseRequested, this, [this](int index) {
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

    m_MainLayout = new QVBoxLayout(this);
    m_MainLayout->addWidget(m_TabWidget);
    this->setLayout(m_MainLayout);
}

RightClientView::~RightClientView() {
    delete ui;
}

void RightClientView::openProjectMetaInfo(const std::string& projectName) {
    auto index = findIfTabAlreadOpenned(projectName, MetaInfoType::eProject);

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
        if (WrappedCall::getProjectMetaInfoByName(projectName, response, this)) {
            auto projectEditor = dynamic_cast<EditorProjectMetaInfo *>(base);
            if (!projectEditor) {
                return;
            }
            projectEditor->refresh(response);
        }
        return;
    }

    if (WrappedCall::getProjectMetaInfoByName(projectName, response, this)) {
        auto* editor = new EditorProjectMetaInfo(response, m_TabWidget);
        auto newIndex = m_TabWidget->addTab(editor, QIcon(Image::ImageProject),
                                            QString::fromStdString(response.projectinfo().name()));
        m_TabWidget->setCurrentIndex(newIndex);
    }
}

void RightClientView::closeWithoutSavingProject(const std::string& projectName) {
    auto index = findIfTabAlreadOpenned(projectName, MetaInfoType::eProject);
    if(index!=-1) {
        m_TabWidget->removeTab(index);
    }
}

void RightClientView::refreshProjectMetaInfo(const std::string& projectName) {
    auto index = findIfTabAlreadOpenned(projectName, MetaInfoType::eProject);

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
        if (WrappedCall::getProjectMetaInfoByName(projectName, response, this)) {
            auto projectEditor = dynamic_cast<EditorProjectMetaInfo *>(base);
            if (!projectEditor) {
                return;
            }
            projectEditor->refresh(response);
        }
    }
}

void RightClientView::openSwcMetaInfo(const std::string& swcName) {
    auto index = findIfTabAlreadOpenned(swcName, MetaInfoType::eSwc);

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
        if (WrappedCall::getSwcMetaInfoByName(swcName, response, this)) {
            auto swcEditor = dynamic_cast<EditorSwcMetaInfo *>(base);
            if (!swcEditor) {
                return;
            }
            swcEditor->refresh(response);
        }
        return;
    }

    if (WrappedCall::getSwcMetaInfoByName(swcName, response, this)) {
        auto* editor = new EditorSwcMetaInfo(response, m_TabWidget);
        auto newIndex = m_TabWidget->addTab(editor, QIcon(Image::ImageNode),
                                            QString::fromStdString(response.swcinfo().name()));
        m_TabWidget->setCurrentIndex(newIndex);
    }
}

void RightClientView::closeWithoutSavingSwc(const std::string& swcName) {
    auto index = findIfTabAlreadOpenned(swcName, MetaInfoType::eSwc);
    if(index!=-1) {
        m_TabWidget->removeTab(index);
    }
}

void RightClientView::refreshSwcMetaInfo(const std::string& swcName) {
    auto index = findIfTabAlreadOpenned(swcName, MetaInfoType::eSwc);

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
        if (WrappedCall::getSwcMetaInfoByName(swcName, response, this)) {
            auto swcEditor = dynamic_cast<EditorSwcMetaInfo *>(base);
            if (!swcEditor) {
                return;
            }
            swcEditor->refresh(response);
        }
    }
}

void RightClientView::openDailyStatisticsMetaInfo(const std::string& dailyStatisticsName) {
    auto index = findIfTabAlreadOpenned(dailyStatisticsName, MetaInfoType::eDailyStatistics);

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
        if (WrappedCall::getDailyStatisticsmMetaInfoByName(dailyStatisticsName, response, this)) {
            auto dailyStatisticsEditor = dynamic_cast<EditorDailyStatisticsMetaInfo *>(base);
            if (!dailyStatisticsEditor) {
                return;
            }
            dailyStatisticsEditor->refresh(response);
        }
        return;
    }

    if (WrappedCall::getDailyStatisticsmMetaInfoByName(dailyStatisticsName, response, this)) {
        auto* editor = new EditorDailyStatisticsMetaInfo(response, m_TabWidget);
        auto newIndex = m_TabWidget->addTab(editor, QIcon(Image::ImageDaily),
                                            QString::fromStdString(response.dailystatisticsinfo().name()));
        m_TabWidget->setCurrentIndex(newIndex);
    }
}

void RightClientView::closeWithoutSavingDailyStatistics(const std::string& dailyStatisticsName) {
    auto index = findIfTabAlreadOpenned(dailyStatisticsName, MetaInfoType::eDailyStatistics);
    if(index!=-1) {
        m_TabWidget->removeTab(index);
    }
}

void RightClientView::refreshDailyStatisticsMetaInfo(const std::string& dailyStatisticsName) {
    auto index = findIfTabAlreadOpenned(dailyStatisticsName, MetaInfoType::eDailyStatistics);

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
        if (WrappedCall::getDailyStatisticsmMetaInfoByName(dailyStatisticsName, response, this)) {
            auto dailyStatisticsEditor = dynamic_cast<EditorDailyStatisticsMetaInfo *>(base);
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
            refreshProjectMetaInfo(editorBase->getName());
        }
    }
}

int RightClientView::findIfTabAlreadOpenned(const std::string& name, MetaInfoType metaInfoType) {
    for (int i = 0; i < m_TabWidget->count(); i++) {
        auto editorBase = dynamic_cast<EditorBase *>(m_TabWidget->widget(i));
        if (!editorBase) {
            continue;
        }

        if (editorBase->getName() == name && editorBase->getMetaInfoType() == metaInfoType) {
            return i;
        }
    }
    return -1;
}

void RightClientView::openSwcNodeData(const std::string &swcName) {
    auto index = findIfTabAlreadOpenned(swcName, MetaInfoType::eSwcData);

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
        auto editor= dynamic_cast<EditorSwcNode *>(base);
        if (!editor) {
            return;
        }
        editor->refreshUserArea();
        return;
    }

    auto* editor = new EditorSwcNode(swcName, m_TabWidget);
    auto newIndex = m_TabWidget->addTab(editor, QIcon(Image::ImageDaily),QString::fromStdString(swcName));
    m_TabWidget->setCurrentIndex(newIndex);
}

void RightClientView::closeWithoutSavingSwcNodeData(const std::string &swcName) {
    auto index = findIfTabAlreadOpenned(swcName, MetaInfoType::eSwcData);
    if(index!=-1) {
        m_TabWidget->removeTab(index);
    }
}

