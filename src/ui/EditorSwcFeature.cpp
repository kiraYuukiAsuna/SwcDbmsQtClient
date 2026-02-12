#include "EditorSwcFeature.h"

#include <QDir>
#include <QFileInfo>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QTextStream>
#include <QVBoxLayout>

#include "src/framework/service/WrappedCall.h"
#include "src/SWCFeature/global_neuron_feature/compute_morph.h"
#include "src/SWCFeature/pruning_swc/pruning_swc_algo.h"
#include "src/SWCFeature/resample_swc/resampling.h"
#include "src/SWCFeature/sort_neuron_swc/sort_swc.h"
#include <QApplication>

namespace {

struct FeatureDef {
	int index;
	const char* name;
};

constexpr FeatureDef kFeatures[] = {
	{0, "Number of Nodes"},
	{1, "Soma Surface"},
	{2, "Number of Stems"},
	{3, "Number of Bifurcations"},
	{4, "Number of Branches"},
	{5, "Number of Tips"},
	{6, "Overall Width"},
	{7, "Overall Height"},
	{8, "Overall Depth"},
	{9, "Average Diameter"},
	{10, "Total Length"},
	{11, "Total Surface"},
	{12, "Total Volume"},
	{13, "Max Euclidean Distance"},
	{14, "Max Path Distance"},
	{15, "Max Branch Order"},
	{16, "Average Contraction"},
	{17, "Average Fragmentation"},
	{18, "Average Parent-daughter Ratio"},
	{19, "Average Bifurcation Angle Local"},
	{20, "Average Bifurcation Angle Remote"},
	{21, "Hausdorff Dimension"},
};

NeuronTree buildHashNeuron(QList<NeuronSWC>& list) {
	NeuronTree nt;
	nt.listNeuron = list;
	for (int i = 0; i < list.size(); i++) {
		nt.hashNeuron.insert(list[i].n, i);
	}
	return nt;
}

}  // namespace

EditorSwcFeature::EditorSwcFeature(const std::string& swcUuid,
								   const std::string& swcName, QWidget* parent)
	: QWidget(parent), m_SwcUuid(swcUuid), m_Name(swcName) {
	setupUi();
}

EditorSwcFeature::EditorSwcFeature(const QString& filePath, QWidget* parent)
	: QWidget(parent), m_FilePath(filePath) {
	QFileInfo fi(filePath);
	m_Name = fi.fileName().toStdString();
	setupUi();
}

EditorSwcFeature::~EditorSwcFeature() = default;

std::string EditorSwcFeature::getUuid() {
	if (!m_SwcUuid.empty()) {
		return m_SwcUuid;
	}
	return m_FilePath.toStdString();
}

MetaInfoType EditorSwcFeature::getMetaInfoType() {
	return MetaInfoType::eSwcFeature;
}

bool EditorSwcFeature::save() { return true; }

void EditorSwcFeature::setupUi() {
	auto* mainLayout = new QHBoxLayout(this);

	// Left panel: preprocessing parameters
	auto* leftPanel = new QVBoxLayout;

	auto* titleLabel = new QLabel(
		QString::fromStdString(m_Name) + " - SWC Feature Analysis");
	auto titleFont = titleLabel->font();
	titleFont.setBold(true);
	titleFont.setPointSize(titleFont.pointSize() + 2);
	titleLabel->setFont(titleFont);
	leftPanel->addWidget(titleLabel);

	// Sort
	auto* sortGroup = new QGroupBox("Preprocessing");
	auto* sortLayout = new QVBoxLayout(sortGroup);

	m_ChkSort = new QCheckBox("Sort", this);
	m_ChkSort->setChecked(true);
	m_ChkSort->setToolTip(
		"Sort the neuron tree to ensure valid topology (root, parent links)");
	sortLayout->addWidget(m_ChkSort);

	// Prune
	m_ChkPrune = new QCheckBox("Prune", this);
	m_ChkPrune->setChecked(true);
	m_ChkPrune->setToolTip(
		"Iteratively remove short terminal branches (noise)");
	sortLayout->addWidget(m_ChkPrune);

	auto* pruneParamLayout = new QHBoxLayout;
	pruneParamLayout->addSpacing(20);
	pruneParamLayout->addWidget(new QLabel("Threshold:"));
	m_SpinPruneThreshold = new QDoubleSpinBox(this);
	m_SpinPruneThreshold->setRange(0.1, 100000.0);
	m_SpinPruneThreshold->setValue(2.0);
	m_SpinPruneThreshold->setDecimals(1);
	m_SpinPruneThreshold->setToolTip(
		"Branches shorter than this length (pixels) will be removed");
	pruneParamLayout->addWidget(m_SpinPruneThreshold);
	sortLayout->addLayout(pruneParamLayout);

	connect(m_ChkPrune, &QCheckBox::toggled, m_SpinPruneThreshold,
			&QWidget::setEnabled);

	// Resample
	m_ChkResample = new QCheckBox("Resample", this);
	m_ChkResample->setChecked(true);
	m_ChkResample->setToolTip(
		"Resample the neuron tree with a uniform step length");
	sortLayout->addWidget(m_ChkResample);

	auto* resampleParamLayout = new QHBoxLayout;
	resampleParamLayout->addSpacing(20);
	resampleParamLayout->addWidget(new QLabel("Step:"));
	m_SpinResampleStep = new QDoubleSpinBox(this);
	m_SpinResampleStep->setRange(0.1, 1000.0);
	m_SpinResampleStep->setValue(1.0);
	m_SpinResampleStep->setDecimals(1);
	m_SpinResampleStep->setToolTip("Resampling step length (pixels)");
	resampleParamLayout->addWidget(m_SpinResampleStep);
	sortLayout->addLayout(resampleParamLayout);

	connect(m_ChkResample, &QCheckBox::toggled, m_SpinResampleStep,
			&QWidget::setEnabled);

	leftPanel->addWidget(sortGroup);
	leftPanel->addStretch();

	m_RunButton = new QPushButton("Run Analysis");
	connect(m_RunButton, &QPushButton::clicked, this,
			&EditorSwcFeature::runAnalysis);
	leftPanel->addWidget(m_RunButton);

	m_StatusLabel = new QLabel("Ready");
	leftPanel->addWidget(m_StatusLabel);

	auto* leftWidget = new QWidget;
	leftWidget->setLayout(leftPanel);
	leftWidget->setMinimumWidth(240);
	leftWidget->setMaximumWidth(320);
	mainLayout->addWidget(leftWidget);

	// Right panel: results table
	m_ResultTable = new QTableWidget(this);
	m_ResultTable->setColumnCount(2);
	m_ResultTable->setHorizontalHeaderLabels({"Feature", "Value"});
	m_ResultTable->horizontalHeader()->setStretchLastSection(true);
	m_ResultTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_ResultTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	mainLayout->addWidget(m_ResultTable, 1);

	setLayout(mainLayout);

	// Auto-run on open
	runAnalysis();
}

