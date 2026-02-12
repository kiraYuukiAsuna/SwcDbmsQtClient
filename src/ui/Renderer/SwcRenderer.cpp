#include "SwcRenderer.h"

#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

SwcRenderer::SwcRenderer(SwcRendererCreateInfo createInfo, QWidget* parent)
	: m_CreateInfo(std::move(createInfo)), QOpenGLWidget(parent) {
	if (m_CreateInfo.dataSource == DataSource::eLoadFromFile) {
		ESwc swc(m_CreateInfo.swcPath);
		swc.ReadFromFile();
		m_NeuronUnits = swc.getValue();

		ESwc newSwc(m_CreateInfo.newSwcPath);
		newSwc.ReadFromFile();
		m_NewNeuronUnits = newSwc.getValue();
	} else if (m_CreateInfo.dataSource == DataSource::eLoadFromMemory) {
		for (auto& node : m_CreateInfo.swcData.swcdata()) {
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

		for (auto& node : m_CreateInfo.newSwcData.swcdata()) {
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
			compareNeuronUnits(m_NeuronUnits, m_NewNeuronUnits, deletedUnits,
							   addedUnits, modifiedUnits, unchangedUnits);
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
	auto& units = m_NeuronUnits;

	// Find the min and max coordinates
	float minX = std::numeric_limits<float>::max();
	float minY = std::numeric_limits<float>::max();
	float minZ = std::numeric_limits<float>::max();
	float maxX = std::numeric_limits<float>::min();
	float maxY = std::numeric_limits<float>::min();
	float maxZ = std::numeric_limits<float>::min();
	for (auto& node : units) {
		minX = (std::min)(minX, node.x);
		minY = (std::min)(minY, node.y);
		minZ = (std::min)(minZ, node.z);
		maxX = (std::max)(maxX, node.x);
		maxY = (std::max)(maxY, node.y);
		maxZ = (std::max)(maxZ, node.z);
	}

	// Calculate center and scale for optimal viewing
	float centerX = (minX + maxX) / 2.0f;
	float centerY = (minY + maxY) / 2.0f;
	float centerZ = (minZ + maxZ) / 2.0f;

	float rangeX = maxX - minX;
	float rangeY = maxY - minY;
	float rangeZ = maxZ - minZ;
	float maxRange = (std::max)((std::max)(rangeX, rangeY), rangeZ);

	// Add padding to ensure complete visibility
	float scale = maxRange > 0 ? 1.6f / maxRange : 1.0f;

	// render point
	glPointSize(m_CreateInfo.style.pointSize);
	glBegin(GL_POINTS);
	for (auto& node : units) {
		// Center and scale coordinates
		float x = (node.x - centerX) * scale;
		float y = (node.y - centerY) * scale;
		float z = (node.z - centerZ) * scale;
		glColor4f(
			m_CreateInfo.style.pointColor[0], m_CreateInfo.style.pointColor[1],
			m_CreateInfo.style.pointColor[2], m_CreateInfo.style.pointColor[3]);
		glVertex3f(x, y, z);
	}
	glEnd();

	// Render lines
	glLineWidth(m_CreateInfo.style.lineWidth);
	glBegin(GL_LINES);
	glColor4f(m_CreateInfo.style.lineColor[0], m_CreateInfo.style.lineColor[1],
			  m_CreateInfo.style.lineColor[2], m_CreateInfo.style.lineColor[3]);
	for (int i = 0; i < units.size(); ++i) {
		auto node = units[i];
		if (node.parent != -1) {
			auto iter = n_to_index_map.find(node.parent);
			if (iter == n_to_index_map.end()) {
				continue;
			}

			// Center and scale coordinates
			float x1 = (node.x - centerX) * scale;
			float y1 = (node.y - centerY) * scale;
			float z1 = (node.z - centerZ) * scale;

			float x2 = (units[iter->second].x - centerX) * scale;
			float y2 = (units[iter->second].y - centerY) * scale;
			float z2 = (units[iter->second].z - centerZ) * scale;

			glVertex3f(x1, y1, z1);
			glVertex3f(x2, y2, z2);
		}
	}
	glEnd();

	renderBoundingBox(minX, minY, minZ, maxX, maxY, maxZ);

	// Render detection markers on top
	renderMarkers(centerX, centerY, centerZ, scale);
}

void SwcRenderer::renderDiffSwc() {
	auto& oldUnits = m_NeuronUnits;
	auto& newUnits = m_NewNeuronUnits;

	// Find the min and max coordinates from both datasets
	float minX = std::numeric_limits<float>::max();
	float minY = std::numeric_limits<float>::max();
	float minZ = std::numeric_limits<float>::max();
	float maxX = std::numeric_limits<float>::min();
	float maxY = std::numeric_limits<float>::min();
	float maxZ = std::numeric_limits<float>::min();
	for (auto& node : oldUnits) {
		minX = (std::min)(minX, node.x);
		minY = (std::min)(minY, node.y);
		minZ = (std::min)(minZ, node.z);
		maxX = (std::max)(maxX, node.x);
		maxY = (std::max)(maxY, node.y);
		maxZ = (std::max)(maxZ, node.z);
	}
	for (auto& node : newUnits) {
		minX = (std::min)(minX, node.x);
		minY = (std::min)(minY, node.y);
		minZ = (std::min)(minZ, node.z);
		maxX = (std::max)(maxX, node.x);
		maxY = (std::max)(maxY, node.y);
		maxZ = (std::max)(maxZ, node.z);
	}

	// Calculate center and scale for optimal viewing
	float centerX = (minX + maxX) / 2.0f;
	float centerY = (minY + maxY) / 2.0f;
	float centerZ = (minZ + maxZ) / 2.0f;

	float rangeX = maxX - minX;
	float rangeY = maxY - minY;
	float rangeZ = maxZ - minZ;
	float maxRange = (std::max)((std::max)(rangeX, rangeY), rangeZ);

	// Add padding to ensure complete visibility
	float scale = maxRange > 0 ? 1.6f / maxRange : 1.0f;

	// render point for added units
	glPointSize(m_CreateInfo.style.pointSize);
	glBegin(GL_POINTS);
	for (auto& node : addedUnits) {
		float x = (node.x - centerX) * scale;
		float y = (node.y - centerY) * scale;
		float z = (node.z - centerZ) * scale;
		glColor4f(m_CreateInfo.style.lineColorAdd[0],
				  m_CreateInfo.style.lineColorAdd[1],
				  m_CreateInfo.style.lineColorAdd[2],
				  m_CreateInfo.style.lineColorAdd[3]);
		glVertex3f(x, y, z);
	}
	glEnd();

	glPointSize(m_CreateInfo.style.pointSize);
	glBegin(GL_POINTS);
	for (auto& node : deletedUnits) {
		float x = (node.x - centerX) * scale;
		float y = (node.y - centerY) * scale;
		float z = (node.z - centerZ) * scale;
		glColor4f(m_CreateInfo.style.lineColorDelete[0],
				  m_CreateInfo.style.lineColorDelete[1],
				  m_CreateInfo.style.lineColorDelete[2],
				  m_CreateInfo.style.lineColorDelete[3]);
		glVertex3f(x, y, z);
	}
	glEnd();

	glPointSize(m_CreateInfo.style.pointSize);
	glBegin(GL_POINTS);
	for (auto& node : modifiedUnits) {
		float x = (node.x - centerX) * scale;
		float y = (node.y - centerY) * scale;
		float z = (node.z - centerZ) * scale;
		glColor4f(m_CreateInfo.style.lineColorModified[0],
				  m_CreateInfo.style.lineColorModified[1],
				  m_CreateInfo.style.lineColorModified[2],
				  m_CreateInfo.style.lineColorModified[3]);
		glVertex3f(x, y, z);
	}
	glEnd();

	glPointSize(m_CreateInfo.style.pointSize);
	glBegin(GL_POINTS);
	for (auto& node : unchangedUnits) {
		float x = (node.x - centerX) * scale;
		float y = (node.y - centerY) * scale;
		float z = (node.z - centerZ) * scale;
		glColor4f(
			m_CreateInfo.style.lineColor[0], m_CreateInfo.style.lineColor[1],
			m_CreateInfo.style.lineColor[2], m_CreateInfo.style.lineColor[3]);
		glVertex3f(x, y, z);
	}
	glEnd();

	// Render lines for added units
	glLineWidth(m_CreateInfo.style.lineWidth);
	glBegin(GL_LINES);
	glColor4f(
		m_CreateInfo.style.lineColorAdd[0], m_CreateInfo.style.lineColorAdd[1],
		m_CreateInfo.style.lineColorAdd[2], m_CreateInfo.style.lineColorAdd[3]);
	for (int i = 0; i < addedUnits.size(); ++i) {
		auto node = addedUnits[i];
		if (node.parent != -1) {
			auto iter = new_n_to_index_map.find(node.parent);
			if (iter == new_n_to_index_map.end()) {
				continue;
			}

			// Center and scale coordinates
			float x1 = (node.x - centerX) * scale;
			float y1 = (node.y - centerY) * scale;
			float z1 = (node.z - centerZ) * scale;

			float x2 = (newUnits[iter->second].x - centerX) * scale;
			float y2 = (newUnits[iter->second].y - centerY) * scale;
			float z2 = (newUnits[iter->second].z - centerZ) * scale;

			glVertex3f(x1, y1, z1);
			glVertex3f(x2, y2, z2);
		}
	}
	glEnd();

	glLineWidth(m_CreateInfo.style.lineWidth);
	glBegin(GL_LINES);
	glColor4f(m_CreateInfo.style.lineColorDelete[0],
			  m_CreateInfo.style.lineColorDelete[1],
			  m_CreateInfo.style.lineColorDelete[2],
			  m_CreateInfo.style.lineColorDelete[3]);
	for (int i = 0; i < deletedUnits.size(); ++i) {
		auto node = deletedUnits[i];
		if (node.parent != -1) {
			auto iter = n_to_index_map.find(node.parent);
			if (iter == n_to_index_map.end()) {
				continue;
			}

			// Center and scale coordinates
			float x1 = (node.x - centerX) * scale;
			float y1 = (node.y - centerY) * scale;
			float z1 = (node.z - centerZ) * scale;

			float x2 = (oldUnits[iter->second].x - centerX) * scale;
			float y2 = (oldUnits[iter->second].y - centerY) * scale;
			float z2 = (oldUnits[iter->second].z - centerZ) * scale;

			glVertex3f(x1, y1, z1);
			glVertex3f(x2, y2, z2);
		}

		if (childMap.find(node.n) != childMap.end()) {
			auto childNode = childMap[node.n];
			// Center and scale coordinates
			float x1 = (node.x - centerX) * scale;
			float y1 = (node.y - centerY) * scale;
			float z1 = (node.z - centerZ) * scale;

			float x2 = (childNode.x - centerX) * scale;
			float y2 = (childNode.y - centerY) * scale;
			float z2 = (childNode.z - centerZ) * scale;

			glVertex3f(x1, y1, z1);
			glVertex3f(x2, y2, z2);
		}
	}
	glEnd();

	glLineWidth(m_CreateInfo.style.lineWidth);
	glBegin(GL_LINES);
	glColor4f(m_CreateInfo.style.lineColorModified[0],
			  m_CreateInfo.style.lineColorModified[1],
			  m_CreateInfo.style.lineColorModified[2],
			  m_CreateInfo.style.lineColorModified[3]);
	for (int i = 0; i < modifiedUnits.size(); ++i) {
		auto node = modifiedUnits[i];
		if (node.parent != -1) {
			auto iter = new_n_to_index_map.find(node.parent);
			if (iter == new_n_to_index_map.end()) {
				continue;
			}

			// Center and scale coordinates
			float x1 = (node.x - centerX) * scale;
			float y1 = (node.y - centerY) * scale;
			float z1 = (node.z - centerZ) * scale;

			float x2 = (newUnits[iter->second].x - centerX) * scale;
			float y2 = (newUnits[iter->second].y - centerY) * scale;
			float z2 = (newUnits[iter->second].z - centerZ) * scale;

			glVertex3f(x1, y1, z1);
			glVertex3f(x2, y2, z2);
		}

		if (childMap.find(node.n) != childMap.end()) {
			auto childNode = childMap[node.n];
			// Center and scale coordinates
			float x1 = (node.x - centerX) * scale;
			float y1 = (node.y - centerY) * scale;
			float z1 = (node.z - centerZ) * scale;

			float x2 = (childNode.x - centerX) * scale;
			float y2 = (childNode.y - centerY) * scale;
			float z2 = (childNode.z - centerZ) * scale;

			glVertex3f(x1, y1, z1);
			glVertex3f(x2, y2, z2);
		}
	}
	glEnd();

	glLineWidth(m_CreateInfo.style.lineWidth);
	glBegin(GL_LINES);
	glColor4f(m_CreateInfo.style.lineColor[0], m_CreateInfo.style.lineColor[1],
			  m_CreateInfo.style.lineColor[2], m_CreateInfo.style.lineColor[3]);
	for (int i = 0; i < unchangedUnits.size(); ++i) {
		auto node = unchangedUnits[i];
		if (node.parent != -1) {
			auto iter = new_n_to_index_map.find(node.parent);
			if (iter == new_n_to_index_map.end()) {
				continue;
			}

			// Center and scale coordinates
			float x1 = (node.x - centerX) * scale;
			float y1 = (node.y - centerY) * scale;
			float z1 = (node.z - centerZ) * scale;

			float x2 = (newUnits[iter->second].x - centerX) * scale;
			float y2 = (newUnits[iter->second].y - centerY) * scale;
			float z2 = (newUnits[iter->second].z - centerZ) * scale;

			glVertex3f(x1, y1, z1);
			glVertex3f(x2, y2, z2);
		}
	}
	glEnd();

	renderBoundingBox(minX, minY, minZ, maxX, maxY, maxZ);
}

void SwcRenderer::initializeGL() {
	initializeOpenGLFunctions();
	glEnable(GL_DEPTH_TEST);  // Enable depth testing
	glDepthFunc(GL_LESS);	  // Depth test function
	glDepthRange(0.1, 10.0);  // Adjust the depth range for better precision
	glClearColor(backgroundColor[0], backgroundColor[1], backgroundColor[2],
				 backgroundColor[3]);

	glEnable(GL_BLEND);	 // Enable color blending for anti-aliasing
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	// Set blend function

	glEnable(GL_POINT_SMOOTH);	// Enable point smoothing
	glHint(GL_POINT_SMOOTH_HINT,
		   GL_NICEST);	// Set the quality of point smoothing

	glEnable(GL_LINE_SMOOTH);  // Enable line smoothing for better line quality
	glHint(GL_LINE_SMOOTH_HINT,
		   GL_NICEST);	// Set the quality of line smoothing

	// Enable multisampling for anti-aliasing
	glEnable(GL_MULTISAMPLE);
}

void SwcRenderer::resizeGL(int w, int h) {
	glViewport(0, 0, w, h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	float aspectRatio = static_cast<float>(w) / static_cast<float>(h);

	// Use perspective projection for better 3D visualization
	float fov = 45.0f;	// Field of view in degrees
	float near = 0.1f;
	float far = 100.0f;

	// Convert to radians and calculate perspective
	float fH = tan(fov / 360.0f * M_PI) * near;
	float fW = fH * aspectRatio;

	glFrustum(-fW, fW, -fH, fH, near, far);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Position camera for optimal viewing
	glTranslatef(0.0f, 0.0f, -3.0f);  // Move camera back
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
	} else if (event->buttons() & Qt::RightButton) {
		setXRotation(xRot + 8 * dy);
		setZRotation(zRot + 8 * dx);
	}
	lastPos = event->pos();
}

void SwcRenderer::wheelEvent(QWheelEvent* event) {
	// More precise zoom control with limits
	float zoomFactor = 1.05f;  // Smaller zoom steps for smoother experience

	if (event->angleDelta().y() > 0) {
		zoom *= zoomFactor;
		if (zoom > 5.0f) zoom = 5.0f;  // Maximum zoom limit
	} else if (event->angleDelta().y() < 0) {
		zoom /= zoomFactor;
		if (zoom < 0.1f) zoom = 0.1f;  // Minimum zoom limit
	}
	update();
}

void SwcRenderer::paintGL() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Apply camera positioning
	glTranslatef(0.0f, 0.0f, -3.0f);

	// Apply transformations in proper order for intuitive interaction
	glScalef(zoom, zoom, zoom);	 // Apply zoom first
	glRotatef(xRot / 16.0f, 1.0f, 0.0f, 0.0f);
	glRotatef(yRot / 16.0f, 0.0f, 1.0f, 0.0f);
	glRotatef(zRot / 16.0f, 0.0f, 0.0f, 1.0f);

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

void SwcRenderer::renderBoundingBox(float minX, float minY, float minZ,
									float maxX, float maxY, float maxZ) {
	// Calculate center and scale for consistent coordinate transformation
	float centerX = (minX + maxX) / 2.0f;
	float centerY = (minY + maxY) / 2.0f;
	float centerZ = (minZ + maxZ) / 2.0f;

	float rangeX = maxX - minX;
	float rangeY = maxY - minY;
	float rangeZ = maxZ - minZ;
	float maxRange = (std::max)((std::max)(rangeX, rangeY), rangeZ);

	float scale = maxRange > 0 ? 1.6f / maxRange : 1.0f;

	// Transform bounding box vertices using the same coordinate system
	float x1 = (minX - centerX) * scale;
	float y1 = (minY - centerY) * scale;
	float z1 = (minZ - centerZ) * scale;
	float x2 = (maxX - centerX) * scale;
	float y2 = (maxY - centerY) * scale;
	float z2 = (maxZ - centerZ) * scale;

	// Draw bounding box with enhanced visual
	glLineWidth(1.5f);
	glBegin(GL_LINES);
	glColor4f(m_CreateInfo.style.boundingBoxLineColor[0],
			  m_CreateInfo.style.boundingBoxLineColor[1],
			  m_CreateInfo.style.boundingBoxLineColor[2],
			  m_CreateInfo.style.boundingBoxLineColor[3]);

	// Bottom square
	glVertex3f(x1, y1, z1);
	glVertex3f(x2, y1, z1);
	glVertex3f(x2, y1, z1);
	glVertex3f(x2, y2, z1);
	glVertex3f(x2, y2, z1);
	glVertex3f(x1, y2, z1);
	glVertex3f(x1, y2, z1);
	glVertex3f(x1, y1, z1);

	// Top square
	glVertex3f(x1, y1, z2);
	glVertex3f(x2, y1, z2);
	glVertex3f(x2, y1, z2);
	glVertex3f(x2, y2, z2);
	glVertex3f(x2, y2, z2);
	glVertex3f(x1, y2, z2);
	glVertex3f(x1, y2, z2);
	glVertex3f(x1, y1, z2);

	// Vertical lines
	glVertex3f(x1, y1, z1);
	glVertex3f(x1, y1, z2);
	glVertex3f(x2, y1, z1);
	glVertex3f(x2, y1, z2);
	glVertex3f(x2, y2, z1);
	glVertex3f(x2, y2, z2);
	glVertex3f(x1, y2, z1);
	glVertex3f(x1, y2, z2);
	glEnd();
}

void SwcRenderer::compareNeuronUnits(const std::vector<NeuronUnit>& oldUnits,
									 const std::vector<NeuronUnit>& newUnits,
									 std::vector<NeuronUnit>& deleted,
									 std::vector<NeuronUnit>& added,
									 std::vector<NeuronUnit>& modified,
									 std::vector<NeuronUnit>& unchanged) {
	// Clear output vectors
	deleted.clear();
	added.clear();
	modified.clear();
	unchanged.clear();

	// Create maps for efficient lookup
	std::unordered_map<std::string, const NeuronUnit*> oldUnitMap;
	std::unordered_map<std::string, const NeuronUnit*> newUnitMap;

	// Build maps using UUID as key
	for (const auto& unit : oldUnits) {
		oldUnitMap[unit.uuid] = &unit;
	}

	for (const auto& unit : newUnits) {
		newUnitMap[unit.uuid] = &unit;
	}

	// Find deleted and unchanged/modified units
	for (const auto& oldUnit : oldUnits) {
		auto newIt = newUnitMap.find(oldUnit.uuid);
		if (newIt == newUnitMap.end()) {
			// Unit not found in new data - deleted
			deleted.push_back(oldUnit);
		} else {
			// Unit exists in both - check if modified
			const NeuronUnit& newUnit = *(newIt->second);
			if (oldUnit.x != newUnit.x || oldUnit.y != newUnit.y ||
				oldUnit.z != newUnit.z || oldUnit.radius != newUnit.radius ||
				oldUnit.type != newUnit.type ||
				oldUnit.parent != newUnit.parent) {
				// Unit has been modified
				modified.push_back(newUnit);
			} else {
				// Unit is unchanged
				unchanged.push_back(newUnit);
			}
		}
	}

	// Find added units
	for (const auto& newUnit : newUnits) {
		auto oldIt = oldUnitMap.find(newUnit.uuid);
		if (oldIt == oldUnitMap.end()) {
			// Unit not found in old data - added
			added.push_back(newUnit);
		}
	}
}

void SwcRenderer::renderMarkers(float centerX, float centerY, float centerZ,
								float scale) {
	auto& markers = m_CreateInfo.markers;
	if (markers.empty()) return;

	glPointSize(m_CreateInfo.style.markerPointSize);
	glBegin(GL_POINTS);
	for (auto& m : markers) {
		float x = (m.x - centerX) * scale;
		float y = (m.y - centerY) * scale;
		float z = (m.z - centerZ) * scale;
		glColor4f(m.color[0], m.color[1], m.color[2], m.color[3]);
		glVertex3f(x, y, z);
	}
	glEnd();
}

void SwcRenderer::setMarkerPointSize(int size) {
	m_CreateInfo.style.markerPointSize = static_cast<float>(size);
	update();
}

void SwcRenderer::resetView() {
	// Reset to optimal viewing position
	xRot = 15 * 16;	 // Initial slight downward angle
	yRot = 30 * 16;	 // Initial slight rotation for 3D perspective
	zRot = 0;
	zoom = 0.8f;  // Slightly zoomed out for better initial view
	update();
}

#include "SwcRenderer.moc"
