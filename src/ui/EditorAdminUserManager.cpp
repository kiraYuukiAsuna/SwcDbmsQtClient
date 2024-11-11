#include "EditorAdminUserManager.h"

#include <QInputDialog>
#include <QPushButton>
#include <QVBoxLayout>

#include "ui_EditorAdminUserManager.h"
#include "src/framework/defination/ImageDefination.h"
#include "src/framework/service/WrappedCall.h"
#include "src/framework/util/util.hpp"

EditorAdminUserManager::EditorAdminUserManager(QWidget* parent)
    : QDialog(parent), ui(new Ui::EditorAdminUserManager) {
    ui->setupUi(this);

    auto* layout = new QVBoxLayout;

    m_TreeWidget = new TreeWidget(this);
    QStringList headers;
    headers << tr("User Name");
    headers << tr("Permission Group");
    headers << tr("Register Time");
    headers << tr("User Description");
    m_TreeWidget->getQTreeWidget()->setHeaderLabels(headers);
    layout->addWidget(m_TreeWidget);

    auto controlLayout = new QHBoxLayout;
    layout->addLayout(controlLayout);
    auto btnCreate = new QPushButton("Create New User", this);
    auto btnDelete = new QPushButton("Delete User", this);
    auto btnChangePermissionGroup = new QPushButton("Change Permission Group", this);

    connect(btnCreate, &QPushButton::clicked, [&]() {
        bool bOk;
        auto newUserName = QInputDialog::getText(this, "Create User", "Please input new user name"
                                                 , QLineEdit::Normal, "NewUserName", &bOk);
        if (!bOk) {
            return;
        }
        if (newUserName.isEmpty()) {
            QMessageBox::information(this, "Create User", "User name can not be empty!");
            return;
        }

        proto::GetAllUserResponse response;
        if (!WrappedCall::getAllUserMetaInfo(response, this)) {
            return;
        }

        for (auto&value: response.userinfo()) {
            if (value.name() == newUserName.toStdString()) {
                QMessageBox::information(this, "Create User", "User name already exists!");
                return;
            }
        }

        auto description = QInputDialog::getText(this, "New User", "Please input new user description"
                                                 , QLineEdit::Normal, "", &bOk);
        if (!bOk) {
            return;
        }

        auto password = QInputDialog::getText(this, "New User", "Please input new user password"
                                              , QLineEdit::Normal, "", &bOk);
        if (!bOk) {
            return;
        }
        if (newUserName.isEmpty()) {
            QMessageBox::information(this, "Create User", "User password can not be empty!");
            return;
        }

        proto::CreateUserResponse rsp2;
        WrappedCall::CreateUser(newUserName.toStdString(), password.toStdString(), description.toStdString(), rsp2,
                                this);

        refresh();
    });

    connect(btnDelete, &QPushButton::clicked, [&]() {
        for (auto index_map: m_TreeWidget->getIndexMap()) {
            if (index_map.second == m_TreeWidget->getQTreeWidget()->currentItem()) {
                proto::GetAllUserResponse response;
                if (!WrappedCall::getAllUserMetaInfo(response, this)) {
                    return;
                }
                auto iter = std::find_if(response.userinfo().begin(), response.userinfo().end(),
                                         [&](const proto::UserMetaInfoV1&value) {
                                             return value.base().uuid() == index_map.first;
                                         });
                if (iter != response.userinfo().end()) {
                    int result = QMessageBox::information(this, "Warning",
                                                          "Are you sure to delete this user? User Name: " +
                                                          QString::fromStdString(iter->name()), QMessageBox::Yes,
                                                          QMessageBox::No);
                    if (result == QMessageBox::Yes) {
                        proto::DeleteUserResponse rsp;
                        WrappedCall::DeleteUser(iter->name(), rsp, this);
                        refresh();
                    }
                    return;
                }
                QMessageBox::information(this, "Error", "User Selected Not found!");
                return;
            }
        }
    });

    connect(btnChangePermissionGroup, &QPushButton::clicked, [&]() {
        for (auto index_map: m_TreeWidget->getIndexMap()) {
            if (index_map.second == m_TreeWidget->getQTreeWidget()->currentItem()) {
                proto::GetAllUserResponse response;
                if (!WrappedCall::getAllUserMetaInfo(response, this)) {
                    return;
                }
                auto iter = std::find_if(response.userinfo().begin(), response.userinfo().end(),
                                         [&](const proto::UserMetaInfoV1&value) {
                                             return value.base().uuid() == index_map.first;
                                         });

                if (iter != response.userinfo().end()) {
                    int result = QMessageBox::information(this, "Warning",
                                                          "Are you sure to change this user's permission group? User Name: "
                                                          +
                                                          QString::fromStdString(iter->name()), QMessageBox::Yes,
                                                          QMessageBox::No);
                    if (result == QMessageBox::Yes) {
                        proto::GetUserByUuidResponse rsp1;
                        WrappedCall::GetUserInfoByUuid(iter->base().uuid(), rsp1, this);

                        bool bOk;
                        auto permissionGroupName = QInputDialog::getText(
                            this, "Change Permission Group", "Please input permission group name"
                            , QLineEdit::Normal, "Default", &bOk);
                        if (!bOk) {
                            return;
                        }
                        if (permissionGroupName.trimmed().isEmpty()) {
                            QMessageBox::information(this, "Change Permission Group",
                                                     "Permission group name can not be empty!");
                            return;
                        }

                        proto::GetAllPermissionGroupResponse rsp2;
                        if (!WrappedCall::GetAllPermissionGroup(rsp2, this)) {
                            return;
                        }

                        auto pgIter = std::find_if(rsp2.permissiongrouplist().begin(), rsp2.permissiongrouplist().end(),
                                                   [&](const proto::PermissionGroupMetaInfoV1&value) {
                                                       return value.name() == permissionGroupName.toStdString();
                                                   });
                        if (pgIter == rsp2.permissiongrouplist().end()) {
                            QMessageBox::information(this, "Change Permission Group", "Permission group not found!");
                            return;
                        }

                        rsp1.mutable_userinfo()->set_permissiongroupuuid(pgIter->base().uuid());

                        proto::UpdateUserResponse rsp3;
                        WrappedCall::UpdateUser(rsp1.userinfo(), rsp3, this);
                        refresh();
                    }
                    return;
                }
                QMessageBox::information(this, "Error", "User Selected Not found!");
                return;
            }
        }
    });

    controlLayout->addWidget(btnCreate);
    controlLayout->addWidget(btnDelete);
    controlLayout->addWidget(btnChangePermissionGroup);

    setLayout(layout);

    refresh();
}

