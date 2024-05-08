#include "EditorPermissionGroup.h"

#include <qinputdialog.h>
#include <QPushButton>
#include <QVBoxLayout>

#include "ui_EditorPermissionGroup.h"
#include "src/framework/defination/ImageDefination.h"
#include "src/framework/service/WrappedCall.h"

EditorPermissionGroup::EditorPermissionGroup(QWidget* parent) : QDialog(parent), ui(new Ui::EditorPermissionGroup) {
    ui->setupUi(this);

    auto* layout = new QVBoxLayout;

    m_TreeWidget = new TreeWidget(this);
    QStringList headers;
    headers << tr("PermissionGroup");
    m_TreeWidget->getQTreeWidget()->setHeaderLabels(headers);
    layout->addWidget(m_TreeWidget);

    auto controlLayout = new QHBoxLayout;
    layout->addLayout(controlLayout);
    auto btnCreate = new QPushButton("Create New Group", this);
    auto btnDelete = new QPushButton("Delete Group", this);
    auto btnCommit = new QPushButton("Commit Changes", this);

    connect(btnCreate, &QPushButton::clicked, [&]() {
        bool bOk;
        auto newGroupName = QInputDialog::getText(this, "New Permission Group", "Please input new group name"
                                                  , QLineEdit::Normal, "NewPermissionGroupName", &bOk);
        if (!bOk) {
            return;
        }
        if (newGroupName.isEmpty()) {
            QMessageBox::information(this, "Permission Group", "Group name can not be empty");
            return;
        }

        for (auto&value: m_PermissionGroupList) {
            if (value.name() == newGroupName.toStdString()) {
                QMessageBox::information(this, "Permission Group", "Group name already exists");
                return;
            }
        }

        auto description = QInputDialog::getText(this, "New Permission Group", "Please input new group description"
                                                 , QLineEdit::Normal, "", &bOk);

        m_TreeWidget->addTopItem(newGroupName.toStdString(), newGroupName.toStdString(),
                                 QIcon(Image::ImageUserPermission), {});

        proto::CreatePermissionGroupResponse response;
        WrappedCall::CreatePermissionGroup(newGroupName.toStdString(), description.toStdString(), response, this);

        refresh();
    });
    connect(btnDelete, &QPushButton::clicked, [&]() {
        for (auto index_map: m_TreeWidget->getIndexMap()) {
            if (index_map.second == m_TreeWidget->getQTreeWidget()->currentItem()) {
                auto iter = std::find_if(m_PermissionGroupList.begin(), m_PermissionGroupList.end(),
                                         [&](const proto::PermissionGroupMetaInfoV1&value) {
                                             return value.name() == index_map.first;
                                         });
                if (iter != m_PermissionGroupList.end()) {
                    int result = QMessageBox::information(this, "Warning",
                                                          "Are you sure to delete this group? Group Name: " +
                                                          QString::fromStdString(iter->name()), QMessageBox::Yes,
                                                          QMessageBox::No);
                    if (result == QMessageBox::Yes) {
                        proto::DeletePermissionGroupResponse response;
                        WrappedCall::DeletePermissionGroup(iter->base().uuid(), response, this);
                        refresh();
                    }
                    return;
                }
                QMessageBox::information(this, "Error", "PermissionGroup Selected Not found!");
                return;
            }
        }
    });
    connect(btnCommit, &QPushButton::clicked, [&]() {
        int result = QMessageBox::information(this, "Warning",
                                              "Are you sure to commit your changes to permissions in above groups?",
                                              QMessageBox::Yes,
                                              QMessageBox::No);
        if (result != QMessageBox::Yes) {
            return;
        }

        for (auto&value: m_PermissionGroupList) {
            auto* item = m_TreeWidget->findItem(value.name());
            if (item) {
                auto descriptor = value.ace().GetDescriptor();
                for (int permission = 0; permission < descriptor->field_count(); permission++) {
                    auto name = descriptor->field(permission)->name();

                    auto permissionItem = m_TreeWidget->findItem(value.name() + name);
                    if (!permissionItem) {
                        continue;
                    }

                    bool isChecked = permissionItem->checkState(0) == Qt::Checked;

                    value.ace().GetReflection()->SetBool(value.mutable_ace(), descriptor->field(permission), isChecked);
                }
            }
            proto::UpdatePermissionGroupResponse response;
            WrappedCall::UpdatePermissionGroup(value.base().uuid(), value.name(), value.description(), value.ace(),
                                               response, this);
        }
    });

    controlLayout->addWidget(btnCreate);
    controlLayout->addWidget(btnDelete);
    controlLayout->addWidget(btnCommit);

    setLayout(layout);

    refresh();
}

EditorPermissionGroup::~EditorPermissionGroup() {
    delete ui;
}

void EditorPermissionGroup::refresh() {
    m_TreeWidget->getIndexMap().clear();
    m_TreeWidget->getQTreeWidget()->clear();

    proto::GetAllPermissionGroupResponse response;
    WrappedCall::GetAllPermissionGroup(response, this);
    m_PermissionGroupList = response.permissiongrouplist();

    for (auto&value: m_PermissionGroupList) {
        m_TreeWidget->addTopItem(value.name(), value.name(), QIcon(Image::ImageUserPermission), {});

        auto descriptor = value.ace().GetDescriptor();
        for (int permission = 0; permission < descriptor->field_count(); permission++) {
            auto name = descriptor->field(permission)->name();
            auto* item = m_TreeWidget->addItem(value.name(), value.name() + name, name,
                                               QIcon(Image::ImageUserPermission), {});

            if (value.ace().GetReflection()->GetBool(value.ace(), descriptor->field(permission))) {
                item->setCheckState(0, Qt::Checked);
            }
            else {
                item->setCheckState(0, Qt::Unchecked);
            }
        }
    }
}
