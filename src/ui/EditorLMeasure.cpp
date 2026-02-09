#include "EditorLMeasure.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QHeaderView>
#include <QScrollArea>
#include <QTextStream>
#include <QVBoxLayout>

#include "src/framework/service/WrappedCall.h"
#include "src/L-Measure/Elaboration.h"
#include "src/L-Measure/Neuron.h"
#include "src/L-Measure/Func.h"

namespace {

struct MetricDef {
	int id;
	const char* name;
};

constexpr MetricDef kMetrics[] = {
	{0, "Soma_Surface"},
	{1, "N_stems"},
	{2, "N_bifs"},
	{3, "N_branch"},
	{4, "N_tips"},
	{5, "Width"},
	{6, "Height"},
	{7, "Depth"},
	{8, "Type"},
	{9, "Diameter"},
	{10, "Diameter_pow"},
	{11, "Length"},
	{12, "Surface"},
	{13, "SectionArea"},
	{14, "Volume"},
	{15, "EucDistance"},
	{16, "PathDistance"},
	{18, "Branch_Order"},
	{19, "Terminal_degree"},
	{20, "TerminalSegment"},
	{21, "Taper_1"},
	{22, "Taper_2"},
	{23, "Branch_pathlength"},
	{24, "Contraction"},
	{25, "Fragmentation"},
	{26, "Daughter_Ratio"},
	{27, "Parent_Daughter_Ratio"},
	{28, "Partition_asymmetry"},
	{29, "Rall_Power"},
	{30, "Pk"},
	{31, "Pk_classic"},
	{32, "Pk_2"},
	{33, "Bif_ampl_local"},
	{34, "Bif_ampl_remote"},
	{35, "Bif_tilt_local"},
	{36, "Bif_tilt_remote"},
	{37, "Bif_torque_local"},
	{38, "Bif_torque_remote"},
	{39, "Last_parent_diam"},
	{40, "Diam_threshold"},
	{41, "HillmanThreshold"},
	{42, "Hausdorff"},
	{43, "Helix"},
	{44, "Fractal_Dim"},
};

Func* createFunc(int id, Func* limit) {
	switch (id) {
		case 0: return new Soma_Surface(limit);
		case 1: return new N_stems(limit);
		case 2: return new N_bifs(limit);
		case 3: return new N_branch(limit);
		case 4: return new N_tips(limit);
		case 5: return new Width(limit);
		case 6: return new Height(limit);
		case 7: return new Depth(limit);
		case 8: return new Type(limit);
		case 9: return new Diameter(limit);
		case 10: return new Diameter_pow(limit);
		case 11: return new Length(limit);
		case 12: return new Surface(limit);
		case 13: return new SectionArea(limit);
		case 14: return new Volume(limit);
		case 15: return new EucDistance(limit);
		case 16: return new PathDistance(limit);
		case 18: return new Branch_Order(limit);
		case 19: return new Terminal_degree(limit);
		case 20: return new TerminalSegment(limit);
		case 21: return new Taper_1(limit);
		case 22: return new Taper_2(limit);
		case 23: return new Branch_pathlength(limit);
		case 24: return new Contraction(limit);
		case 25: return new Fragmentation(limit);
		case 26: return new Daughter_Ratio(limit);
		case 27: return new Parent_Daughter_Ratio(limit);
		case 28: return new Partition_asymmetry(limit);
		case 29: return new Rall_Power(limit);
		case 30: return new Pk(limit);
		case 31: return new Pk_classic(limit);
		case 32: return new Pk_2(limit);
		case 33: return new Bif_ampl_local(limit);
		case 34: return new Bif_ampl_remote(limit);
		case 35: return new Bif_tilt_local(limit);
		case 36: return new Bif_tilt_remote(limit);
		case 37: return new Bif_torque_local(limit);
		case 38: return new Bif_torque_remote(limit);
		case 39: return new Last_parent_diam(limit);
		case 40: return new Diam_threshold(limit);
		case 41: return new HillmanThreshold(limit);
		case 42: return new Hausdorff(limit);
		case 43: return new Helix(limit);
		case 44: return new Fractal_Dim(limit);
		default: return new Zero();
	}
}

// Subclass of Single that exposes protected data/counter fields
class SingleAccessor : public Single {
public:
	SingleAccessor(ListElaboration* l, Func* aa)
		: Single(l, aa, 1, 0) {}

