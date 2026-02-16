#pragma once

#include <QCloseEvent>
#include <QDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QMatrix4x4>
#include <QMouseEvent>
#include <QOpenGLFunctions>
#include <QOpenGLWidget>
#include <QPushButton>
#include <QSlider>
#include <QVBoxLayout>

#include "Message/Message.pb.h"
#include "src/FileIo/SwcIo.hpp"

struct DiffEdge {
	float x1, y1, z1;	// child position
	float x2, y2, z2;	// parent position
};

struct SwcMarker {
	float x, y, z;
	float color[4]; // RGBA
};

struct MarkerLegendEntry {
	QString label;
	float color[3]; // RGB
};

enum class SwcRendererMode {
	eVisualizeOneSwc,
	eVisualizeDiffSwc,
	eVisualizeUser,
	eVisualizeTime
};

enum class DataSource { eLoadFromFile, eLoadFromMemory };

inline float backgroundColor[4] = {
	0.05f, 0.05f, 0.1f, 1.0f};	// Dark blue background for better contrast

struct SwcStyle {
	float lineWidth = 2.0f;
	float pointSize = 4.0f;	 // Slightly smaller points for cleaner look
	float markerPointSize = 12.0f;	// Large points for detection markers

	float boundingBoxLineColor[4] = {0.5f, 0.5f, 0.5f,
									 0.8f};	 // Semi-transparent gray
	float lineColor[4] = {0.2f, 0.6f, 1.0f,
						  0.9f};  // Bright blue with transparency
	float pointColor[4] = {0.2f, 0.6f, 1.0f, 1.0f};	 // Solid bright blue

	float lineColorAdd[4] = {0.0f, 0.8f, 0.2f,
							 0.9f};	 // Vibrant green for additions
	float lineColorDelete[4] = {1.0f, 0.2f, 0.2f,
								0.9f};	// Vibrant red for deletions
	float lineColorModified[4] = {1.0f, 0.7f, 0.0f,
								  0.9f};  // Orange for modifications
	float lineColorUnchanged[4] = {0.3f, 0.4f, 0.6f,
								   0.3f};  // Dim blue-gray for unchanged
};

struct SwcRendererCreateInfo {
	SwcRendererMode mode{SwcRendererMode::eVisualizeOneSwc};
	DataSource dataSource{DataSource::eLoadFromMemory};

	SwcStyle style{};

	std::string swcPath;
	proto::SwcDataV1 swcData;
	std::string newSwcPath;
	proto::SwcDataV1 newSwcData;
	proto::SwcIncrementOperationListV1 incrementOperationList;

	std::vector<SwcMarker> markers;
	std::vector<MarkerLegendEntry> markerLegend;
};

class SwcRenderer : public QOpenGLWidget, protected QOpenGLFunctions {
	Q_OBJECT

public:
	SwcRenderer(SwcRendererCreateInfo createInfo, QWidget* parent = nullptr);

	void renderOneSwc();

	void renderDiffSwc();

public slots:
	void resetView();
	void setMarkerPointSize(int size);

protected:
	void initializeGL() override;

	void resizeGL(int w, int h) override;

	void mousePressEvent(QMouseEvent* event) override;

	void mouseMoveEvent(QMouseEvent* event) override;

	void wheelEvent(QWheelEvent* event) override;

	void paintGL() override;

	void setXRotation(int angle);

	void setYRotation(int angle);

	void setZRotation(int angle);

	void normalizeAngle(int* angle);

	void renderMarkers(float centerX, float centerY, float centerZ,
					   float scale);

	void renderBoundingBox(float minX, float minY, float minZ, float maxX,
						   float maxY, float maxZ);

	void computeEdgeDiff(const std::vector<NeuronUnit>& oldUnits,
						 const std::vector<NeuronUnit>& newUnits);

	SwcRendererCreateInfo m_CreateInfo;

	std::vector<NeuronUnit> m_NeuronUnits;
	std::unordered_map<int, int> n_to_index_map;

	std::vector<NeuronUnit> m_NewNeuronUnits;

	// Edge-based diff results
	std::vector<DiffEdge> m_DeletedEdges;
	std::vector<DiffEdge> m_AddedEdges;
	std::vector<DiffEdge> m_ModifiedEdges;
	std::vector<DiffEdge> m_UnchangedEdges;
	float m_DiffMinX{}, m_DiffMinY{}, m_DiffMinZ{};
	float m_DiffMaxX{}, m_DiffMaxY{}, m_DiffMaxZ{};

