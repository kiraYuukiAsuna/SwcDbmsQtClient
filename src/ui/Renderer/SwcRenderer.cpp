#include "SwcRenderer.h"

SwcRenderer::SwcRenderer(SwcRendererCreateInfo createInfo, QWidget* parent) : m_CreateInfo(std::move(createInfo)),
                                                                              QOpenGLWidget(parent) {
    if (m_CreateInfo.dataSource == DataSource::eLoadFromFile) {
        ESwc swc(m_CreateInfo.swcPath);
        swc.ReadFromFile();
        m_NeuronUnits = swc.getValue();

        ESwc newSwc(m_CreateInfo.newSwcPath);
        newSwc.ReadFromFile();
        m_NewNeuronUnits = newSwc.getValue();
    }
    else if (m_CreateInfo.dataSource == DataSource::eLoadFromMemory) {
        for (auto&node: m_CreateInfo.swcData.swcdata()) {
            NeuronUnit unit;
            unit.n = node.swcnodeinternaldata().n();
            unit.type = node.swcnodeinternaldata().type();
            unit.x = node.swcnodeinternaldata().x();
            unit.y = node.swcnodeinternaldata().y();
            unit.z = node.swcnodeinternaldata().z();
            unit.radius = node.swcnodeinternaldata().radius();
            unit.parent = node.swcnodeinternaldata().parent();
            unit.seg_id = node.swcnodeinternaldata().seg_id();
            unit.level = node.swcnodeinternaldata().level();
            unit.mode = node.swcnodeinternaldata().mode();
            unit.timestamp = node.swcnodeinternaldata().timestamp();
            unit.feature_value = node.swcnodeinternaldata().feature_value();

            unit.uuid = node.base().uuid();

            m_NeuronUnits.push_back(unit);
        }

        for (auto&node: m_CreateInfo.newSwcData.swcdata()) {
            NeuronUnit unit;
            unit.n = node.swcnodeinternaldata().n();
            unit.type = node.swcnodeinternaldata().type();
            unit.x = node.swcnodeinternaldata().x();
            unit.y = node.swcnodeinternaldata().y();
            unit.z = node.swcnodeinternaldata().z();
            unit.radius = node.swcnodeinternaldata().radius();
            unit.parent = node.swcnodeinternaldata().parent();
            unit.seg_id = node.swcnodeinternaldata().seg_id();
            unit.level = node.swcnodeinternaldata().level();
            unit.mode = node.swcnodeinternaldata().mode();
            unit.timestamp = node.swcnodeinternaldata().timestamp();
            unit.feature_value = node.swcnodeinternaldata().feature_value();

            unit.uuid = node.base().uuid();

            m_NewNeuronUnits.push_back(unit);
        }
    }

    switch (m_CreateInfo.mode) {
        case SwcRendererMode::eVisualizeOneSwc: {
            break;
        }
        case SwcRendererMode::eVisualizeDiffSwc: {
            compareNeuronUnits(
                m_NeuronUnits, m_NewNeuronUnits, deletedUnits, addedUnits, modifiedUnits, unchangedUnits
            );
            break;
        }
        case SwcRendererMode::eVisualizeUser:
            break;
        case SwcRendererMode::eVisualizeTime:
            break;
    }

    for (int i = 0; i < m_NeuronUnits.size(); ++i) {
        n_to_index_map[m_NeuronUnits[i].n] = i;
    }

    for (int i = 0; i < m_NewNeuronUnits.size(); ++i) {
        new_n_to_index_map[m_NewNeuronUnits[i].n] = i;
    }

    for (int i = 0; i < m_NeuronUnits.size(); ++i) {
        if (m_NeuronUnits[i].parent != -1) {
            childMap[m_NeuronUnits[i].parent] = m_NeuronUnits[i];
        }
    }

    for (int i = 0; i < m_NewNeuronUnits.size(); ++i) {
        if (m_NewNeuronUnits[i].parent != -1) {
            newChildMap[m_NewNeuronUnits[i].parent] = m_NewNeuronUnits[i];
        }
    }
}