bool EditorSwcFeature::writeSwcToTempFile(const QString& tempPath) {
	if (!m_SwcUuid.empty()) {
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
	return true;
}

void EditorSwcFeature::runAnalysis() {
	m_StatusLabel->setText("Running analysis...");
	m_RunButton->setEnabled(false);

	// 1. Load SWC
	QString swcPath;
	QString tempFilePath;

	if (!m_SwcUuid.empty()) {
		tempFilePath = QDir::tempPath() + "/swcfeature_" +
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

	int originalSize = nt.listNeuron.size();
	QString pipelineLog;

	// 2. Sort
	if (m_ChkSort->isChecked()) {
		m_StatusLabel->setText("Sorting...");
		QApplication::processEvents();
		QList<NeuronSWC> sorted;
		if (SortSWC(nt.listNeuron, sorted, VOID_VALUE, VOID_VALUE)) {
			nt = buildHashNeuron(sorted);
			pipelineLog +=
				QString("Sort: %1 nodes. ").arg(nt.listNeuron.size());
		}
	}

	// 3. Prune
	if (m_ChkPrune->isChecked()) {
		m_StatusLabel->setText("Pruning...");
		QApplication::processEvents();
		double pruneThreshold = m_SpinPruneThreshold->value();
		bool pruned = false;
		nt = prune_swc_iterative(nt, pruneThreshold, pruned);
		pipelineLog +=
			QString("Prune(%.1f): %2 nodes. ")
				.arg(pruneThreshold)
				.arg(nt.listNeuron.size());
	}

	// 4. Resample
	if (m_ChkResample->isChecked()) {
		m_StatusLabel->setText("Resampling...");
		QApplication::processEvents();
		double resampleStep = m_SpinResampleStep->value();
		nt = resample(nt, resampleStep);
		pipelineLog +=
			QString("Resample(%.1f): %2 nodes. ")
				.arg(resampleStep)
				.arg(nt.listNeuron.size());
	}

	if (nt.listNeuron.isEmpty()) {
		QMessageBox::critical(
			this, "Error",
			"Neuron tree is empty after preprocessing. "
			"Try reducing the prune threshold.");
		m_StatusLabel->setText("Error: empty after preprocessing.");
		m_RunButton->setEnabled(true);
		return;
	}

	// 5. Compute features
	m_StatusLabel->setText("Computing features...");
	QApplication::processEvents();

	double features[FNUM] = {};
	computeFeature(nt, features);

	constexpr int featureCount =
		static_cast<int>(sizeof(kFeatures) / sizeof(kFeatures[0]));
	m_ResultTable->setRowCount(featureCount);

	for (int i = 0; i < featureCount; i++) {
		int idx = kFeatures[i].index;
		m_ResultTable->setItem(i, 0,
							   new QTableWidgetItem(kFeatures[i].name));
		m_ResultTable->setItem(
			i, 1,
			new QTableWidgetItem(QString::number(features[idx], 'f', 4)));
	}

	m_ResultTable->resizeColumnsToContents();

	m_StatusLabel->setText(
		QString("Done. Original: %1 nodes. %2")
			.arg(originalSize)
			.arg(pipelineLog.trimmed()));
	m_RunButton->setEnabled(true);
}