	QPoint lastPos;
	int xRot = 15 * 16;	 // Initial slight downward angle for better view
	int yRot = 30 * 16;	 // Initial slight rotation for 3D perspective
	int zRot = 0;
	float zoom = 0.8f;	// Slightly zoomed out for better initial view
};

class SwcRendererDailog : public QDialog {
	Q_OBJECT

public:
	SwcRendererDailog(SwcRendererCreateInfo createInfo,
					  QWidget* parent = nullptr)
		: QDialog(parent), m_Renderer(createInfo, this) {
		QString title;
		switch (createInfo.mode) {
			case SwcRendererMode::eVisualizeOneSwc:
				title = "SWC 3D Visualization - Single Version";
				break;
			case SwcRendererMode::eVisualizeDiffSwc:
				title = "SWC 3D Visualization - Version Comparison";
				break;
			default:
				title = "SWC 3D Visualization";
				break;
		}

		setWindowTitle(title);
		setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint |
					   Qt::WindowMaximizeButtonHint |
					   Qt::WindowMinimizeButtonHint);

		resize(1600, 800);
		setMinimumSize(800, 600);

		auto layout = new QVBoxLayout(this);
		layout->setContentsMargins(5, 5, 5, 5);
		layout->addWidget(&m_Renderer, 1);

		// Legend row (only if legend entries exist)
		if (!createInfo.markerLegend.empty()) {
			auto legendLayout = new QHBoxLayout();
			auto legendTitle = new QLabel("Legend:", this);
			legendTitle->setStyleSheet(
				"font-weight: bold; font-size: 12px; padding: 2px 5px;");
			legendLayout->addWidget(legendTitle);

			for (auto& entry : createInfo.markerLegend) {
				auto colorBlock = new QLabel(this);
				colorBlock->setFixedSize(14, 14);
				colorBlock->setStyleSheet(
					QString("background-color: rgb(%1,%2,%3); "
							"border: 1px solid #888; border-radius: 2px;")
						.arg(static_cast<int>(entry.color[0] * 255))
						.arg(static_cast<int>(entry.color[1] * 255))
						.arg(static_cast<int>(entry.color[2] * 255)));
				auto label = new QLabel(entry.label, this);
				label->setStyleSheet("font-size: 12px; padding-right: 8px;");
				legendLayout->addWidget(colorBlock);
				legendLayout->addWidget(label);
			}
			legendLayout->addStretch();
			layout->addLayout(legendLayout);
		}

		// Bottom toolbar: help, marker size slider, reset button
		auto buttonLayout = new QHBoxLayout();

		auto helpLabel = new QLabel(
			"Left drag: rotate | Right drag: tilt | Wheel: zoom", this);
		helpLabel->setStyleSheet(
			"color: #666; font-size: 12px; padding: 5px;");
		buttonLayout->addWidget(helpLabel);
		buttonLayout->addStretch();

		// Marker size slider (only if markers exist)
		if (!createInfo.markers.empty()) {
			auto sizeLabel = new QLabel("Marker Size:", this);
			sizeLabel->setStyleSheet("font-size: 12px;");
			auto sizeSlider = new QSlider(Qt::Horizontal, this);
			sizeSlider->setRange(4, 30);
			sizeSlider->setValue(
				static_cast<int>(createInfo.style.markerPointSize));
			sizeSlider->setFixedWidth(120);
			sizeSlider->setToolTip("Adjust marker point size");
			connect(sizeSlider, &QSlider::valueChanged, &m_Renderer,
					&SwcRenderer::setMarkerPointSize);
			buttonLayout->addWidget(sizeLabel);
			buttonLayout->addWidget(sizeSlider);
		}

		auto resetViewButton = new QPushButton("Reset View", this);
		resetViewButton->setToolTip("Reset to optimal viewing angle");
		connect(resetViewButton, &QPushButton::clicked, &m_Renderer,
				&SwcRenderer::resetView);
		buttonLayout->addWidget(resetViewButton);

		layout->addLayout(buttonLayout);
		setLayout(layout);
	}

private:
	SwcRenderer m_Renderer;
};