void SwcRenderer::renderOneSwc() {
    auto&units = m_NeuronUnits;

    // Find the min and max coordinates
    float minX = std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max();
    float minZ = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::min();
    float maxY = std::numeric_limits<float>::min();
    float maxZ = std::numeric_limits<float>::min();
    for (auto&node: units) {
        minX = (std::min)(minX, node.x);
        minY = (std::min)(minY, node.y);
        minZ = (std::min)(minZ, node.z);
        maxX = (std::max)(maxX, node.x);
        maxY = (std::max)(maxY, node.y);
        maxZ = (std::max)(maxZ, node.z);
    }

    // render point
    glPointSize(m_CreateInfo.style.pointSize);
    glBegin(GL_POINTS);
    for (auto&node: units) {
        float x = 2.0f * (node.x - minX) / (maxX - minX) - 1.0f;
        float y = 2.0f * (node.y - minY) / (maxY - minY) - 1.0f;
        float z = 2.0f * (node.z - minZ) / (maxZ - minZ) - 1.0f;
        glColor4f(m_CreateInfo.style.pointColor[0], m_CreateInfo.style.pointColor[1], m_CreateInfo.style.pointColor[2],
                  m_CreateInfo.style.pointColor[3]);
        glVertex3f(x, y, z);
    }
    glEnd();

    // Render lines
    glLineWidth(m_CreateInfo.style.lineWidth);
    glBegin(GL_LINES);
    glColor4f(m_CreateInfo.style.lineColor[0], m_CreateInfo.style.lineColor[1], m_CreateInfo.style.lineColor[2],
              m_CreateInfo.style.lineColor[3]);
    for (int i = 0; i < units.size(); ++i) {
        auto node = units[i];
        if (node.parent != -1) {
            auto iter = n_to_index_map.find(node.parent);
            if (iter == n_to_index_map.end()) {
                continue;
            }

            // Normalize the coordinates to the range [-1, 1]
            float x1 = 2.0f * (node.x - minX) / (maxX - minX) - 1.0f;
            float y1 = 2.0f * (node.y - minY) / (maxY - minY) - 1.0f;
            float z1 = 2.0f * (node.z - minZ) / (maxZ - minZ) - 1.0f;

            float x2 = 2.0f * (units[iter->second].x - minX) / (maxX - minX) - 1.0f;
            float y2 = 2.0f * (units[iter->second].y - minY) / (maxY - minY) - 1.0f;
            float z2 = 2.0f * (units[iter->second].z - minZ) / (maxZ - minZ) - 1.0f;

            glVertex3f(x1, y1, z1);
            glVertex3f(x2, y2, z2);
        }
    }
    glEnd();

    renderBoundingBox(minX, minY, minZ, maxX, maxY, maxZ);
}