	double getTotalSum() const { return data[tab]; }
	int getCount() const { return counter[tab]; }
};

}  // namespace

EditorLMeasure::EditorLMeasure(const std::string& swcUuid,
							   const std::string& swcName, QWidget* parent)
	: QWidget(parent), m_SwcUuid(swcUuid), m_Name(swcName) {
	setupUi();
}

EditorLMeasure::EditorLMeasure(const QString& filePath, QWidget* parent)
	: QWidget(parent), m_FilePath(filePath) {
	QFileInfo fi(filePath);
	m_Name = fi.fileName().toStdString();
	setupUi();
}

EditorLMeasure::~EditorLMeasure() = default;

std::string EditorLMeasure::getUuid() {
	if (!m_SwcUuid.empty()) {
		return m_SwcUuid;
	}
	return m_FilePath.toStdString();
}

MetaInfoType EditorLMeasure::getMetaInfoType() {
	return MetaInfoType::eLMeasure;
}

bool EditorLMeasure::save() { return true; }

void EditorLMeasure::setupUi() {
	auto* mainLayout = new QHBoxLayout(this);

	// Left panel: metric selection
	auto* leftPanel = new QVBoxLayout;

	auto* titleLabel =
		new QLabel(QString::fromStdString(m_Name) + " - L-Measure Analysis");
	auto titleFont = titleLabel->font();
	titleFont.setBold(true);
	titleFont.setPointSize(titleFont.pointSize() + 2);
	titleLabel->setFont(titleFont);
	leftPanel->addWidget(titleLabel);

	auto* scrollArea = new QScrollArea;
	scrollArea->setWidgetResizable(true);
	scrollArea->setMinimumWidth(220);
	auto* scrollWidget = new QWidget;
	auto* checkboxLayout = new QVBoxLayout(scrollWidget);

	populateMetrics();
	for (auto& metric : m_Metrics) {
		checkboxLayout->addWidget(metric.checkbox);
	}
	checkboxLayout->addStretch();

	scrollArea->setWidget(scrollWidget);
	leftPanel->addWidget(scrollArea);

	// Button row
	auto* buttonLayout = new QHBoxLayout;
	m_SelectAllButton = new QPushButton("Select All");
	m_DeselectAllButton = new QPushButton("Deselect All");
	m_RunButton = new QPushButton("Run Analysis");

	connect(m_SelectAllButton, &QPushButton::clicked, this, [this]() {
		for (auto& m : m_Metrics) m.checkbox->setChecked(true);
	});
	connect(m_DeselectAllButton, &QPushButton::clicked, this, [this]() {
		for (auto& m : m_Metrics) m.checkbox->setChecked(false);
	});
	connect(m_RunButton, &QPushButton::clicked, this,
			&EditorLMeasure::runAnalysis);

	buttonLayout->addWidget(m_SelectAllButton);
	buttonLayout->addWidget(m_DeselectAllButton);
	leftPanel->addLayout(buttonLayout);
	leftPanel->addWidget(m_RunButton);

	m_StatusLabel = new QLabel("Ready");
	leftPanel->addWidget(m_StatusLabel);

	mainLayout->addLayout(leftPanel);

	// Right panel: results table
	m_ResultTable = new QTableWidget(this);
	m_ResultTable->setColumnCount(7);
	m_ResultTable->setHorizontalHeaderLabels(
		{"Metric", "TotalSum", "#Compartments", "Min", "Avg", "Max", "StdDev"});
	m_ResultTable->horizontalHeader()->setStretchLastSection(true);
	m_ResultTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_ResultTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	mainLayout->addWidget(m_ResultTable, 1);

	setLayout(mainLayout);
}

