#include "EditorQualityControl.h"

#include <QDir>
#include <QFileInfo>
#include <QHeaderView>
#include <QMessageBox>
#include <QTextStream>
#include <QVBoxLayout>

#include "SWCDetection/SWCDetection.h"
#include "neuron_editing/neuron_format_converter.h"
#include "src/framework/service/WrappedCall.h"

EditorQualityControl::EditorQualityControl(const std::string& swcUuid,
										   const std::string& swcName,
										   QWidget* parent)
	: QWidget(parent), m_SwcUuid(swcUuid), m_Name(swcName) {
	setupUi();
}

EditorQualityControl::EditorQualityControl(const QString& filePath,
										   QWidget* parent)
	: QWidget(parent), m_FilePath(filePath) {
	QFileInfo fi(filePath);
	m_Name = fi.fileName().toStdString();
	setupUi();
}

EditorQualityControl::~EditorQualityControl() = default;

std::string EditorQualityControl::getUuid() {
	if (!m_SwcUuid.empty()) {
		return m_SwcUuid;
	}
	return m_FilePath.toStdString();
}

MetaInfoType EditorQualityControl::getMetaInfoType() {
	return MetaInfoType::eQualityControl;
}

bool EditorQualityControl::save() { return true; }

void EditorQualityControl::setupUi() {
	auto* mainLayout = new QHBoxLayout(this);

	// Left panel: detection options
	auto* leftPanel = new QVBoxLayout;

	auto* titleLabel = new QLabel(
		QString::fromStdString(m_Name) + " - Quality Control");
	auto titleFont = titleLabel->font();
	titleFont.setBold(true);
	titleFont.setPointSize(titleFont.pointSize() + 2);
	titleLabel->setFont(titleFont);
	leftPanel->addWidget(titleLabel);

	m_ChkLoop = new QCheckBox("Loop Detection", this);
	m_ChkTip = new QCheckBox("Tip Detection", this);
	m_ChkBranching = new QCheckBox("Branching Detection", this);
	m_ChkCrossing = new QCheckBox("Crossing Detection", this);
	m_ChkSpecStructs = new QCheckBox("Special Structures Detection", this);

	m_ChkLoop->setChecked(true);
	m_ChkTip->setChecked(true);
	m_ChkBranching->setChecked(true);
	m_ChkCrossing->setChecked(true);
	m_ChkSpecStructs->setChecked(true);

	leftPanel->addWidget(m_ChkLoop);
	leftPanel->addWidget(m_ChkTip);
	leftPanel->addWidget(m_ChkBranching);
	leftPanel->addWidget(m_ChkCrossing);
	leftPanel->addWidget(m_ChkSpecStructs);
	leftPanel->addStretch();

	m_RunButton = new QPushButton("Run Analysis");
	connect(m_RunButton, &QPushButton::clicked, this,
			&EditorQualityControl::runAnalysis);
	leftPanel->addWidget(m_RunButton);

	m_VisualizeButton = new QPushButton("Visualize");
	m_VisualizeButton->setEnabled(false);
	connect(m_VisualizeButton, &QPushButton::clicked, this,
			&EditorQualityControl::onVisualize);
	leftPanel->addWidget(m_VisualizeButton);

	m_StatusLabel = new QLabel("Ready");
	leftPanel->addWidget(m_StatusLabel);

	auto* leftWidget = new QWidget;
	leftWidget->setLayout(leftPanel);
	leftWidget->setMinimumWidth(220);
	leftWidget->setMaximumWidth(280);
	mainLayout->addWidget(leftWidget);

	// Right panel: results table
	m_ResultTable = new QTableWidget(this);
	m_ResultTable->setColumnCount(5);
	m_ResultTable->setHorizontalHeaderLabels(
		{"Type", "X", "Y", "Z", "Description"});
	m_ResultTable->horizontalHeader()->setStretchLastSection(true);
	m_ResultTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_ResultTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	mainLayout->addWidget(m_ResultTable, 1);

	setLayout(mainLayout);
}

