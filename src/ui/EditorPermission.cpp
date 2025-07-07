#include "EditorPermission.h"

#include <Message/Request.pb.h>
#include <Message/Response.pb.h>
#include <qinputdialog.h>

#include <QPushButton>
#include <QVBoxLayout>

#include "src/framework/defination/ImageDefination.h"
#include "src/framework/service/WrappedCall.h"
#include "ui_EditorPermission.h"

EditorPermission::EditorPermission(const std::string& uuid, MetaInfoType type,
								   bool noSaveToCloud, QWidget* parent)
	: QDialog(parent),
	  ui(new Ui::EditorPermission),
	  m_Uuid(uuid),
	  m_Type(type),
	  m_NoSaveToCloud(noSaveToCloud) {
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
		auto newUserName =
			QInputDialog::getText(this, "Add User", "Please input user name",
								  QLineEdit::Normal, "", &bOk);
		if (!bOk) {
			return;
		}

		if (newUserName.isEmpty()) {
			QMessageBox::information(this, "Add User",
									 "User name can not be empty");
			return;
		}

		proto::GetUserByNameResponse rsp1;
		if (!WrappedCall::GetUserInfoByName(newUserName.toStdString(), rsp1,
											this)) {
			return;
		}

		if (m_Type == MetaInfoType::eProject) {
			proto::UserPermissionAclV1 acl;
			acl.set_useruuid(rsp1.userinfo().base().uuid());
			for (int i = 0; i < acl.ace().GetDescriptor()->field_count(); i++) {
				acl.ace().GetReflection()->SetBool(
					acl.mutable_ace(), acl.ace().GetDescriptor()->field(i),
					false);
			}

			auto ele = m_PermissionMetaInfo.mutable_users()->Add();
			ele->CopyFrom(acl);

			if (!m_NoSaveToCloud) {
				proto::GetProjectResponse rsp2;
				if (!WrappedCall::getProjectMetaInfoByUuid(m_Uuid, rsp2,
														   this)) {
					return;
				}

				rsp2.mutable_projectinfo()->mutable_permission()->CopyFrom(
					m_PermissionMetaInfo);

				proto::UpdateProjectResponse response;
				if (!WrappedCall::UpdateProjectMetaInfo(rsp2.projectinfo(),
														response, this)) {
					return;
				}
			} else {
				localRefresh();
			}
		} else if (m_Type == MetaInfoType::eFreeSwc ||
				   m_Type == MetaInfoType::eProjectSwc) {
			proto::UserPermissionAclV1 acl;
			acl.set_useruuid(rsp1.userinfo().base().uuid());
			for (int i = 0; i < acl.ace().GetDescriptor()->field_count(); i++) {
				acl.ace().GetReflection()->SetBool(
					acl.mutable_ace(), acl.ace().GetDescriptor()->field(i),
					false);
			}

			auto ele = m_PermissionMetaInfo.mutable_users()->Add();
			ele->CopyFrom(acl);

			if (!m_NoSaveToCloud) {
				proto::GetSwcMetaInfoResponse rsp2;
				if (!WrappedCall::getSwcMetaInfoByUuid(m_Uuid, rsp2, this)) {
					return;
				}

				rsp2.mutable_swcinfo()->mutable_permission()->CopyFrom(
					m_PermissionMetaInfo);

				proto::UpdateSwcResponse response;
				if (!WrappedCall::UpdateSwcMetaInfo(rsp2.swcinfo(), response,
													this)) {
					return;
				}
			} else {
				localRefresh();
			}
		}

		if (!m_NoSaveToCloud) {
			refresh();
		}
	});

	connect(btnDeleteUser, &QPushButton::clicked, [&]() {
		for (auto index_map : m_TreeWidget->getIndexMap()) {
			if (index_map.second ==
				m_TreeWidget->getQTreeWidget()->currentItem()) {
				auto iter = std::find_if(
					m_PermissionMetaInfo.users().begin(),
					m_PermissionMetaInfo.users().end(),
					[&](const proto::UserPermissionAclV1& value) {
						return ("Users" + value.useruuid()) == index_map.first;
					});
				if (iter != m_PermissionMetaInfo.users().end()) {
					proto::GetUserByUuidResponse response;
					if (!WrappedCall::GetUserInfoByUuid(
							index_map.first.substr(5), response, this)) {
						return;
					}
					int result = QMessageBox::information(
						this, "Warning",
						"Are you sure to delete this user? User Name: " +
							QString::fromStdString(response.userinfo().name()),
						QMessageBox::Yes, QMessageBox::No);
					if (result == QMessageBox::Yes) {
						m_PermissionMetaInfo.mutable_users()->erase(iter);

						if (m_Type == MetaInfoType::eProject) {
							if (!m_NoSaveToCloud) {
								proto::GetProjectResponse rsp1;
								WrappedCall::getProjectMetaInfoByUuid(
									m_Uuid, rsp1, this);

								rsp1.mutable_projectinfo()
									->mutable_permission()
									->CopyFrom(m_PermissionMetaInfo);

								proto::UpdateProjectResponse rsp2;
								if (!WrappedCall::UpdateProjectMetaInfo(
										rsp1.projectinfo(), rsp2, this)) {
									return;
								}
							} else {
								localRefresh();
							}
						} else if (m_Type == MetaInfoType::eFreeSwc ||
								   m_Type == MetaInfoType::eProjectSwc) {
							if (!m_NoSaveToCloud) {
								proto::GetSwcMetaInfoResponse rsp1;
								WrappedCall::getSwcMetaInfoByUuid(m_Uuid, rsp1,
																  this);

								rsp1.mutable_swcinfo()
									->mutable_permission()
									->CopyFrom(m_PermissionMetaInfo);

								proto::UpdateSwcResponse rsp2;
								if (!WrappedCall::UpdateSwcMetaInfo(
										rsp1.swcinfo(), rsp2, this)) {
									return;
								}
							} else {
								localRefresh();
							}
						}

						if (!m_NoSaveToCloud) {
							refresh();
						}
					}
					return;
				}
				QMessageBox::information(this, "Error",
										 "User Selected Not found!");
				return;
			}
		}
	});

	connect(btnAddGroup, &QPushButton::clicked, [&]() {
		bool bOk;
		auto newGroupName =
			QInputDialog::getText(this, "Add Group", "Please input group name",
								  QLineEdit::Normal, "", &bOk);
		if (!bOk) {
			return;
		}

		if (newGroupName.isEmpty()) {
			QMessageBox::information(this, "Add Group",
									 "Group name can not be empty");
			return;
		}

		proto::GetPermissionGroupByNameResponse rsp1;
		if (!WrappedCall::GetPermissionGroupByName(newGroupName.toStdString(),
												   rsp1, this)) {
			return;
		}

		if (m_Type == MetaInfoType::eProject) {
			proto::GroupPermissionAclV1 acl;
			acl.set_groupuuid(rsp1.permissiongroup().base().uuid());
			for (int i = 0; i < acl.ace().GetDescriptor()->field_count(); i++) {
				acl.ace().GetReflection()->SetBool(
					acl.mutable_ace(), acl.ace().GetDescriptor()->field(i),
					false);
			}

			auto ele = m_PermissionMetaInfo.mutable_groups()->Add();
			ele->CopyFrom(acl);

			if (!m_NoSaveToCloud) {
				proto::GetProjectResponse rsp2;
				if (!WrappedCall::getProjectMetaInfoByUuid(m_Uuid, rsp2,
														   this)) {
					return;
				}

				rsp2.mutable_projectinfo()->mutable_permission()->CopyFrom(
					m_PermissionMetaInfo);

				proto::UpdateProjectResponse response;
				if (!WrappedCall::UpdateProjectMetaInfo(rsp2.projectinfo(),
														response, this)) {
					return;
				}
			} else {
				localRefresh();
			}
		} else if (m_Type == MetaInfoType::eFreeSwc ||
				   m_Type == MetaInfoType::eProjectSwc) {
			proto::GroupPermissionAclV1 acl;
			acl.set_groupuuid(rsp1.permissiongroup().base().uuid());
			for (int i = 0; i < acl.ace().GetDescriptor()->field_count(); i++) {
				acl.ace().GetReflection()->SetBool(
					acl.mutable_ace(), acl.ace().GetDescriptor()->field(i),
					false);
			}

			auto ele = m_PermissionMetaInfo.mutable_groups()->Add();
			ele->CopyFrom(acl);

			if (!m_NoSaveToCloud) {
				proto::GetSwcMetaInfoResponse rsp2;
				if (!WrappedCall::getSwcMetaInfoByUuid(m_Uuid, rsp2, this)) {
					return;
				}

				rsp2.mutable_swcinfo()->mutable_permission()->CopyFrom(
					m_PermissionMetaInfo);

				proto::UpdateSwcResponse response;
				if (!WrappedCall::UpdateSwcMetaInfo(rsp2.swcinfo(), response,
													this)) {
					return;
				}
			} else {
				localRefresh();
			}
		}

		if (!m_NoSaveToCloud) {
			refresh();
		}
	});

	connect(btnDeleteGroup, &QPushButton::clicked, [&]() {
		for (auto index_map : m_TreeWidget->getIndexMap()) {
			if (index_map.second ==
				m_TreeWidget->getQTreeWidget()->currentItem()) {
				auto iter =
					std::find_if(m_PermissionMetaInfo.groups().begin(),
								 m_PermissionMetaInfo.groups().end(),
								 [&](const proto::GroupPermissionAclV1& value) {
									 return ("Groups" + value.groupuuid()) ==
											index_map.first;
								 });
				if (iter != m_PermissionMetaInfo.groups().end()) {
					proto::GetPermissionGroupByUuidResponse rsp;
					if (!WrappedCall::GetPermissionGroupByUuid(
							index_map.first.substr(6), rsp, this)) {
						return;
					}
					int result = QMessageBox::information(
						this, "Warning",
						"Are you sure to delete this group? Group Name: " +
							QString::fromStdString(
								rsp.permissiongroup().name()),
						QMessageBox::Yes, QMessageBox::No);
					if (result == QMessageBox::Yes) {
						m_PermissionMetaInfo.mutable_groups()->erase(iter);

						if (m_Type == MetaInfoType::eProject) {
							if (!m_NoSaveToCloud) {
								proto::GetProjectResponse rsp1;
								WrappedCall::getProjectMetaInfoByUuid(
									m_Uuid, rsp1, this);

								rsp1.mutable_projectinfo()
									->mutable_permission()
									->CopyFrom(m_PermissionMetaInfo);

								proto::UpdateProjectResponse rsp2;
								if (!WrappedCall::UpdateProjectMetaInfo(
										rsp1.projectinfo(), rsp2, this)) {
									return;
								}
							} else {
								localRefresh();
							}
						} else if (m_Type == MetaInfoType::eFreeSwc ||
								   m_Type == MetaInfoType::eProjectSwc) {
							if (!m_NoSaveToCloud) {
								proto::GetSwcMetaInfoResponse rsp1;
								WrappedCall::getSwcMetaInfoByUuid(m_Uuid, rsp1,
																  this);

								rsp1.mutable_swcinfo()
									->mutable_permission()
									->CopyFrom(m_PermissionMetaInfo);

								proto::UpdateSwcResponse rsp2;
								if (!WrappedCall::UpdateSwcMetaInfo(
										rsp1.swcinfo(), rsp2, this)) {
									return;
								}
							} else {
								localRefresh();
							}
						}

						if (!m_NoSaveToCloud) {
							refresh();
						}
					}
					return;
				}
				QMessageBox::information(this, "Error",
										 "Group Selected Not found!");
				return;
			}
		}
	});

	connect(btnCommit, &QPushButton::clicked, [&]() {
		int result = QMessageBox::information(
			this, "Warning",
			"Are you sure to commit your changes to permissions?",
			QMessageBox::Yes, QMessageBox::No);
		if (result != QMessageBox::Yes) {
			return;
		}

		auto* item =
			m_TreeWidget->findItem(m_PermissionMetaInfo.owner().useruuid());
		if (item) {
			auto descriptor =
				m_PermissionMetaInfo.owner().ace().GetDescriptor();
			for (int permission = 0; permission < descriptor->field_count();
				 permission++) {
				auto name = descriptor->field(permission)->name();

				auto permissionItem = m_TreeWidget->findItem(
					m_PermissionMetaInfo.owner().useruuid() + name);
				if (!permissionItem) {
					continue;
				}

				bool isChecked = permissionItem->checkState(0) == Qt::Checked;

				m_PermissionMetaInfo.owner().ace().GetReflection()->SetBool(
					m_PermissionMetaInfo.mutable_owner()->mutable_ace(),
					descriptor->field(permission), isChecked);
			}
		}

		for (auto& value : *m_PermissionMetaInfo.mutable_users()) {
			auto* item = m_TreeWidget->findItem("Users" + value.useruuid());
			if (item) {
				auto descriptor = value.ace().GetDescriptor();
				for (int permission = 0; permission < descriptor->field_count();
					 permission++) {
					auto name = descriptor->field(permission)->name();

					auto permissionItem = m_TreeWidget->findItem(
						"Users" + value.useruuid() + name);
					if (!permissionItem) {
						continue;
					}

					bool isChecked =
						permissionItem->checkState(0) == Qt::Checked;

					value.ace().GetReflection()->SetBool(
						value.mutable_ace(), descriptor->field(permission),
						isChecked);
				}
			}
		}

		for (auto& value : *m_PermissionMetaInfo.mutable_groups()) {
			auto* item = m_TreeWidget->findItem("Groups" + value.groupuuid());
			if (item) {
				auto descriptor = value.ace().GetDescriptor();
				for (int permission = 0; permission < descriptor->field_count();
					 permission++) {
					auto name = descriptor->field(permission)->name();

					auto permissionItem = m_TreeWidget->findItem(
						"Groups" + value.groupuuid() + name);
					if (!permissionItem) {
						continue;
					}

					bool isChecked =
						permissionItem->checkState(0) == Qt::Checked;

					value.ace().GetReflection()->SetBool(
						value.mutable_ace(), descriptor->field(permission),
						isChecked);
				}
			}
		}

		if (m_Type == MetaInfoType::eProject) {
			if (!m_NoSaveToCloud) {
				proto::GetProjectResponse rsp1;
				WrappedCall::getProjectMetaInfoByUuid(m_Uuid, rsp1, this);

				rsp1.mutable_projectinfo()->mutable_permission()->CopyFrom(
					m_PermissionMetaInfo);

				proto::UpdateProjectResponse rsp2;
				if (!WrappedCall::UpdateProjectMetaInfo(rsp1.projectinfo(),
														rsp2, this)) {
					return;
				}
				refresh();
			} else {
				localRefresh();
			}
		} else if (m_Type == MetaInfoType::eFreeSwc ||
				   m_Type == MetaInfoType::eProjectSwc) {
			if (!m_NoSaveToCloud) {
				proto::GetSwcMetaInfoResponse rsp1;
				WrappedCall::getSwcMetaInfoByUuid(m_Uuid, rsp1, this);

				rsp1.mutable_swcinfo()->mutable_permission()->CopyFrom(
					m_PermissionMetaInfo);

				proto::UpdateSwcResponse rsp2;
				if (!WrappedCall::UpdateSwcMetaInfo(rsp1.swcinfo(), rsp2,
													this)) {
					return;
				}
				refresh();
			} else {
				localRefresh();
			}
		}
	});

	controlLayout->addWidget(btnAddUser);
	controlLayout->addWidget(btnDeleteUser);
	controlLayout->addWidget(btnAddGroup);
	controlLayout->addWidget(btnDeleteGroup);
	controlLayout->addWidget(btnCommit);

	setLayout(layout);

	if (!m_NoSaveToCloud) {
		refresh();
	} else {
		localRefresh();
	}
}

