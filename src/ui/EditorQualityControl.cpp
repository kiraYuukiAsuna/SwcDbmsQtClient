#include "EditorQualityControl.h"

#include <QDir>
#include <QFileInfo>
#include <QHeaderView>
#include <QMessageBox>
#include <QScrollArea>
#include <QVBoxLayout>

#include <fstream>
#include <functional>
#include <map>
#include <sstream>

#include "SWCDetection/SWCDetection.h"
#include "src/framework/service/WrappedCall.h"

namespace {

struct RawNode {
	int n, type, parent;
	float x, y, z, r;
};

// Build V_NeuronSWC_list from flat node list by DFS, splitting at branch points
V_NeuronSWC_list buildSegList(const std::vector<RawNode>& rawNodes) {
	V_NeuronSWC_list result;
	if (rawNodes.empty()) return result;

	std::map<int, int> nodeIndex;
	for (int i = 0; i < static_cast<int>(rawNodes.size()); i++) {
		nodeIndex[rawNodes[i].n] = i;
	}

	std::map<int, std::vector<int>> childMap;
	std::vector<int> roots;
	for (auto& node : rawNodes) {
		if (node.parent == -1) {
			roots.push_back(node.n);
		} else {
			childMap[node.parent].push_back(node.n);
		}
	}

	std::function<void(int, V_NeuronSWC&)> buildSegment =
		[&](int nodeId, V_NeuronSWC& currentSeg) {
			auto& rn = rawNodes[nodeIndex[nodeId]];
			V_NeuronSWC_unit unit;
			unit.n = currentSeg.row.size() + 1;
			unit.type = rn.type;
			unit.x = rn.x;
			unit.y = rn.y;
			unit.z = rn.z;
			unit.r = rn.r;
			unit.parent =
				currentSeg.row.empty() ? -1 : static_cast<int>(currentSeg.row.size());
			currentSeg.row.push_back(unit);

			auto it = childMap.find(nodeId);
			if (it == childMap.end() || it->second.empty()) {
				return;
			} else if (it->second.size() == 1) {
				buildSegment(it->second[0], currentSeg);
			} else {
				for (int childId : it->second) {
					V_NeuronSWC newSeg;
					V_NeuronSWC_unit branchUnit;
					branchUnit.n = 1;
					branchUnit.type = rn.type;
					branchUnit.x = rn.x;
					branchUnit.y = rn.y;
					branchUnit.z = rn.z;
					branchUnit.r = rn.r;
					branchUnit.parent = -1;
					newSeg.row.push_back(branchUnit);
					buildSegment(childId, newSeg);
					result.append(newSeg);
				}
			}
		};

	for (int rootId : roots) {
		V_NeuronSWC seg;
		buildSegment(rootId, seg);
		result.append(seg);
	}

	return result;
}

std::vector<RawNode> parseSwcFile(const std::string& filepath) {
	std::vector<RawNode> nodes;
	std::ifstream infile(filepath);
	if (!infile.is_open()) return nodes;

	std::string line;
	while (std::getline(infile, line)) {
		if (line.empty() || line[0] == '#') continue;
		std::istringstream iss(line);
		RawNode node;
		if (iss >> node.n >> node.type >> node.x >> node.y >> node.z >>
			node.r >> node.parent) {
			nodes.push_back(node);
		}
	}
	return nodes;
}

std::vector<RawNode> parseSwcFromProto(
	const proto::GetSwcFullNodeDataResponse& response) {
	std::vector<RawNode> nodes;
	const auto& swcData = response.swcnodedata();
	for (int i = 0; i < swcData.swcdata_size(); i++) {
		const auto& nd = swcData.swcdata(i).swcnodeinternaldata();
		RawNode node;
		node.n = nd.n();
		node.type = nd.type();
		node.x = static_cast<float>(nd.x());
		node.y = static_cast<float>(nd.y());
		node.z = static_cast<float>(nd.z());
		node.r = static_cast<float>(nd.radius());
		node.parent = nd.parent();
		nodes.push_back(node);
	}
	return nodes;
}

}  // namespace

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

void EditorQualityControl::runAnalysis() {
	m_StatusLabel->setText("Running analysis...");
	m_RunButton->setEnabled(false);

	// Load SWC data
	std::vector<RawNode> rawNodes;
	if (!m_SwcUuid.empty()) {
		proto::GetSwcFullNodeDataResponse response;
		if (!WrappedCall::getSwcFullNodeDataByUuid(m_SwcUuid, response, this)) {
			m_StatusLabel->setText("Failed to fetch SWC data from server.");
			m_RunButton->setEnabled(true);
			return;
		}
		rawNodes = parseSwcFromProto(response);
	} else {
		rawNodes = parseSwcFile(m_FilePath.toStdString());
	}

	if (rawNodes.empty()) {
		QMessageBox::critical(this, "Error",
							  "Failed to parse SWC data or data is empty.");
		m_StatusLabel->setText("Error: no SWC data.");
		m_RunButton->setEnabled(true);
		return;
	}

	V_NeuronSWC_list segList = buildSegList(rawNodes);

	if (segList.seg.empty()) {
		QMessageBox::critical(this, "Error",
							  "Failed to build segment list from SWC data.");
		m_StatusLabel->setText("Error: no segments.");
		m_RunButton->setEnabled(true);
		return;
	}

	// Collect results
	struct ResultRow {
		QString type;
		float x, y, z;
		QString description;
	};
	std::vector<ResultRow> rows;

	int totalIssues = 0;

	if (m_ChkLoop->isChecked()) {
		auto results = swcdetection::loopDetection(segList);
		totalIssues += static_cast<int>(results.size());
		for (auto& r : results) {
			rows.push_back({"Loop", r.x, r.y, r.z,
							QString::fromStdString(r.description)});
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
		}
	}

	if (m_ChkBranching->isChecked()) {
		auto results = swcdetection::branchingDetection(segList);
		totalIssues += static_cast<int>(results.size());
		for (auto& r : results) {
			rows.push_back({"Branching", r.x, r.y, r.z,
							QString::fromStdString(r.description)});
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
		}
	}

	if (m_ChkSpecStructs->isChecked()) {
		auto results = swcdetection::specStructsDetection(segList);
		totalIssues += static_cast<int>(results.size());
		for (auto& r : results) {
			rows.push_back({"SpecStructs", r.x, r.y, r.z,
							QString::fromStdString(r.description)});
		}
	}

	// Populate table
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
}
