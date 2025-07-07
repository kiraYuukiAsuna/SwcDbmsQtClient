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
#include <QVBoxLayout>
#include <QWheelEvent>

#include "SwcDbmsCommon/Generated/cpp/Message/Message.pb.h"
#include "src/FileIo/SwcIo.hpp"

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
};

class SwcRenderer : public QOpenGLWidget, protected QOpenGLFunctions {
	Q_OBJECT

public:
	SwcRenderer(SwcRendererCreateInfo createInfo, QWidget* parent = nullptr);

	void renderOneSwc();

	void renderDiffSwc();

public slots:
	void resetView();  // Reset view to optimal position

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

	void renderBoundingBox(float minX, float minY, float minZ, float maxX,
						   float maxY, float maxZ);

	void compareNeuronUnits(const std::vector<NeuronUnit>& oldUnits,
							const std::vector<NeuronUnit>& newUnits,
							std::vector<NeuronUnit>& deleted,
							std::vector<NeuronUnit>& added,
							std::vector<NeuronUnit>& modified,
							std::vector<NeuronUnit>& unchanged);

	SwcRendererCreateInfo m_CreateInfo;

	std::vector<NeuronUnit> m_NeuronUnits;
	std::unordered_map<int, int> n_to_index_map;
	std::unordered_map<int, NeuronUnit> childMap;

	std::vector<NeuronUnit> m_NewNeuronUnits;
	std::unordered_map<int, int> new_n_to_index_map;
	std::unordered_map<int, NeuronUnit> newChildMap;

	std::vector<NeuronUnit> deletedUnits;
	std::vector<NeuronUnit> addedUnits;
	std::vector<NeuronUnit> modifiedUnits;
	std::vector<NeuronUnit> unchangedUnits;

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
		// 根据模式设置不同的窗口标题
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

		// 设置窗口属性
		setWindowTitle(title);
		setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint |
					   Qt::WindowMaximizeButtonHint |
					   Qt::WindowMinimizeButtonHint);

		// 设置窗口大小
		resize(1600, 800);
		setMinimumSize(800, 600);

		// 设置布局
		auto layout = new QVBoxLayout(this);
		layout->setContentsMargins(5, 5, 5, 5);
		layout->addWidget(&m_Renderer);

		// 添加重置视角按钮和帮助信息
		auto buttonLayout = new QHBoxLayout();

		// 帮助标签
		auto helpLabel = new QLabel(
			"💡 Left click + drag: rotate, Right click + drag: tilt, Mouse "
			"wheel: zoom",
			this);
		helpLabel->setStyleSheet("color: #666; font-size: 12px; padding: 5px;");

		auto resetViewButton = new QPushButton("Reset View", this);
		resetViewButton->setToolTip("Reset to optimal viewing angle");
		connect(resetViewButton, &QPushButton::clicked, &m_Renderer,
				&SwcRenderer::resetView);

		buttonLayout->addWidget(helpLabel);
		buttonLayout->addStretch();
		buttonLayout->addWidget(resetViewButton);

		layout->addLayout(buttonLayout);
		setLayout(layout);
	}

private:
	SwcRenderer m_Renderer;
};
