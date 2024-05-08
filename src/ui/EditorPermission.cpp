#include "EditorPermission.h"

#include <qinputdialog.h>
#include <QPushButton>
#include <QVBoxLayout>
#include <Message/Request.pb.h>
#include <Message/Response.pb.h>

#include "ui_EditorPermission.h"
#include "src/framework/defination/ImageDefination.h"
#include "src/framework/service/WrappedCall.h"

EditorPermission::EditorPermission(const std::string&name, MetaInfoType type, QWidget* parent) : QDialog(parent),
    ui(new Ui::EditorPermission), m_Name(name), m_Type(type) {
    ui->setupUi(this);

    auto* layout = new QVBoxLayout;

    m_TreeWidget = new TreeWidget(this);
    QStringList headers;
    headers << tr("Permission");
    m_TreeWidget->getQTreeWidget()->setHeaderLabels(headers);
    layout->addWidget(m_TreeWidget);

    auto controlLayout = new QHBoxLayout;
    layout->addLayout(controlLayout);

    auto btnAddUser = new QPushButton("Add User", this);
    auto btnDeleteUser = new QPushButton("Delete User", this);
    auto btnAddGroup = new QPushButton("Add Group", this);
    auto btnDeleteGroup = new QPushButton("Delete Group", this);
    auto btnCommit = new QPushButton("Commit Changes", this);

    connect(btnAddUser, &QPushButton::clicked, [&]() {
        bool bOk;
        auto newGroupName = QInputDialog::getText(this, "Add User", "Please input user name"
                                                  , QLineEdit::Normal, "", &bOk);
        if (!bOk) {
            return;
        }

        if (newGroupName.isEmpty()) {
            QMessageBox::information(this, "Add User", "Group name can not be empty");
            return;
        }

        proto::GetUserByNameResponse rsp1;
        if(!WrappedCall::GetUserInfoByName(newGroupName.toStdString(),rsp1, this)) {
            return;
        }

        proto::GetSwcMetaInfoResponse rsp2;
        if(!WrappedCall::getSwcMetaInfoByName(name, rsp2, this)) {
            return;
        }

        proto::UserPermissionAclV1 acl;
        acl.set_useruuid(rsp1.userinfo().base().uuid());

        for (int i = 0; i < acl.ace().GetDescriptor()->field_count(); i++) {
           acl.ace().GetReflection()->SetBool(acl.mutable_ace(), acl.ace().GetDescriptor()->field(i), false);
        }

        acl.PrintDebugString();

        auto permission = rsp2.mutable_swcinfo()->mutable_permission();
        auto ele = permission->mutable_users()->Add();
        ele->CopyFrom(acl);


        proto::UpdateSwcResponse response;
        if(!WrappedCall::UpdateSwcMetaInfo(rsp2.swcinfo(),response,this)) {
            return;
        }

        refresh();
    });


    controlLayout->addWidget(btnAddUser);
    controlLayout->addWidget(btnDeleteUser);
    controlLayout->addWidget(btnAddGroup);
    controlLayout->addWidget(btnDeleteGroup);
    controlLayout->addWidget(btnCommit);

    setLayout(layout);

    refresh();
}

EditorPermission::~EditorPermission() {
    delete ui;
}

void EditorPermission::refresh() {
    m_TreeWidget->getIndexMap().clear();
    m_TreeWidget->getQTreeWidget()->clear();

    if (m_Type == MetaInfoType::eProject) {
        proto::GetProjectResponse response;
        WrappedCall::getProjectMetaInfoByName(m_Name, response, this);
        m_PermissionMetaInfo = response.projectinfo().permission();
    }
    else if (m_Type == MetaInfoType::eSwc) {
        proto::GetSwcMetaInfoResponse response;
        WrappedCall::getSwcMetaInfoByName(m_Name, response, this);
        m_PermissionMetaInfo = response.swcinfo().permission();
    }

    auto&owner = m_PermissionMetaInfo.owner();
    proto::GetUserByUuidResponse ownerResponse;
    WrappedCall::GetUserInfoByUuid(owner.useruuid(), ownerResponse, this);
    m_TreeWidget->addTopItem(owner.useruuid(), "Owner: " + ownerResponse.userinfo().name(), QIcon(Image::ImageUserPermission), {});
    auto ownerDescriptor = owner.ace().GetDescriptor();
    for (int permission = 0; permission < ownerDescriptor->field_count(); permission++) {
        auto name = ownerDescriptor->field(permission)->name();
        auto* item = m_TreeWidget->addItem(owner.useruuid(), owner.useruuid() + name, name,
                                           QIcon(Image::ImageUserPermission), {});

        if (owner.ace().GetReflection()->GetBool(owner.ace(), ownerDescriptor->field(permission))) {
            item->setCheckState(0, Qt::Checked);
        }
        else {
            item->setCheckState(0, Qt::Unchecked);
        }
    }


    auto&users = m_PermissionMetaInfo.users();
    m_TreeWidget->addTopItem("Users", "Users", QIcon(Image::ImageUserPermission), {});
    for (auto&value: users) {
        proto::GetUserByUuidResponse response;
        WrappedCall::GetUserInfoByUuid(owner.useruuid(), response, this);
        m_TreeWidget->addItem("Users", "Users" + value.useruuid(), response.userinfo().name(),
                              QIcon(Image::ImageUserPermission), {});

        auto descriptor = value.ace().GetDescriptor();
        for (int permission = 0; permission < descriptor->field_count(); permission++) {
            auto name = descriptor->field(permission)->name();
            auto* item = m_TreeWidget->addItem("Users" + value.useruuid(), "Users" + value.useruuid() + name, name,
                                               QIcon(Image::ImageUserPermission), {});
            if (value.ace().GetReflection()->GetBool(value.ace(), descriptor->field(permission))) {
                item->setCheckState(0, Qt::Checked);
            }
            else {
                item->setCheckState(0, Qt::Unchecked);
            }
        }
    }

    auto&groups = m_PermissionMetaInfo.groups();
    m_TreeWidget->addTopItem("Groups", "Groups", QIcon(Image::ImageUserPermission), {});
    for (auto&value: groups) {
        proto::GetPermissionGroupResponse response;
        WrappedCall::GetPermissionGroupByUuid(value.groupuuid(), response, this);
        m_TreeWidget->addItem("Groups", "Groups" + value.groupuuid(), response.permissiongroup().name(),
                              QIcon(Image::ImageUserPermission), {});

        auto descriptor = value.ace().GetDescriptor();
        for (int permission = 0; permission < descriptor->field_count(); permission++) {
            auto name = descriptor->field(permission)->name();
            auto* item = m_TreeWidget->addItem("Groups" + value.groupuuid(), "Groups" + value.groupuuid() + name, name,
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