EditorPermission::~EditorPermission() { delete ui; }

void EditorPermission::refresh() {
	if (m_Type == MetaInfoType::eProject) {
		proto::GetProjectResponse response;
		WrappedCall::getProjectMetaInfoByUuid(m_Uuid, response, this);
		m_PermissionMetaInfo = response.projectinfo().permission();
	} else if (m_Type == MetaInfoType::eFreeSwc ||
			   m_Type == MetaInfoType::eProjectSwc) {
		proto::GetSwcMetaInfoResponse response;
		WrappedCall::getSwcMetaInfoByUuid(m_Uuid, response, this);
		m_PermissionMetaInfo = response.swcinfo().permission();
	}

	localRefresh();
}

void EditorPermission::localRefresh() {
	m_TreeWidget->getIndexMap().clear();
	m_TreeWidget->getQTreeWidget()->clear();

	auto owner = m_PermissionMetaInfo.mutable_owner();
	if (owner->useruuid().empty() && m_NoSaveToCloud) {
		owner->set_useruuid(CachedProtoData::getInstance().UserUuid);
		auto descriptor = owner->ace().GetDescriptor();
		for (int permission = 0; permission < descriptor->field_count();
			 permission++) {
			auto name = descriptor->field(permission)->name();
			owner->ace().GetReflection()->SetBool(
				owner->mutable_ace(), descriptor->field(permission), true);
		}
	}
	proto::GetUserByUuidResponse ownerResponse;
	WrappedCall::GetUserInfoByUuid(owner->useruuid(), ownerResponse, this);
	m_TreeWidget->addTopItem(owner->useruuid(),
							 "Owner: " + ownerResponse.userinfo().name(),
							 QIcon(Image::ImageUser), {});
	auto ownerDescriptor = owner->ace().GetDescriptor();
	for (int permission = 0; permission < ownerDescriptor->field_count();
		 permission++) {
		auto name = ownerDescriptor->field(permission)->name();
		auto* item =
			m_TreeWidget->addItem(owner->useruuid(), owner->useruuid() + name,
								  name, QIcon(Image::ImageACE), {});
		item->setDisabled(true);
		if (owner->ace().GetReflection()->GetBool(
				owner->ace(), ownerDescriptor->field(permission))) {
			item->setCheckState(0, Qt::Checked);
		} else {
			item->setCheckState(0, Qt::Unchecked);
		}
	}

	auto& users = m_PermissionMetaInfo.users();
	m_TreeWidget->addTopItem("Users", "Users", QIcon(Image::ImageUser), {});
	for (auto& value : users) {
		proto::GetUserByUuidResponse response;
		WrappedCall::GetUserInfoByUuid(value.useruuid(), response, this);
		m_TreeWidget->addItem("Users", "Users" + value.useruuid(),
							  response.userinfo().name(),
							  QIcon(Image::ImageUser), {});

		auto descriptor = value.ace().GetDescriptor();
		for (int permission = 0; permission < descriptor->field_count();
			 permission++) {
			auto name = descriptor->field(permission)->name();
			auto* item = m_TreeWidget->addItem(
				"Users" + value.useruuid(), "Users" + value.useruuid() + name,
				name, QIcon(Image::ImageACE), {});
			if (value.ace().GetReflection()->GetBool(
					value.ace(), descriptor->field(permission))) {
				item->setCheckState(0, Qt::Checked);
			} else {
				item->setCheckState(0, Qt::Unchecked);
			}
		}
	}

	auto& groups = m_PermissionMetaInfo.groups();
	m_TreeWidget->addTopItem("Groups", "Groups",
							 QIcon(Image::ImageUserPermission), {});
	for (auto& value : groups) {
		proto::GetPermissionGroupByUuidResponse response;
		WrappedCall::GetPermissionGroupByUuid(value.groupuuid(), response,
											  this);
		m_TreeWidget->addItem("Groups", "Groups" + value.groupuuid(),
							  response.permissiongroup().name(),
							  QIcon(Image::ImageUserPermission), {});

		auto descriptor = value.ace().GetDescriptor();
		for (int permission = 0; permission < descriptor->field_count();
			 permission++) {
			auto name = descriptor->field(permission)->name();
			auto* item =
				m_TreeWidget->addItem("Groups" + value.groupuuid(),
									  "Groups" + value.groupuuid() + name, name,
									  QIcon(Image::ImageACE), {});
			if (value.ace().GetReflection()->GetBool(
					value.ace(), descriptor->field(permission))) {
				item->setCheckState(0, Qt::Checked);
			} else {
				item->setCheckState(0, Qt::Unchecked);
			}
		}
	}
}
