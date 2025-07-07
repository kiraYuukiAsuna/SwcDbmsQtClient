#include "EditorApoAttachment.h"

#include <QFileDialog>
#include <QStandardPaths>
#include <filesystem>

#include "ViewSwcApoData.h"
#include "src/FileIo/ApoIo.hpp"
#include "src/framework/service/WrappedCall.h"
#include "ui_EditorApoAttachment.h"

EditorApoAttachment::EditorApoAttachment(const std::string &swcUuid,
										 QWidget *parent)
	: QDialog(parent), ui(new Ui::EditorApoAttachment), m_SwcUuid(swcUuid) {
	ui->setupUi(this);

	auto font = QApplication::font();
	font.setBold(true);
	font.setPixelSize(22);
	ui->tableView->setFont(font);

	connect(ui->OKBtn, &QPushButton::clicked, this, [&]() {
		if (m_IsApoAttachmentExist) {
			proto::UpdateSwcAttachmentApoResponse response;
			if (WrappedCall::updateSwcAttachmentApoByUuid(
					m_SwcUuid, m_AttachmentUuid, m_SwcAttachmentApoData,
					response, this)) {
				QMessageBox::information(
					this, "Info", "Update Swc Apo attachment successfully!");
				accept();
			} else {
				QMessageBox::critical(
					this, "Error",
					"Update Swc Apo attachment Failed! " +
						QString::fromStdString(response.metainfo().message()));
			}
		} else {
			proto::CreateSwcAttachmentApoResponse response;
			if (WrappedCall::createSwcAttachmentApo(
					m_SwcUuid, m_SwcAttachmentApoData, response, this)) {
				QMessageBox::information(
					this, "Info", "Create Swc Apo attachment successfully!");
				accept();
			} else {
				QMessageBox::critical(
					this, "Error",
					"Create Swc Apo attachment Failed! " +
						QString::fromStdString(response.metainfo().message()));
			}
		}
	});

	connect(ui->CancelBtn, &QPushButton::clicked, this, [&]() { reject(); });

	connect(ui->DeleteBtn, &QPushButton::clicked, this, [&]() {
		if (m_SwcAttachmentApoData.empty()) {
			return;
		}

		QModelIndex currentIndex =
			ui->tableView->selectionModel()->currentIndex();
		int currentRow = currentIndex.row();
		if (currentRow < 0) {
			QMessageBox::information(this, "Info",
									 "You need to select one row first!");
			return;
		}

		m_SwcAttachmentApoData.erase(m_SwcAttachmentApoData.begin() +
									 currentRow);
		loadSwcAttachmentApoData();
		QMessageBox::information(this, "Info", "Delete Successfully!");
	});

	connect(ui->AddBtn, &QPushButton::clicked, this, [&]() {
		ViewSwcApoData editor(this);
		if (editor.exec() == QDialog::Accepted) {
			auto swcNodeInternalData = editor.getSwcApoData();
			m_SwcAttachmentApoData.push_back(swcNodeInternalData);
			loadSwcAttachmentApoData();
			QMessageBox::information(this, "Info", "Add Successfully!");
		}
	});

	connect(ui->ModifyBtn, &QPushButton::clicked, this, [&]() {
		ViewSwcApoData editor(true, this);

		QModelIndex currentIndex =
			ui->tableView->selectionModel()->currentIndex();
		int currentRow = currentIndex.row();
		if (currentRow < 0) {
			QMessageBox::information(this, "Info",
									 "You need to select one row first!");
			return;
		}

		auto InitSwcApoData = m_SwcAttachmentApoData.at(currentRow);
		editor.setSwcApoData(InitSwcApoData);
		if (editor.exec() == QDialog::Accepted) {
			auto swcNodeInternalData = editor.getSwcApoData();
			m_SwcAttachmentApoData[currentRow] = swcNodeInternalData;
			loadSwcAttachmentApoData();
			QMessageBox::information(this, "Info", "Modify Successfully!");
		}
	});

	connect(ui->DeleteBtnFromDbBtn, &QPushButton::clicked, this, [&]() {
		proto::DeleteSwcAttachmentApoResponse response;
		if (WrappedCall::deleteSwcAttachmentApoByUuid(
				m_SwcUuid, m_AttachmentUuid, response, this)) {
			QMessageBox::information(this, "Info",
									 "Delete Swc Apo attachment successfully!");
			accept();
		} else {
			QMessageBox::critical(
				this, "Error",
				"Delete Swc Apo attachment Failed! " +
					QString::fromStdString(response.metainfo().message()));
		}
	});

	connect(ui->ImportBtn, &QPushButton::clicked, this, [&]() {
		QFileDialog dialog;
		dialog.setDirectory(
			QStandardPaths::displayName(QStandardPaths::HomeLocation));
		dialog.setFilter(QDir::Files);
		dialog.setNameFilter("*.apo");

		if (dialog.exec()) {
			std::string filePath;
			if (int num = dialog.selectedFiles().count(); num != 0) {
				filePath = dialog.selectedFiles()[0].toStdString();
			}

			std::filesystem::path path(filePath);
			if (std::filesystem::exists(path)) {
				ApoIo io(filePath);

				try {
					io.ReadFromFile();
					auto &value = io.getValue();

					std::vector<proto::SwcAttachmentApoV1> modelData;
					std::for_each(value.begin(), value.end(),
								  [&](ApoUnit &val) {
									  proto::SwcAttachmentApoV1 data;
									  data.set_n(val.n);
									  data.set_orderinfo(val.orderinfo);
									  data.set_name(val.name);
									  data.set_comment(val.comment);
									  data.set_z(val.z);
									  data.set_x(val.x);
									  data.set_y(val.y);
									  data.set_pixmax(val.pixmax);
									  data.set_intensity(val.intensity);
									  data.set_sdev(val.sdev);
									  data.set_volsize(val.volsize);
									  data.set_mass(val.mass);
									  data.set_colorr(val.color_r);
									  data.set_colorg(val.color_g);
									  data.set_colorb(val.color_b);
									  modelData.push_back(data);
								  });

					m_SwcAttachmentApoData = modelData;
					loadSwcAttachmentApoData();

					QMessageBox::information(this, "Info",
											 "Import Successfully!");
				} catch (std::exception &e) {
					QMessageBox::critical(this, "Error", e.what());
				}
			} else {
				QMessageBox::critical(this, "Error",
									  "Selected Apo File Not Exists!");
			}
		}
	});

	connect(ui->ExportBtn, &QPushButton::clicked, this, [&]() {
		if (m_SwcAttachmentApoData.empty()) {
			QMessageBox::information(this, "Info", "No data to export!");
		}

		QString dirPath = QFileDialog::getExistingDirectory(
			this, tr("Select Save Directory"),
			QStandardPaths::displayName(QStandardPaths::HomeLocation),
			QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
		if (dirPath.isEmpty()) {
			return;
		}

		std::filesystem::path path(dirPath.toStdString());
		if (std::filesystem::exists(path)) {
			proto::GetSwcMetaInfoResponse response;
			WrappedCall::getSwcMetaInfoByUuid(m_SwcUuid, response, this);
			auto filePath = path / (response.swcinfo().name() + ".Apo.apo");
			ApoIo io(filePath.string());
			std::vector<ApoUnit> units;
			std::for_each(m_SwcAttachmentApoData.begin(),
						  m_SwcAttachmentApoData.end(),
						  [&](proto::SwcAttachmentApoV1 &val) {
							  ApoUnit unit;
							  unit.n = val.n();
							  unit.orderinfo = val.orderinfo();
							  unit.name = val.name();
							  unit.comment = val.comment();
							  unit.z = val.z();
							  unit.x = val.x();
							  unit.y = val.y();
							  unit.pixmax = val.pixmax();
							  unit.intensity = val.intensity();
							  unit.sdev = val.sdev();
							  unit.volsize = val.volsize();
							  unit.mass = val.mass();
							  unit.color_r = val.colorr();
							  unit.color_g = val.colorg();
							  unit.color_b = val.colorb();
							  units.push_back(unit);
						  });

			io.setValue(units);
			io.WriteToFile();

			QMessageBox::information(this, "Info", "Export Successfully!");
		} else {
			QMessageBox::critical(this, "Error",
								  "Selected Apo File Save Path Not Exists!");
		}
	});

	getSwcApoAttachment();
	loadSwcAttachmentApoData();
}

EditorApoAttachment::~EditorApoAttachment() { delete ui; }

void EditorApoAttachment::getSwcApoAttachment() {
	proto::GetSwcMetaInfoResponse get_swc_meta_info_response;
	if (!WrappedCall::getSwcMetaInfoByUuid(m_SwcUuid,
										   get_swc_meta_info_response, this)) {
		return;
	}

	if (!get_swc_meta_info_response.swcinfo()
			 .swcattachmentapometainfo()
			 .attachmentuuid()
			 .empty()) {
		m_IsApoAttachmentExist = true;
	} else {
		QMessageBox::critical(
			this, "Error",
			"No Apo Attachment found! You can create a new apo attchment!");
		return;
	}

	proto::GetSwcAttachmentApoResponse response;
	m_AttachmentUuid = get_swc_meta_info_response.swcinfo()
						   .swcattachmentapometainfo()
						   .attachmentuuid();
	if (!WrappedCall::getSwcAttachmentApoByUuid(m_SwcUuid, m_AttachmentUuid,
												response, this)) {
		return;
	}

	for (auto &data : response.swcattachmentapo()) {
		m_SwcAttachmentApoData.push_back(data);
	}
}

void EditorApoAttachment::loadSwcAttachmentApoData() {
	auto *model = new SwcAttachmentApoTableModel(m_SwcAttachmentApoData, this);
	ui->tableView->setModel(model);
	ui->tableView->resizeColumnsToContents();
}
