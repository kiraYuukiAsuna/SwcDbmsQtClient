#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include "src/FileIo/SwcIo.hpp"
#include <QMouseEvent>
#include <QWheelEvent>
#include <QMatrix4x4>
#include <limits>
#include <QVBoxLayout>
#include <utility>
#include "src/ui/ViewSwcApoData.h"

enum class SwcRendererMode {
    eVisualizeOneSwc,
    eVisualizeDiffSwc,
    eVisualizeUser,
    eVisualizeTime
};

enum class DataSource {
    eLoadFromFile,
    eLoadFromMemory
};

struct SwcStyle {
    float lineWidth = 2.0f;
    float pointSize = 3.0f;

    float lineColor[4] = {0.0f, 0.0f, 1.0f, 1.0f};
    float pointColor[4] = {0.0f, 0.0f, 1.0f, 1.0f};

    float lineColorAdd[4] = {0.0f, 1.0f, 0.0f, 1.0f};
    float lineColorDelete[4] = {1.0f, 0.0f, 0.0f, 1.0f};
    float lineColorModified[4] = {1.0f, 1.0f, 0.0f, 1.0f};
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

    void renderBoundingBox(float minX, float minY, float minZ, float maxX, float maxY, float maxZ);

    SwcRendererCreateInfo m_CreateInfo;

    std::vector<NeuronUnit> m_NeuronUnits;
    std::unordered_map<int, int> n_to_index_map;
    std::unordered_map<int,NeuronUnit> childMap;


    std::vector<NeuronUnit> m_NewNeuronUnits;
    std::unordered_map<int, int> new_n_to_index_map;
    std::unordered_map<int,NeuronUnit> newChildMap;


    std::vector<NeuronUnit> deletedUnits;
    std::vector<NeuronUnit> addedUnits;
    std::vector<NeuronUnit> modifiedUnits;
    std::vector<NeuronUnit> unchangedUnits;


    QPoint lastPos;
    int xRot = 0;
    int yRot = 0;
    int zRot = 0;
    float zoom = 1.0f;
};

class SwcRendererDailog : public QDialog {
public:
    SwcRendererDailog(SwcRendererCreateInfo createInfo, QWidget* parent): m_Renderer(std::move(createInfo), this),
                                                                          QDialog(parent) {
        setFixedSize({1600, 800});

        auto layout = new QVBoxLayout;
        layout->addWidget(&m_Renderer);
        setLayout(layout);
    }

private:
    SwcRenderer m_Renderer;
};