void SwcRenderer::renderDiffSwc() {
    auto&oldUnits = m_NeuronUnits;
    auto&newUnits = m_NewNeuronUnits;

    // Find the min and max coordinates
    float minX = std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max();
    float minZ = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::min();
    float maxY = std::numeric_limits<float>::min();
    float maxZ = std::numeric_limits<float>::min();
    for (auto&node: oldUnits) {
        minX = (std::min)(minX, node.x);
        minY = (std::min)(minY, node.y);
        minZ = (std::min)(minZ, node.z);
        maxX = (std::max)(maxX, node.x);
        maxY = (std::max)(maxY, node.y);
        maxZ = (std::max)(maxZ, node.z);
    }
    for (auto&node: newUnits) {
        minX = (std::min)(minX, node.x);
        minY = (std::min)(minY, node.y);
        minZ = (std::min)(minZ, node.z);
        maxX = (std::max)(maxX, node.x);
        maxY = (std::max)(maxY, node.y);
        maxZ = (std::max)(maxZ, node.z);
    }

    // render point
    glPointSize(m_CreateInfo.style.pointSize);
    glBegin(GL_POINTS);
    for (auto&node: addedUnits) {
        float x = 2.0f * (node.x - minX) / (maxX - minX) - 1.0f;
        float y = 2.0f * (node.y - minY) / (maxY - minY) - 1.0f;
        float z = 2.0f * (node.z - minZ) / (maxZ - minZ) - 1.0f;
        glColor4f(m_CreateInfo.style.lineColorAdd[0], m_CreateInfo.style.lineColorAdd[1],
                  m_CreateInfo.style.lineColorAdd[2],
                  m_CreateInfo.style.lineColorAdd[3]);
        glVertex3f(x, y, z);
    }
    glEnd();

    glPointSize(m_CreateInfo.style.pointSize);
    glBegin(GL_POINTS);
    for (auto&node: deletedUnits) {
        float x = 2.0f * (node.x - minX) / (maxX - minX) - 1.0f;
        float y = 2.0f * (node.y - minY) / (maxY - minY) - 1.0f;
        float z = 2.0f * (node.z - minZ) / (maxZ - minZ) - 1.0f;
        glColor4f(m_CreateInfo.style.lineColorDelete[0], m_CreateInfo.style.lineColorDelete[1],
                  m_CreateInfo.style.lineColorDelete[2],
                  m_CreateInfo.style.lineColorDelete[3]);
        glVertex3f(x, y, z);
    }
    glEnd();

    glPointSize(m_CreateInfo.style.pointSize);
    glBegin(GL_POINTS);
    for (auto&node: modifiedUnits) {
        float x = 2.0f * (node.x - minX) / (maxX - minX) - 1.0f;
        float y = 2.0f * (node.y - minY) / (maxY - minY) - 1.0f;
        float z = 2.0f * (node.z - minZ) / (maxZ - minZ) - 1.0f;
        glColor4f(m_CreateInfo.style.lineColorModified[0], m_CreateInfo.style.lineColorModified[1],
                  m_CreateInfo.style.lineColorModified[2],
                  m_CreateInfo.style.lineColorModified[3]);
        glVertex3f(x, y, z);
    }
    glEnd();

    glPointSize(m_CreateInfo.style.pointSize);
    glBegin(GL_POINTS);
    for (auto&node: unchangedUnits) {
        float x = 2.0f * (node.x - minX) / (maxX - minX) - 1.0f;
        float y = 2.0f * (node.y - minY) / (maxY - minY) - 1.0f;
        float z = 2.0f * (node.z - minZ) / (maxZ - minZ) - 1.0f;
        glColor4f(m_CreateInfo.style.lineColor[0], m_CreateInfo.style.lineColor[1], m_CreateInfo.style.lineColor[2],
                  m_CreateInfo.style.lineColor[3]);
        glVertex3f(x, y, z);
    }
    glEnd();

    // Render lines
    glLineWidth(m_CreateInfo.style.lineWidth);
    glBegin(GL_LINES);
    glColor4f(m_CreateInfo.style.lineColorAdd[0], m_CreateInfo.style.lineColorAdd[1],
              m_CreateInfo.style.lineColorAdd[2],
              m_CreateInfo.style.lineColorAdd[3]);
    for (int i = 0; i < addedUnits.size(); ++i) {
        auto node = addedUnits[i];
        if (node.parent != -1) {
            auto iter = new_n_to_index_map.find(node.parent);
            if (iter == new_n_to_index_map.end()) {
                continue;
            }

            // Normalize the coordinates to the range [-1, 1]
            float x1 = 2.0f * (node.x - minX) / (maxX - minX) - 1.0f;
            float y1 = 2.0f * (node.y - minY) / (maxY - minY) - 1.0f;
            float z1 = 2.0f * (node.z - minZ) / (maxZ - minZ) - 1.0f;

            float x2 = 2.0f * (newUnits[iter->second].x - minX) / (maxX - minX) - 1.0f;
            float y2 = 2.0f * (newUnits[iter->second].y - minY) / (maxY - minY) - 1.0f;
            float z2 = 2.0f * (newUnits[iter->second].z - minZ) / (maxZ - minZ) - 1.0f;

            glVertex3f(x1, y1, z1);
            glVertex3f(x2, y2, z2);
        }
    }
    glEnd();

    glLineWidth(m_CreateInfo.style.lineWidth);
    glBegin(GL_LINES);
    glColor4f(m_CreateInfo.style.lineColorDelete[0], m_CreateInfo.style.lineColorDelete[1],
              m_CreateInfo.style.lineColorDelete[2],
              m_CreateInfo.style.lineColorDelete[3]);
    for (int i = 0; i < deletedUnits.size(); ++i) {
        auto node = deletedUnits[i];
        if (node.parent != -1) {
            auto iter = n_to_index_map.find(node.parent);
            if (iter == n_to_index_map.end()) {
                continue;
            }

            // Normalize the coordinates to the range [-1, 1]
            float x1 = 2.0f * (node.x - minX) / (maxX - minX) - 1.0f;
            float y1 = 2.0f * (node.y - minY) / (maxY - minY) - 1.0f;
            float z1 = 2.0f * (node.z - minZ) / (maxZ - minZ) - 1.0f;

            float x2 = 2.0f * (oldUnits[iter->second].x - minX) / (maxX - minX) - 1.0f;
            float y2 = 2.0f * (oldUnits[iter->second].y - minY) / (maxY - minY) - 1.0f;
            float z2 = 2.0f * (oldUnits[iter->second].z - minZ) / (maxZ - minZ) - 1.0f;

            glVertex3f(x1, y1, z1);
            glVertex3f(x2, y2, z2);
        }

        if(childMap.find(node.n)!=childMap.end()) {
            auto childNode = childMap[node.n];
            // Normalize the coordinates to the range [-1, 1]
            float x1 = 2.0f * (node.x - minX) / (maxX - minX) - 1.0f;
            float y1 = 2.0f * (node.y - minY) / (maxY - minY) - 1.0f;
            float z1 = 2.0f * (node.z - minZ) / (maxZ - minZ) - 1.0f;

            float x2 = 2.0f * (childNode.x - minX) / (maxX - minX) - 1.0f;
            float y2 = 2.0f * (childNode.y - minY) / (maxY - minY) - 1.0f;
            float z2 = 2.0f * (childNode.z - minZ) / (maxZ - minZ) - 1.0f;

            glVertex3f(x1, y1, z1);
            glVertex3f(x2, y2, z2);
        }
    }
    glEnd();

    glLineWidth(m_CreateInfo.style.lineWidth);
    glBegin(GL_LINES);
    glColor4f(m_CreateInfo.style.lineColorModified[0], m_CreateInfo.style.lineColorModified[1],
              m_CreateInfo.style.lineColorModified[2],
              m_CreateInfo.style.lineColorModified[3]);
    for (int i = 0; i < modifiedUnits.size(); ++i) {
        auto node = modifiedUnits[i];
        if (node.parent != -1) {
            auto iter = new_n_to_index_map.find(node.parent);
            if (iter == new_n_to_index_map.end()) {
                continue;
            }

            // Normalize the coordinates to the range [-1, 1]
            float x1 = 2.0f * (node.x - minX) / (maxX - minX) - 1.0f;
            float y1 = 2.0f * (node.y - minY) / (maxY - minY) - 1.0f;
            float z1 = 2.0f * (node.z - minZ) / (maxZ - minZ) - 1.0f;

            float x2 = 2.0f * (newUnits[iter->second].x - minX) / (maxX - minX) - 1.0f;
            float y2 = 2.0f * (newUnits[iter->second].y - minY) / (maxY - minY) - 1.0f;
            float z2 = 2.0f * (newUnits[iter->second].z - minZ) / (maxZ - minZ) - 1.0f;

            glVertex3f(x1, y1, z1);
            glVertex3f(x2, y2, z2);
        }

        if(childMap.find(node.n)!=childMap.end()) {
            auto childNode = childMap[node.n];
            // Normalize the coordinates to the range [-1, 1]
            float x1 = 2.0f * (node.x - minX) / (maxX - minX) - 1.0f;
            float y1 = 2.0f * (node.y - minY) / (maxY - minY) - 1.0f;
            float z1 = 2.0f * (node.z - minZ) / (maxZ - minZ) - 1.0f;

            float x2 = 2.0f * (childNode.x - minX) / (maxX - minX) - 1.0f;
            float y2 = 2.0f * (childNode.y - minY) / (maxY - minY) - 1.0f;
            float z2 = 2.0f * (childNode.z - minZ) / (maxZ - minZ) - 1.0f;

            glVertex3f(x1, y1, z1);
            glVertex3f(x2, y2, z2);
        }
    }
    glEnd();

    glLineWidth(m_CreateInfo.style.lineWidth);
    glBegin(GL_LINES);
    glColor4f(m_CreateInfo.style.lineColor[0], m_CreateInfo.style.lineColor[1], m_CreateInfo.style.lineColor[2],
              m_CreateInfo.style.lineColor[3]);
    for (int i = 0; i < unchangedUnits.size(); ++i) {
        auto node = unchangedUnits[i];
        if (node.parent != -1) {
            auto iter = new_n_to_index_map.find(node.parent);
            if (iter == new_n_to_index_map.end()) {
                continue;
            }

            // Normalize the coordinates to the range [-1, 1]
            float x1 = 2.0f * (node.x - minX) / (maxX - minX) - 1.0f;
            float y1 = 2.0f * (node.y - minY) / (maxY - minY) - 1.0f;
            float z1 = 2.0f * (node.z - minZ) / (maxZ - minZ) - 1.0f;

            float x2 = 2.0f * (newUnits[iter->second].x - minX) / (maxX - minX) - 1.0f;
            float y2 = 2.0f * (newUnits[iter->second].y - minY) / (maxY - minY) - 1.0f;
            float z2 = 2.0f * (newUnits[iter->second].z - minZ) / (maxZ - minZ) - 1.0f;

            glVertex3f(x1, y1, z1);
            glVertex3f(x2, y2, z2);
        }
    }
    glEnd();

    renderBoundingBox(minX, minY, minZ, maxX, maxY, maxZ);
}