EditorAdminUserManager::~EditorAdminUserManager() {
    delete ui;
}

void EditorAdminUserManager::refresh() {
    m_TreeWidget->getIndexMap().clear();
    m_TreeWidget->getQTreeWidget()->clear();

    proto::GetAllUserResponse response;
    if (!WrappedCall::getAllUserMetaInfo(response, this)) {
        return;
    }

    proto::GetAllPermissionGroupResponse rsp1;
    if (!WrappedCall::GetAllPermissionGroup(rsp1, this)) {
        return;
    }

    for (auto&userInfo: response.userinfo()) {
        auto* item = m_TreeWidget->addTopItem(userInfo.base().uuid(), userInfo.name(), QIcon(Image::ImageUser), {}, 0);

        auto iter = std::find_if(rsp1.permissiongrouplist().begin(), rsp1.permissiongrouplist().end(),
                     [&](const proto::PermissionGroupMetaInfoV1&value) {
                         return value.base().uuid() == userInfo.permissiongroupuuid();
                     });
        if (iter != rsp1.permissiongrouplist().end()) {
            item->setText(1, QString::fromStdString(iter->name()));
        }else {
            item->setText(1, QString::fromStdString(userInfo.permissiongroupuuid()));
        }

        if(userInfo.createtime().seconds()<0) {
            item->setText(2, "N/A");
        }else {
            item->setText(2, QString::fromStdString(timestampToString(userInfo.createtime().seconds())));
        }

        item->setText(3, QString::fromStdString(userInfo.description()));
    }

    m_TreeWidget->getQTreeWidget()->resizeColumnToContents(0);
    m_TreeWidget->getQTreeWidget()->resizeColumnToContents(1);
    m_TreeWidget->getQTreeWidget()->resizeColumnToContents(2);
}
