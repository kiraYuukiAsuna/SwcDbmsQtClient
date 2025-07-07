#include "ViewImportSwcFromFile.h"

#include <QFileDialog>
#include <filesystem>

#include "MainWindow.h"
#include "Message/Request.pb.h"
#include "ProgressBar.h"
#include "src/FileIo/AnoIo.hpp"
#include "src/FileIo/ApoIo.hpp"
#include "src/FileIo/SwcIo.hpp"
#include "src/framework/service/WrappedCall.h"
#include "src/framework/util/unsortswc/swcutils.h"
#include "ui_ViewImportSwcFromFile.h"

ViewImportSwcFromFile::ViewImportSwcFromFile(
	MainWindow* mainWindow, const std::string& belongToProjectUuid)
	: QDialog(mainWindow), ui(new Ui::ViewImportSwcFromFile) {
	ui->setupUi(this);

	m_MainWindow = mainWindow;

	ui->SwcFileInfo->clear();
	ui->SwcFileInfo->setColumnCount(7);
	QStringList headerLabels;
	headerLabels << "Swc FilePath"
				 << "Ano FilePath"
				 << "Apo FilePath"
				 << "Type"
				 << "Detected Swc Node Number"
				 << "New Name"
				 << "Import Status";
	ui->SwcFileInfo->setHorizontalHeaderLabels(headerLabels);
	ui->SwcFileInfo->resizeColumnsToContents();

	proto::GetAllProjectResponse response;
	if (WrappedCall::GetAllProject(response, this)) {
		for (auto& project : response.projectinfo()) {
			ui->ProjectName->addItem(QString::fromStdString(project.name()));
			ui->ProjectName->setItemData(
				ui->ProjectName->count() - 1,
				QString::fromStdString(project.base().uuid()));
			if (belongToProjectUuid == project.base().uuid()) {
				ui->ProjectName->setCurrentIndex(ui->ProjectName->count() - 1);
			}
		}
	}

	connect(ui->SelectFilesBtn, &QPushButton::clicked, this, [this]() {
		QFileDialog fileDialog(this);
		fileDialog.setWindowTitle("Select Swc Files");
		fileDialog.setDirectory(
			QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
		fileDialog.setNameFilter(tr("File(*.swc *.eswc)"));
		fileDialog.setFileMode(QFileDialog::ExistingFiles);
		fileDialog.setViewMode(QFileDialog::Detail);

		m_SwcList.clear();
		m_ESwcList.clear();

		QStringList fileNames;
		if (fileDialog.exec()) {
			fileNames = fileDialog.selectedFiles();

			ProgressBar progressBar(this);
			progressBar.setText("Process swc Files...");
			auto task = [this, fileNames, &progressBar]() {
				int totalFileCount = fileNames.size();
				int currentFileCount = 0;
				for (int i = 0; i < fileNames.size(); i++) {
					std::filesystem::path filePath(fileNames[i].toStdString());
					if (filePath.extension() == ".swc") {
						std::string unsortedSwcPath;
						try {
							unsortedSwcPath = convertSwcToUnsorted(filePath);
						} catch (std::runtime_error& e) {
							QApplication::postEvent(
								this, new UpdateImportUiErrorEvent(
										  e.what() +
										  std::string(
											  " , skip this file! filename: ") +
										  filePath.filename().string()));
							continue;
						}

						Swc swc(unsortedSwcPath);
						swc.ReadFromFile();
						auto neuron = swc.getValue();

						std::string anoPath =
							(filePath.parent_path() / filePath.stem().string())
								.string();
						std::string apoPath =
							(filePath.parent_path() /
							 (filePath.stem().string() + ".apo"))
								.string();

						ExtraSwcImportAttribute attribute;

						if (std::filesystem::exists(anoPath)) {
							attribute.m_AnoPath = anoPath;
						}

						if (std::filesystem::exists(apoPath)) {
							attribute.m_ApoPath = apoPath;
						}

						if (!std::filesystem::exists(anoPath) &&
							!std::filesystem::exists(apoPath)) {
							anoPath = (filePath.parent_path() /
									   (filePath.filename().string() + ".ano"))
										  .string();
							if (std::filesystem::exists(anoPath)) {
								attribute.m_AnoPath = anoPath;
							}
						}

						m_SwcList.emplace_back(swc, attribute);

						QApplication::postEvent(
							this, new UpdateImportUiEvent(
									  i, filePath.string(), attribute.m_AnoPath,
									  attribute.m_ApoPath, "swc", neuron.size(),
									  "Unprocessed"));
					} else if (filePath.extension() == ".eswc") {
						std::string unsortedSwcPath;
						try {
							unsortedSwcPath = convertSwcToUnsorted(filePath);
						} catch (std::runtime_error& e) {
							QApplication::postEvent(
								this, new UpdateImportUiErrorEvent(
										  e.what() +
										  std::string(
											  " , skip this file! filename: ") +
										  filePath.filename().string()));
							continue;
						}

						ESwc eSwc(unsortedSwcPath);
						eSwc.ReadFromFile();
						auto neuron = eSwc.getValue();

						std::string anoPath =
							(filePath.parent_path() / filePath.stem().string())
								.string();
						std::string apoPath =
							(filePath.parent_path() /
							 (filePath.stem().string() + ".apo"))
								.string();

						ExtraSwcImportAttribute attribute;

						if (std::filesystem::exists(anoPath)) {
							attribute.m_AnoPath = anoPath;
						}

						if (std::filesystem::exists(apoPath)) {
							attribute.m_ApoPath = apoPath;
						}

						m_ESwcList.emplace_back(eSwc, attribute);

						QApplication::postEvent(
							this, new UpdateImportUiEvent(
									  i, filePath.string(), attribute.m_AnoPath,
									  attribute.m_ApoPath, "eswc",
									  neuron.size(), "Unprocessed"));
					}
					currentFileCount++;

					QMetaObject::invokeMethod(
						this,
						[&progressBar, currentFileCount, totalFileCount]() {
							progressBar.setValue(currentFileCount * 100 /
												 totalFileCount);
						});
				}
				QApplication::postEvent(this, new UpdateImportUiEndEvent());
				QMetaObject::invokeMethod(
					this, [&progressBar, currentFileCount, totalFileCount]() {
						progressBar.finish();
					});
			};

			m_IoThread = std::thread(task);
			m_IoThread.detach();

			progressBar.exec();
		}
	});

	connect(ui->SelectFolderBtn, &QPushButton::clicked, this, [this]() {
		QFileDialog fileDialog(this);
		fileDialog.setWindowTitle("Select Swc Folder");
		fileDialog.setDirectory(
			QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
		fileDialog.setFileMode(QFileDialog::Directory);
		fileDialog.setViewMode(QFileDialog::Detail);

		m_SwcList.clear();
		m_ESwcList.clear();

		QStringList folderNames;
		if (fileDialog.exec()) {
			folderNames = fileDialog.selectedFiles();

			ProgressBar progressBar(this);
			progressBar.setText("Process swc Files...");
			auto task = [this, folderNames, &progressBar]() {
				int totalFileCount = 0;
				for (auto& folder : folderNames) {
					std::filesystem::path folderPath(folder.toStdString());
					for (auto& dirEntry :
						 std::filesystem::recursive_directory_iterator(
							 folderPath)) {
						const std::filesystem::path& filePath(dirEntry.path());
						if (filePath.extension() == ".swc" ||
							filePath.extension() == ".eswc") {
							totalFileCount++;
						}
					}
				}

				int currentRow = 0;
				for (auto folder : folderNames) {
					std::filesystem::path folderPath(folder.toStdString());
					for (auto& dirEntry :
						 std::filesystem::recursive_directory_iterator(
							 folderPath)) {
						const std::filesystem::path& filePath(dirEntry.path());
						if (filePath.extension() == ".swc") {
							std::string unsortedSwcPath;
							try {
								unsortedSwcPath =
									convertSwcToUnsorted(filePath);
							} catch (std::runtime_error& e) {
								QApplication::postEvent(
									this,
									new UpdateImportUiErrorEvent(
										e.what() +
										std::string(
											" , skip this file! filename: ") +
										filePath.filename().string()));
								continue;
							}

							Swc swc(unsortedSwcPath);
							swc.ReadFromFile();
							auto neuron = swc.getValue();

							std::string anoPath = (filePath.parent_path() /
												   filePath.stem().string())
													  .string();
							std::string apoPath =
								(filePath.parent_path() /
								 (filePath.stem().string() + ".apo"))
									.string();

							ExtraSwcImportAttribute attribute;

							if (std::filesystem::exists(anoPath)) {
								attribute.m_AnoPath = anoPath;
							}

							if (std::filesystem::exists(apoPath)) {
								attribute.m_ApoPath = apoPath;
							}

							if (!std::filesystem::exists(anoPath) &&
								!std::filesystem::exists(apoPath)) {
								anoPath =
									(filePath.parent_path() /
									 (filePath.filename().string() + ".ano"))
										.string();
								if (std::filesystem::exists(anoPath)) {
									attribute.m_AnoPath = anoPath;
								}
							}

							m_SwcList.emplace_back(swc, attribute);

							QApplication::postEvent(
								this,
								new UpdateImportUiEvent(
									currentRow, filePath.string(),
									attribute.m_AnoPath, attribute.m_ApoPath,
									"swc", neuron.size(), "Unprocessed"));

							currentRow++;
						} else if (filePath.extension() == ".eswc") {
							std::string unsortedSwcPath;
							try {
								unsortedSwcPath =
									convertSwcToUnsorted(filePath);
							} catch (std::runtime_error& e) {
								QApplication::postEvent(
									this,
									new UpdateImportUiErrorEvent(
										e.what() +
										std::string(
											" , skip this file! filename: ") +
										filePath.filename().string()));
								continue;
							}

							ESwc eSwc(unsortedSwcPath);
							eSwc.ReadFromFile();
							auto neuron = eSwc.getValue();

							std::string anoPath = (filePath.parent_path() /
												   filePath.stem().string())
													  .string();
							std::string apoPath =
								(filePath.parent_path() /
								 (filePath.stem().string() + ".apo"))
									.string();

							ExtraSwcImportAttribute attribute;

							if (std::filesystem::exists(anoPath)) {
								attribute.m_AnoPath = anoPath;
							}

							if (std::filesystem::exists(apoPath)) {
								attribute.m_ApoPath = apoPath;
							}

							m_ESwcList.emplace_back(eSwc, attribute);

							QApplication::postEvent(
								this,
								new UpdateImportUiEvent(
									currentRow, filePath.string(),
									attribute.m_AnoPath, attribute.m_ApoPath,
									"eswc", neuron.size(), "Unprocessed"));

							currentRow++;
						}
						QMetaObject::invokeMethod(
							this, [&progressBar, currentRow, totalFileCount]() {
								progressBar.setValue(currentRow * 100 /
													 totalFileCount);
							});
					}
				}

				QApplication::postEvent(this, new UpdateImportUiEndEvent());
				QMetaObject::invokeMethod(
					this, [&progressBar, currentRow, totalFileCount]() {
						progressBar.finish();
					});

				int totalAnoFileCount = 0;
				for (auto& swc : m_SwcList) {
					if (!swc.second.m_AnoPath.empty()) {
						totalAnoFileCount++;
					}
				}
				for (auto& swc : m_ESwcList) {
					if (!swc.second.m_AnoPath.empty()) {
						totalAnoFileCount++;
					}
				}
				int totalApoFileCount = 0;
				for (auto& swc : m_SwcList) {
					if (!swc.second.m_ApoPath.empty()) {
						totalApoFileCount++;
					}
				}
				for (auto& swc : m_ESwcList) {
					if (!swc.second.m_ApoPath.empty()) {
						totalApoFileCount++;
					}
				}

				ui->SummaryLabel->setText(QString::fromStdString(
					"Total Swc File: " +
					std::to_string(m_SwcList.size() + m_ESwcList.size()) +
					", Total Ano File: " + std::to_string(totalAnoFileCount) +
					", Total Apo File: " + std::to_string(totalApoFileCount)));
			};

			m_IoThread = std::thread(task);
			m_IoThread.detach();

			progressBar.exec();
		}
	});

	connect(ui->ImportBtn, &QPushButton::clicked, this, [this]() {
		if (ui->SwcFileInfo->rowCount() !=
			m_SwcList.size() + m_ESwcList.size()) {
			QMessageBox::critical(
				this, "Error",
				"Data outdated! Please reopen this import window!");
			return;
		}

		if (m_ActionImportComplete) {
			QMessageBox::information(
				this, "Warning",
				"Import action has completed! Please reopen this import window "
				"if you want to import more swc data!");
			return;
		}

		ProgressBar progressBar(this);
		progressBar.setText("Import swc Files...");
		auto task = [this, &progressBar]() {
			int processedSwcNumber = 0;
			int processedESwcNumber = 0;
			for (int i = 0; i < ui->SwcFileInfo->rowCount(); i++) {
				auto swcName =
					ui->SwcFileInfo->item(i, 5)->text().toStdString();

				bool errorDetected = false;

				std::string belongToProjectUuid;
				if (ui->ProjectName->count() != 0 &&
					ui->ProjectName->currentIndex() != -1) {
					if (!ui->ProjectName->currentText().isEmpty()) {
						belongToProjectUuid =
							ui->ProjectName
								->itemData(ui->ProjectName->currentIndex())
								.toString()
								.toStdString();
					}
				}

				if (ui->SwcFileInfo->item(i, 3)->text() == "swc") {
					std::string description =
						"Auto Generated By SwcDbmsQtClient.";
					proto::CreateSwcResponse response;
					if (WrappedCall::createSwcMeta(swcName, description,
												   belongToProjectUuid,
												   response, this, true)) {
						proto::SwcDataV1 swcData;
						auto& newSwcRawData =
							m_SwcList[processedSwcNumber].first.getValue();
						for (auto& val : newSwcRawData) {
							auto* newData = swcData.add_swcdata();
							auto internalData =
								newData->mutable_swcnodeinternaldata();
							internalData->set_n(val.n);
							internalData->set_type(val.type);
							internalData->set_x(val.x);
							internalData->set_y(val.y);
							internalData->set_z(val.z);
							internalData->set_radius(val.radius);
							internalData->set_parent(val.parent);
							internalData->set_seg_id(val.seg_id);
							internalData->set_level(val.level);
							internalData->set_mode(val.mode);
							internalData->set_timestamp(val.timestamp);
							internalData->set_feature_value(val.feature_value);
						}

						proto::CreateSwcNodeDataResponse response1;
						if (WrappedCall::addSwcNodeDataByUuid(
								response.swcinfo().base().uuid(), swcData,
								response1, this, true)) {
							QMetaObject::invokeMethod(this, [this, i]() {
								ui->SwcFileInfo->item(i, 6)->setText(
									"Create Swc Node Success!");
								setAllGridColor(i, Qt::green);
							});
						} else {
							QMetaObject::invokeMethod(this, [this, i]() {
								ui->SwcFileInfo->item(i, 6)->setText(
									"Create Swc Node failed!");
								setAllGridColor(i, Qt::red);
							});
						}
					} else {
						QMetaObject::invokeMethod(this, [this, i]() {
							ui->SwcFileInfo->item(i, 6)->setText(
								"Create Swc Node Meta Info failed!");
							setAllGridColor(i, Qt::red);
						});
					}

					if (!m_SwcList[processedSwcNumber]
							 .second.m_ApoPath.empty()) {
						ApoIo io(
							m_SwcList[processedSwcNumber].second.m_ApoPath);

						try {
							io.ReadFromFile();
							auto& value = io.getValue();

							std::vector<proto::SwcAttachmentApoV1> modelData;
							std::for_each(value.begin(), value.end(),
										  [&](ApoUnit& val) {
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

							std::vector<proto::SwcAttachmentApoV1>
								m_SwcAttachmentApoData = modelData;

							proto::CreateSwcAttachmentApoResponse responseApo;
							if (!WrappedCall::createSwcAttachmentApo(
									response.swcinfo().base().uuid(),
									m_SwcAttachmentApoData, responseApo, this,
									true)) {
								if (!errorDetected) {
									QMetaObject::invokeMethod(
										this, [this, i, responseApo]() {
											ui->SwcFileInfo->item(i, 6)
												->setText(
													"Create Swc Apo attachment "
													"Failed! " +
													QString::fromStdString(
														responseApo.metainfo()
															.message()));
											setAllGridColor(i, Qt::red);
										});
								}
							}
						} catch (std::exception& e) {
							QMetaObject::invokeMethod(this, [this, i, e]() {
								ui->SwcFileInfo->item(i, 6)->setText(e.what());
								ui->SwcFileInfo->item(i, 1)->setBackground(
									QBrush(Qt::red));
							});
						}
					} else {
						proto::CreateSwcAttachmentApoResponse response1;
						std::vector<proto::SwcAttachmentApoV1>
							swcAttachmentApoData;
						if (WrappedCall::createSwcAttachmentApo(
								response.swcinfo().base().uuid(),
								swcAttachmentApoData, response1, this)) {
							QMetaObject::invokeMethod(this, [this, i,
															 response1]() {
								ui->SwcFileInfo->item(i, 6)->setText(
									"Create Swc Apo attachment successfully! ");
								setAllGridColor(i, Qt::green);
							});
						} else {
							QMetaObject::invokeMethod(
								this, [this, i, response1]() {
									ui->SwcFileInfo->item(i, 6)->setText(
										"Create Swc Apo attachment Failed! " +
										QString::fromStdString(
											response1.metainfo().message()));
									setAllGridColor(i, Qt::red);
								});
						}
					}

					if (!m_SwcList[processedSwcNumber]
							 .second.m_AnoPath.empty()) {
						AnoIo io(
							m_SwcList[processedSwcNumber].second.m_AnoPath);
						io.ReadFromFile();
						proto::CreateSwcAttachmentAnoResponse responseAno;
						if (!WrappedCall::createSwcAttachmentAno(
								response.swcinfo().base().uuid(),
								io.getValue().APOFILE, io.getValue().SWCFILE,
								responseAno, nullptr, true)) {
							if (!errorDetected) {
								QMetaObject::invokeMethod(
									this, [this, i, responseAno]() {
										ui->SwcFileInfo->item(i, 6)->setText(
											"Create Swc Ano attachment "
											"Failed! " +
											QString::fromStdString(
												responseAno.metainfo()
													.message()));
										setAllGridColor(i, Qt::red);
									});
							}
						}
					} else {
						proto::CreateSwcAttachmentAnoResponse response1;
						if (WrappedCall::createSwcAttachmentAno(
								response.swcinfo().base().uuid(), "", "",
								response1, nullptr, true)) {
							QMetaObject::invokeMethod(this, [this, i,
															 response1]() {
								ui->SwcFileInfo->item(i, 6)->setText(
									"Create Swc Ano attachment successfully!");
								setAllGridColor(i, Qt::green);
							});
						} else {
							QMetaObject::invokeMethod(
								this, [this, i, response1]() {
									ui->SwcFileInfo->item(i, 6)->setText(
										"Create Swc Ano attachment Failed! " +
										QString::fromStdString(
											response1.metainfo().message()));
									setAllGridColor(i, Qt::red);
								});
						}
					}

					processedSwcNumber++;
				} else if (ui->SwcFileInfo->item(i, 3)->text() == "eswc") {
					std::string description =
						"Auto Generated By SwcDbmsQtClient.";
					proto::CreateSwcResponse response;
					if (WrappedCall::createSwcMeta(swcName, description,
												   belongToProjectUuid,
												   response, this, true)) {
						proto::SwcDataV1 swcData;
						auto& newSwcRawData =
							m_ESwcList[processedESwcNumber].first.getValue();
						for (auto& j : newSwcRawData) {
							auto* newData = swcData.add_swcdata();
							auto internalData =
								newData->mutable_swcnodeinternaldata();
							internalData->set_n(j.n);
							internalData->set_type(j.type);
							internalData->set_x(j.x);
							internalData->set_y(j.y);
							internalData->set_z(j.z);
							internalData->set_radius(j.radius);
							internalData->set_parent(j.parent);
							internalData->set_seg_id(j.seg_id);
							internalData->set_level(j.level);
							internalData->set_mode(j.mode);
							internalData->set_timestamp(j.timestamp);
							internalData->set_feature_value(j.feature_value);
						}

						proto::CreateSwcNodeDataResponse response1;
						if (WrappedCall::addSwcNodeDataByUuid(
								response.swcinfo().base().uuid(), swcData,
								response1, this, true)) {
							QMetaObject::invokeMethod(this, [this, i]() {
								ui->SwcFileInfo->item(i, 6)->setText(
									"Create Swc Node Success!");
								setAllGridColor(i, Qt::green);
							});
						} else {
							QMetaObject::invokeMethod(this, [this, i]() {
								ui->SwcFileInfo->item(i, 6)->setText(
									"Create Swc Node failed!");
								setAllGridColor(i, Qt::red);
							});
						}
					} else {
						QMetaObject::invokeMethod(this, [this, i]() {
							ui->SwcFileInfo->item(i, 6)->setText(
								"Create Swc Node Meta Info failed!");
							setAllGridColor(i, Qt::red);
						});
					}

					if (!m_ESwcList[processedESwcNumber]
							 .second.m_ApoPath.empty()) {
						ApoIo io(
							m_ESwcList[processedESwcNumber].second.m_ApoPath);

						try {
							io.ReadFromFile();
							auto& value = io.getValue();

							std::vector<proto::SwcAttachmentApoV1> modelData;
							std::for_each(value.begin(), value.end(),
										  [&](ApoUnit& val) {
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

							std::vector<proto::SwcAttachmentApoV1>
								m_SwcAttachmentApoData = modelData;

							proto::CreateSwcAttachmentApoResponse responseApo;
							if (!WrappedCall::createSwcAttachmentApo(
									response.swcinfo().base().uuid(),
									m_SwcAttachmentApoData, responseApo, this,
									true)) {
								if (!errorDetected) {
									QMetaObject::invokeMethod(
										this, [this, i, responseApo]() {
											ui->SwcFileInfo->item(i, 6)
												->setText(
													"Create Swc Apo attachment "
													"Failed! " +
													QString::fromStdString(
														responseApo.metainfo()
															.message()));
											setAllGridColor(i, Qt::red);
										});
								}
							}
						} catch (std::exception& e) {
							QMetaObject::invokeMethod(this, [this, i, e]() {
								ui->SwcFileInfo->item(i, 6)->setText(e.what());
								ui->SwcFileInfo->item(i, 1)->setBackground(
									QBrush(Qt::red));
							});
						}
					} else {
						proto::CreateSwcAttachmentApoResponse response1;
						std::vector<proto::SwcAttachmentApoV1>
							swcAttachmentApoData;
						if (WrappedCall::createSwcAttachmentApo(
								response.swcinfo().base().uuid(),
								swcAttachmentApoData, response1, nullptr,
								true)) {
							QMetaObject::invokeMethod(this, [this, i,
															 response1]() {
								ui->SwcFileInfo->item(i, 6)->setText(
									"Create Swc Apo attachment successfully! ");
								setAllGridColor(i, Qt::green);
							});
						} else {
							QMetaObject::invokeMethod(
								this, [this, i, response1]() {
									ui->SwcFileInfo->item(i, 6)->setText(
										"Create Swc Apo attachment Failed! " +
										QString::fromStdString(
											response1.metainfo().message()));
									setAllGridColor(i, Qt::red);
								});
						}
					}

					if (!m_ESwcList[processedESwcNumber]
							 .second.m_AnoPath.empty()) {
						AnoIo io(
							m_ESwcList[processedESwcNumber].second.m_AnoPath);
						io.ReadFromFile();
						proto::CreateSwcAttachmentAnoResponse responseAno;
						if (!WrappedCall::createSwcAttachmentAno(
								response.swcinfo().base().uuid(),
								io.getValue().APOFILE, io.getValue().SWCFILE,
								responseAno, this, true)) {
							if (!errorDetected) {
								QMetaObject::invokeMethod(
									this, [this, i, responseAno]() {
										ui->SwcFileInfo->item(i, 6)->setText(
											"Create Swc Ano attachment "
											"Failed! " +
											QString::fromStdString(
												responseAno.metainfo()
													.message()));
										setAllGridColor(i, Qt::red);
									});
							}
						}
					} else {
						proto::CreateSwcAttachmentAnoResponse response1;
						if (WrappedCall::createSwcAttachmentAno(
								response.swcinfo().base().uuid(), "", "",
								response1, nullptr, true)) {
							QMetaObject::invokeMethod(this, [this, i,
															 response1]() {
								ui->SwcFileInfo->item(i, 6)->setText(
									"Create Swc Ano attachment successfully!");
								setAllGridColor(i, Qt::green);
							});
						} else {
							QMetaObject::invokeMethod(
								this, [this, i, response1]() {
									ui->SwcFileInfo->item(i, 6)->setText(
										"Create Swc Ano attachment Failed! " +
										QString::fromStdString(
											response1.metainfo().message()));
									setAllGridColor(i, Qt::red);
								});
						}
					}

					processedESwcNumber++;
				}
				QMetaObject::invokeMethod(
					this, [this, &progressBar, processedESwcNumber,
						   processedSwcNumber]() {
						progressBar.setValue(
							(processedESwcNumber + processedSwcNumber) * 100 /
							(m_ESwcList.size() + m_SwcList.size()));
					});
			}
			QMetaObject::invokeMethod(
				this, [&progressBar]() { progressBar.finish(); });
		};

		m_IoThread = std::thread(task);
		m_IoThread.detach();

		progressBar.exec();

		m_ActionImportComplete = true;
		QMessageBox::information(this, "Info",
								 "Import action has completed! Please check "
								 "the <Import Status> in the table below!");
	});

	connect(ui->CancelBtn, &QPushButton::clicked, this, [this]() { reject(); });
}

ViewImportSwcFromFile::~ViewImportSwcFromFile() { delete ui; }

bool ViewImportSwcFromFile::event(QEvent* e) {
	if (e->type() == UpdateImportUiEvent::TYPE) {
		ui->SwcFileInfo->setRowCount(ui->SwcFileInfo->rowCount() + 1);

		auto ev = dynamic_cast<UpdateImportUiEvent*>(e);

		ui->SwcFileInfo->setItem(
			ev->currentRow, 0,
			new QTableWidgetItem(QString::fromStdString(ev->swcFilePath)));
		ui->SwcFileInfo->setItem(
			ev->currentRow, 1,
			new QTableWidgetItem(QString::fromStdString(ev->anoFilePath)));
		ui->SwcFileInfo->setItem(
			ev->currentRow, 2,
			new QTableWidgetItem(QString::fromStdString(ev->apoFilePath)));
		ui->SwcFileInfo->setItem(
			ev->currentRow, 3,
			new QTableWidgetItem(QString::fromStdString(ev->swcType)));
		ui->SwcFileInfo->setItem(ev->currentRow, 4,
								 new QTableWidgetItem(QString::fromStdString(
									 std::to_string(ev->nodeSize))));
		auto swcName =
			std::filesystem::path(ev->swcFilePath).filename().string();
		swcName = replaceAll(swcName, ".swc", ".eswc");
		ui->SwcFileInfo->setItem(
			ev->currentRow, 5,
			new QTableWidgetItem(QString::fromStdString(swcName)));
		ui->SwcFileInfo->setItem(
			ev->currentRow, 6,
			new QTableWidgetItem(QString::fromStdString("Unprocessed")));
	} else if (e->type() == UpdateImportUiEndEvent::TYPE) {
	} else if (e->type() == UpdateImportUiErrorEvent::TYPE) {
		auto ev = dynamic_cast<UpdateImportUiErrorEvent*>(e);
		QMessageBox::critical(this, "Error",
							  QString::fromStdString(ev->errorMessage));
	}

	return QWidget::event(e);
}

void ViewImportSwcFromFile::setAllGridColor(int row, const QColor& color) {
	ui->SwcFileInfo->item(row, 0)->setBackground(QBrush(color));
	ui->SwcFileInfo->item(row, 1)->setBackground(QBrush(color));
	ui->SwcFileInfo->item(row, 2)->setBackground(QBrush(color));
	ui->SwcFileInfo->item(row, 3)->setBackground(QBrush(color));
	ui->SwcFileInfo->item(row, 4)->setBackground(QBrush(color));
	ui->SwcFileInfo->item(row, 5)->setBackground(QBrush(color));
	ui->SwcFileInfo->item(row, 6)->setBackground(QBrush(color));
}

std::string ViewImportSwcFromFile::convertSwcToUnsorted(
	const std::filesystem::path& filePath) {
	QString inputSwcPath = QString::fromStdString(filePath.string());
	QString outputSwcPath = QString::fromStdString(
		(getTempLocation() / filePath.filename()).string());

	convertSWC2UnSorted(inputSwcPath, outputSwcPath);

	return outputSwcPath.toStdString();
}