void SwcRenderer::initializeGL() {
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST); // Enable depth testing
    glDepthRange(0, 1); // Adjust the depth range
    glClearColor(0, 0, 0, 1);

    glEnable(GL_BLEND); // Enable color blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Set blend function

    glEnable(GL_POINT_SMOOTH); // Enable point smoothing
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST); // Set the quality of point smoothing
}

void SwcRenderer::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float aspectRatio = static_cast<float>(w) / static_cast<float>(h);
    glOrtho(-aspectRatio, aspectRatio, -1.0, 1.0, -10.0, 10.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void SwcRenderer::mousePressEvent(QMouseEvent* event) {
    lastPos = event->pos();
}

void SwcRenderer::mouseMoveEvent(QMouseEvent* event) {
    int dx = event->x() - lastPos.x();
    int dy = event->y() - lastPos.y();

    if (event->buttons() & Qt::LeftButton) {
        setXRotation(xRot + 8 * dy);
        setYRotation(yRot + 8 * dx);
    }
    else if (event->buttons() & Qt::RightButton) {
        setXRotation(xRot + 8 * dy);
        setZRotation(zRot + 8 * dx);
    }
    lastPos = event->pos();
}

void SwcRenderer::wheelEvent(QWheelEvent* event) {
    if (event->angleDelta().y() > 0) {
        zoom *= 1.1;
    }
    else if (event->angleDelta().y() < 0) {
        zoom /= 1.1;
    }
    update();
}

void SwcRenderer::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotatef(xRot / 16.0f, 1.0f, 0.0f, 0.0f);
    glRotatef(yRot / 16.0f, 0.0f, 1.0f, 0.0f);
    glRotatef(zRot / 16.0f, 0.0f, 0.0f, 1.0f);
    glScalef(zoom, zoom, zoom); // Apply zoom

    switch (m_CreateInfo.mode) {
        case SwcRendererMode::eVisualizeOneSwc:
            renderOneSwc();
            break;
        case SwcRendererMode::eVisualizeDiffSwc:
            renderDiffSwc();
            break;
        case SwcRendererMode::eVisualizeUser:
            break;
        case SwcRendererMode::eVisualizeTime:
            break;
    }
}