void EditorLMeasure::populateMetrics() {
	for (const auto& def : kMetrics) {
		MetricInfo info;
		info.id = def.id;
		info.name = def.name;
		info.checkbox = new QCheckBox(def.name, this);
		info.checkbox->setChecked(true);
		m_Metrics.push_back(info);
	}
}

bool EditorLMeasure::writeSwcToTempFile(const QString& tempPath) {
	if (!m_SwcUuid.empty()) {
		// Fetch from server
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
	// Local file - no need to write, already exists
	return true;
}

void EditorLMeasure::runAnalysis() {
	// Collect selected metrics
	std::vector<const MetricInfo*> selected;
	for (const auto& m : m_Metrics) {
		if (m.checkbox->isChecked()) {
			selected.push_back(&m);
		}
	}

	if (selected.empty()) {
		QMessageBox::warning(this, "Warning",
							 "Please select at least one metric.");
		return;
	}

	m_StatusLabel->setText("Running analysis...");
	m_RunButton->setEnabled(false);

	// Determine the SWC file path
	QString swcPath;
	QString tempFilePath;

	if (!m_SwcUuid.empty()) {
		// Server data: write to temp file
		tempFilePath =
			QDir::tempPath() + "/lmeasure_" +
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

	// Create the Neuron from the SWC file
	QByteArray pathBytes = swcPath.toLocal8Bit();
	auto* neuron = new Neuron(pathBytes.data());

	if (neuron->soma == nullptr) {
		QMessageBox::critical(this, "Error",
							  "Failed to parse SWC file. No soma found.");
		delete neuron;
		if (!tempFilePath.isEmpty()) QFile::remove(tempFilePath);
		m_StatusLabel->setText("Error: invalid SWC file.");
		m_RunButton->setEnabled(true);
		return;
	}

	neuron->init();

	// Set up result table
	m_ResultTable->setRowCount(static_cast<int>(selected.size()));

	// For each selected metric, create a Single elaboration and compute
	for (int row = 0; row < static_cast<int>(selected.size()); row++) {
		const auto* metric = selected[row];

		auto* listElab = new ListElaboration();
		Func* func = createFunc(metric->id, nullptr);
		auto* single = new SingleAccessor(listElab, func);

		listElab->newNeuron();
		listElab->preCompute(neuron);
		listElab->compute(neuron);

		double totalSum = single->getTotalSum();
		int count = single->getCount();
		double minVal = single->getMin();
		double avgVal = single->getMean();
		double maxVal = single->getMax();
		double stdVal = single->getStd();

		// For Width/Height/Depth, totalSum should be max
		QString metricName = metric->name;
		if (metricName == "Width" || metricName == "Height" ||
			metricName == "Depth") {
			totalSum = maxVal;
		}

		m_ResultTable->setItem(
			row, 0, new QTableWidgetItem(metric->name));
		m_ResultTable->setItem(
			row, 1, new QTableWidgetItem(QString::number(totalSum, 'f', 4)));
		m_ResultTable->setItem(
			row, 2, new QTableWidgetItem(QString::number(count)));
		m_ResultTable->setItem(
			row, 3, new QTableWidgetItem(QString::number(minVal, 'f', 4)));
		m_ResultTable->setItem(
			row, 4, new QTableWidgetItem(QString::number(avgVal, 'f', 4)));
		m_ResultTable->setItem(
			row, 5, new QTableWidgetItem(QString::number(maxVal, 'f', 4)));
		m_ResultTable->setItem(
			row, 6, new QTableWidgetItem(QString::number(stdVal, 'f', 4)));
	}

	m_ResultTable->resizeColumnsToContents();

	delete neuron;
	if (!tempFilePath.isEmpty()) {
		QFile::remove(tempFilePath);
	}

	m_StatusLabel->setText(
		QString("Analysis complete. %1 metrics computed.")
			.arg(selected.size()));
	m_RunButton->setEnabled(true);
}