bool EditorQualityControl::writeSwcToTempFile(const QString& tempPath) {
	proto::GetSwcFullNodeDataResponse response;
	if (!WrappedCall::getSwcFullNodeDataByUuid(m_SwcUuid, response, this)) {
		return false;
	}

	QFile file(tempPath);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		return false;
	}
	QTextStream out(&file);

	const auto& swcData = response.swcnodedata();
	for (int i = 0; i < swcData.swcdata_size(); i++) {
		const auto& node = swcData.swcdata(i).swcnodeinternaldata();
		out << node.n() << " " << node.type() << " " << node.x() << " "
			<< node.y() << " " << node.z() << " " << node.radius()
			<< " " << node.parent() << "\n";
	}
	file.close();
	return true;
}

void EditorQualityControl::runAnalysis() {
	m_StatusLabel->setText("Running analysis...");
	m_RunButton->setEnabled(false);

	// 1. Load SWC as NeuronTree
	QString swcPath;
	QString tempFilePath;

	if (!m_SwcUuid.empty()) {
		tempFilePath = QDir::tempPath() + "/swcqc_" +
					   QString::fromStdString(m_SwcUuid).left(8) + ".swc";
		if (!writeSwcToTempFile(tempFilePath)) {
			m_StatusLabel->setText("Failed to fetch SWC data from server.");
			m_RunButton->setEnabled(true);
			return;
		}
		swcPath = tempFilePath;
	} else {
		swcPath = m_FilePath;
	}

	NeuronTree nt = readSWC_file(swcPath);

	if (!tempFilePath.isEmpty()) {
		QFile::remove(tempFilePath);
	}

	if (nt.listNeuron.isEmpty()) {
		QMessageBox::critical(this, "Error",
							  "Failed to parse SWC file or file is empty.");
		m_StatusLabel->setText("Error: invalid SWC file.");
		m_RunButton->setEnabled(true);
		return;
	}

	// 2. Convert to V_NeuronSWC_list using unsortswc converter
	V_NeuronSWC_list segList = NeuronTree__2__V_NeuronSWC_list(nt);

	if (segList.seg.empty()) {
		QMessageBox::critical(this, "Error",
							  "Failed to build segment list from SWC data.");
		m_StatusLabel->setText("Error: no segments.");
		m_RunButton->setEnabled(true);
		return;
	}

	// 3. Run detections and build markers
	struct ResultRow {
		QString type;
		float x, y, z;
		QString description;
	};
	std::vector<ResultRow> rows;
	m_Markers.clear();

	auto addMarker = [this](float x, float y, float z,
							float r, float g, float b) {
		SwcMarker marker;
		marker.x = x; marker.y = y; marker.z = z;
		marker.color[0] = r; marker.color[1] = g;
		marker.color[2] = b; marker.color[3] = 1.0f;
		m_Markers.push_back(marker);
	};

	int totalIssues = 0;

	if (m_ChkLoop->isChecked()) {
		auto results = swcdetection::loopDetection(segList);
		totalIssues += static_cast<int>(results.size());
		for (auto& r : results) {
			rows.push_back({"Loop", r.x, r.y, r.z,
							QString::fromStdString(r.description)});
			addMarker(r.x, r.y, r.z, 1.0f, 0.2f, 0.2f);
		}
	}

	if (m_ChkTip->isChecked()) {
		auto allPoint2SegIdMap =
			swcdetection::getWholeGrid2SegIDMap(segList);
		auto results =
			swcdetection::tipDetection(segList, allPoint2SegIdMap);
		totalIssues += static_cast<int>(results.size());
		for (auto& r : results) {
			rows.push_back({"Tip", r.x, r.y, r.z,
							QString::fromStdString(r.description)});
			addMarker(r.x, r.y, r.z, 1.0f, 0.8f, 0.0f);
		}
	}

	if (m_ChkBranching->isChecked()) {
		auto results = swcdetection::branchingDetection(segList);
		totalIssues += static_cast<int>(results.size());
		for (auto& r : results) {
			rows.push_back({"Branching", r.x, r.y, r.z,
							QString::fromStdString(r.description)});
			addMarker(r.x, r.y, r.z, 0.0f, 0.8f, 0.2f);
		}
	}

	if (m_ChkCrossing->isChecked()) {
		auto results = swcdetection::crossingDetection(segList);
		totalIssues += static_cast<int>(results.size());
		for (auto& r : results) {
			rows.push_back(
				{"Crossing", r.x1, r.y1, r.z1,
				 QString("(%1, %2, %3) <-> (%4, %5, %6)")
					 .arg(r.x1, 0, 'f', 2)
					 .arg(r.y1, 0, 'f', 2)
					 .arg(r.z1, 0, 'f', 2)
					 .arg(r.x2, 0, 'f', 2)
					 .arg(r.y2, 0, 'f', 2)
					 .arg(r.z2, 0, 'f', 2)});
			addMarker(r.x1, r.y1, r.z1, 0.8f, 0.2f, 0.8f);
			addMarker(r.x2, r.y2, r.z2, 0.8f, 0.2f, 0.8f);
		}
	}

	if (m_ChkSpecStructs->isChecked()) {
		auto results = swcdetection::specStructsDetection(segList);
		totalIssues += static_cast<int>(results.size());
		for (auto& r : results) {
			rows.push_back({"SpecStructs", r.x, r.y, r.z,
							QString::fromStdString(r.description)});
			addMarker(r.x, r.y, r.z, 0.0f, 0.8f, 0.8f);
		}
	}

	// 5. Populate table
	m_ResultTable->setRowCount(static_cast<int>(rows.size()));
	for (int i = 0; i < static_cast<int>(rows.size()); i++) {
		m_ResultTable->setItem(i, 0, new QTableWidgetItem(rows[i].type));
		m_ResultTable->setItem(
			i, 1,
			new QTableWidgetItem(QString::number(rows[i].x, 'f', 2)));
		m_ResultTable->setItem(
			i, 2,
			new QTableWidgetItem(QString::number(rows[i].y, 'f', 2)));
		m_ResultTable->setItem(
			i, 3,
			new QTableWidgetItem(QString::number(rows[i].z, 'f', 2)));
		m_ResultTable->setItem(
			i, 4, new QTableWidgetItem(rows[i].description));
	}

	m_ResultTable->resizeColumnsToContents();

	m_StatusLabel->setText(
		QString("Analysis complete. %1 issues found.").arg(totalIssues));
	m_RunButton->setEnabled(true);
	m_VisualizeButton->setEnabled(totalIssues > 0);
}