void SwcRenderer::setXRotation(int angle) {
    normalizeAngle(&angle);
    if (angle != xRot) {
        xRot = angle;
        update();
    }
}

void SwcRenderer::setYRotation(int angle) {
    normalizeAngle(&angle);
    if (angle != yRot) {
        yRot = angle;
        update();
    }
}

void SwcRenderer::setZRotation(int angle) {
    normalizeAngle(&angle);
    if (angle != zRot) {
        zRot = angle;
        update();
    }
}

void SwcRenderer::normalizeAngle(int* angle) {
    while (*angle < 0) {
        *angle += 360 * 16;
    }
    while (*angle > 360 * 16) {
        *angle -= 360 * 16;
    }
}

void SwcRenderer::renderBoundingBox(float minX, float minY, float minZ, float maxX, float maxY, float maxZ) {
    // Draw bounding box
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glColor4f(m_CreateInfo.style.boundingBoxLineColor[0], m_CreateInfo.style.boundingBoxLineColor[1], m_CreateInfo.style.boundingBoxLineColor[2], m_CreateInfo.style.boundingBoxLineColor[3]);
    // Bottom square
    glVertex3f(2.0f * (minX - minX) / (maxX - minX) - 1.0f, 2.0f * (minY - minY) / (maxY - minY) - 1.0f,
               2.0f * (minZ - minZ) / (maxZ - minZ) - 1.0f);
    glVertex3f(2.0f * (maxX - minX) / (maxX - minX) - 1.0f, 2.0f * (minY - minY) / (maxY - minY) - 1.0f,
               2.0f * (minZ - minZ) / (maxZ - minZ) - 1.0f);

    glVertex3f(2.0f * (maxX - minX) / (maxX - minX) - 1.0f, 2.0f * (minY - minY) / (maxY - minY) - 1.0f,
               2.0f * (minZ - minZ) / (maxZ - minZ) - 1.0f);
    glVertex3f(2.0f * (maxX - minX) / (maxX - minX) - 1.0f, 2.0f * (maxY - minY) / (maxY - minY) - 1.0f,
               2.0f * (minZ - minZ) / (maxZ - minZ) - 1.0f);

    glVertex3f(2.0f * (maxX - minX) / (maxX - minX) - 1.0f, 2.0f * (maxY - minY) / (maxY - minY) - 1.0f,
               2.0f * (minZ - minZ) / (maxZ - minZ) - 1.0f);
    glVertex3f(2.0f * (minX - minX) / (maxX - minX) - 1.0f, 2.0f * (maxY - minY) / (maxY - minY) - 1.0f,
               2.0f * (minZ - minZ) / (maxZ - minZ) - 1.0f);

    glVertex3f(2.0f * (minX - minX) / (maxX - minX) - 1.0f, 2.0f * (maxY - minY) / (maxY - minY) - 1.0f,
               2.0f * (minZ - minZ) / (maxZ - minZ) - 1.0f);
    glVertex3f(2.0f * (minX - minX) / (maxX - minX) - 1.0f, 2.0f * (minY - minY) / (maxY - minY) - 1.0f,
               2.0f * (minZ - minZ) / (maxZ - minZ) - 1.0f);

    // Top square
    glVertex3f(2.0f * (minX - minX) / (maxX - minX) - 1.0f, 2.0f * (minY - minY) / (maxY - minY) - 1.0f,
               2.0f * (maxZ - minZ) / (maxZ - minZ) - 1.0f);
    glVertex3f(2.0f * (maxX - minX) / (maxX - minX) - 1.0f, 2.0f * (minY - minY) / (maxY - minY) - 1.0f,
               2.0f * (maxZ - minZ) / (maxZ - minZ) - 1.0f);

    glVertex3f(2.0f * (maxX - minX) / (maxX - minX) - 1.0f, 2.0f * (minY - minY) / (maxY - minY) - 1.0f,
               2.0f * (maxZ - minZ) / (maxZ - minZ) - 1.0f);
    glVertex3f(2.0f * (maxX - minX) / (maxX - minX) - 1.0f, 2.0f * (maxY - minY) / (maxY - minY) - 1.0f,
               2.0f * (maxZ - minZ) / (maxZ - minZ) - 1.0f);

    glVertex3f(2.0f * (maxX - minX) / (maxX - minX) - 1.0f, 2.0f * (maxY - minY) / (maxY - minY) - 1.0f,
               2.0f * (maxZ - minZ) / (maxZ - minZ) - 1.0f);
    glVertex3f(2.0f * (minX - minX) / (maxX - minX) - 1.0f, 2.0f * (maxY - minY) / (maxY - minY) - 1.0f,
               2.0f * (maxZ - minZ) / (maxZ - minZ) - 1.0f);

    glVertex3f(2.0f * (minX - minX) / (maxX - minX) - 1.0f, 2.0f * (maxY - minY) / (maxY - minY) - 1.0f,
               2.0f * (maxZ - minZ) / (maxZ - minZ) - 1.0f);
    glVertex3f(2.0f * (minX - minX) / (maxX - minX) - 1.0f, 2.0f * (minY - minY) / (maxY - minY) - 1.0f,
               2.0f * (maxZ - minZ) / (maxZ - minZ) - 1.0f);

    // Vertical lines
    glVertex3f(2.0f * (minX - minX) / (maxX - minX) - 1.0f, 2.0f * (minY - minY) / (maxY - minY) - 1.0f,
               2.0f * (minZ - minZ) / (maxZ - minZ) - 1.0f);
    glVertex3f(2.0f * (minX - minX) / (maxX - minX) - 1.0f, 2.0f * (minY - minY) / (maxY - minY) - 1.0f,
               2.0f * (maxZ - minZ) / (maxZ - minZ) - 1.0f);

    glVertex3f(2.0f * (maxX - minX) / (maxX - minX) - 1.0f, 2.0f * (minY - minY) / (maxY - minY) - 1.0f,
               2.0f * (minZ - minZ) / (maxZ - minZ) - 1.0f);
    glVertex3f(2.0f * (maxX - minX) / (maxX - minX) - 1.0f, 2.0f * (minY - minY) / (maxY - minY) - 1.0f,
               2.0f * (maxZ - minZ) / (maxZ - minZ) - 1.0f);

    glVertex3f(2.0f * (maxX - minX) / (maxX - minX) - 1.0f, 2.0f * (maxY - minY) / (maxY - minY) - 1.0f,
               2.0f * (minZ - minZ) / (maxZ - minZ) - 1.0f);
    glVertex3f(2.0f * (maxX - minX) / (maxX - minX) - 1.0f, 2.0f * (maxY - minY) / (maxY - minY) - 1.0f,
               2.0f * (maxZ - minZ) / (maxZ - minZ) - 1.0f);

    glVertex3f(2.0f * (minX - minX) / (maxX - minX) - 1.0f, 2.0f * (maxY - minY) / (maxY - minY) - 1.0f,
               2.0f * (minZ - minZ) / (maxZ - minZ) - 1.0f);
    glVertex3f(2.0f * (minX - minX) / (maxX - minX) - 1.0f, 2.0f * (maxY - minY) / (maxY - minY) - 1.0f,
               2.0f * (maxZ - minZ) / (maxZ - minZ) - 1.0f);
    glEnd();
}