void EditorQualityControl::onVisualize() {
	SwcRendererCreateInfo info;
	info.mode = SwcRendererMode::eVisualizeOneSwc;
	info.markers = m_Markers;

	// Build legend from checked detection types
	if (m_ChkLoop->isChecked())
		info.markerLegend.push_back({"Loop", {1.0f, 0.2f, 0.2f}});
	if (m_ChkTip->isChecked())
		info.markerLegend.push_back({"Tip", {1.0f, 0.8f, 0.0f}});
	if (m_ChkBranching->isChecked())
		info.markerLegend.push_back({"Branching", {0.0f, 0.8f, 0.2f}});
	if (m_ChkCrossing->isChecked())
		info.markerLegend.push_back({"Crossing", {0.8f, 0.2f, 0.8f}});
	if (m_ChkSpecStructs->isChecked())
		info.markerLegend.push_back({"SpecStructs", {0.0f, 0.8f, 0.8f}});

	if (!m_SwcUuid.empty()) {
		// Server data: fetch SWC nodes via gRPC
		info.dataSource = DataSource::eLoadFromMemory;
		proto::GetSwcFullNodeDataResponse response;
		if (!WrappedCall::getSwcFullNodeDataByUuid(m_SwcUuid, response,
												   this)) {
			QMessageBox::critical(this, "Error",
								  "Failed to fetch SWC data for visualization.");
			return;
		}
		*info.swcData.mutable_swcdata() =
			response.swcnodedata().swcdata();
	} else {
		// Local file
		info.dataSource = DataSource::eLoadFromFile;
		info.swcPath = m_FilePath.toStdString();
	}

	auto* dialog = new SwcRendererDailog(std::move(info), this);
	dialog->setAttribute(Qt::WA_DeleteOnClose);
	dialog->setWindowTitle(
		QString::fromStdString(m_Name) + " - Quality Control Visualization");
	dialog->show();
}
