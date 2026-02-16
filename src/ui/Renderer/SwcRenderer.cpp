#include "SwcRenderer.h"

#include <algorithm>
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
			for (int i = 0; i < m_NeuronUnits.size(); ++i) {
				n_to_index_map[m_NeuronUnits[i].n] = i;
			}
			break;
		}
		case SwcRendererMode::eVisualizeDiffSwc: {
			computeEdgeDiff(m_NeuronUnits, m_NewNeuronUnits);
			break;
		}
		case SwcRendererMode::eVisualizeUser:
			break;
		case SwcRendererMode::eVisualizeTime:
			break;
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
	float centerX = (m_DiffMinX + m_DiffMaxX) / 2.0f;
	float centerY = (m_DiffMinY + m_DiffMaxY) / 2.0f;
	float centerZ = (m_DiffMinZ + m_DiffMaxZ) / 2.0f;

	float rangeX = m_DiffMaxX - m_DiffMinX;
	float rangeY = m_DiffMaxY - m_DiffMinY;
	float rangeZ = m_DiffMaxZ - m_DiffMinZ;
	float maxRange = (std::max)((std::max)(rangeX, rangeY), rangeZ);
	float scale = maxRange > 0 ? 1.6f / maxRange : 1.0f;

	auto renderEdges = [&](const std::vector<DiffEdge>& edges,
						   const float color[4], float lineWidth) {
		glLineWidth(lineWidth);
		glBegin(GL_LINES);
		glColor4f(color[0], color[1], color[2], color[3]);
		for (auto& e : edges) {
			glVertex3f((e.x1 - centerX) * scale, (e.y1 - centerY) * scale,
					   (e.z1 - centerZ) * scale);
			glVertex3f((e.x2 - centerX) * scale, (e.y2 - centerY) * scale,
					   (e.z2 - centerZ) * scale);
		}
		glEnd();
	};

	float baseWidth = m_CreateInfo.style.lineWidth;

	// Render order: unchanged (dim) -> modified -> deleted -> added (bright)
	renderEdges(m_UnchangedEdges, m_CreateInfo.style.lineColorUnchanged,
				baseWidth);
	renderEdges(m_ModifiedEdges, m_CreateInfo.style.lineColorModified,
				baseWidth * 1.5f);
	renderEdges(m_DeletedEdges, m_CreateInfo.style.lineColorDelete,
				baseWidth * 1.5f);
	renderEdges(m_AddedEdges, m_CreateInfo.style.lineColorAdd,
				baseWidth * 1.5f);

	renderBoundingBox(m_DiffMinX, m_DiffMinY, m_DiffMinZ, m_DiffMaxX,
					  m_DiffMaxY, m_DiffMaxZ);
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

void SwcRenderer::computeEdgeDiff(const std::vector<NeuronUnit>& oldUnits,
								  const std::vector<NeuronUnit>& newUnits) {
	m_DeletedEdges.clear();
	m_AddedEdges.clear();
	m_ModifiedEdges.clear();
	m_UnchangedEdges.clear();

	// --- Step 1: Build all edges from both trees -----------------------
	struct EdgeAttrib {
		DiffEdge geo;
		int type;
		float radius;
	};

	auto buildEdges =
		[](const std::vector<NeuronUnit>& units) -> std::vector<EdgeAttrib> {
		std::unordered_map<int, const NeuronUnit*> nMap;
		for (auto& u : units) nMap[u.n] = &u;

		std::vector<EdgeAttrib> edges;
		for (auto& u : units) {
			if (u.parent == -1) continue;
			auto pit = nMap.find(u.parent);
			if (pit == nMap.end()) continue;
			auto* p = pit->second;
			edges.push_back(
				{{u.x, u.y, u.z, p->x, p->y, p->z}, u.type, u.radius});
		}
		return edges;
	};

	auto oldEdges = buildEdges(oldUnits);
	auto newEdges = buildEdges(newUnits);

	// --- Step 2: Compute adaptive threshold ----------------------------
	auto edgeLen = [](const DiffEdge& e) {
		float dx = e.x1 - e.x2, dy = e.y1 - e.y2, dz = e.z1 - e.z2;
		return std::sqrt(dx * dx + dy * dy + dz * dz);
	};

	float totalLen = 0;
	for (auto& e : oldEdges) totalLen += edgeLen(e.geo);
	for (auto& e : newEdges) totalLen += edgeLen(e.geo);
	auto totalCount =
		static_cast<int>(oldEdges.size() + newEdges.size());
	float avgLen = totalCount > 0 ? totalLen / totalCount : 1.0f;
	float threshold = avgLen * 3.0f;

	// --- Step 3: Spatial grid on old edges for fast lookup --------------
	// Distance between two edges: min of (avg endpoint dist) over both
	// orientations. Handles coordinate drift gracefully.
	auto edgeDist = [](const DiffEdge& a, const DiffEdge& b) -> float {
		auto dist = [](float ax, float ay, float az, float bx, float by,
					   float bz) {
			float dx = ax - bx, dy = ay - by, dz = az - bz;
			return std::sqrt(dx * dx + dy * dy + dz * dz);
		};
		float fwd = (dist(a.x1, a.y1, a.z1, b.x1, b.y1, b.z1) +
					 dist(a.x2, a.y2, a.z2, b.x2, b.y2, b.z2)) /
					2.0f;
		float rev = (dist(a.x1, a.y1, a.z1, b.x2, b.y2, b.z2) +
					 dist(a.x2, a.y2, a.z2, b.x1, b.y1, b.z1)) /
					2.0f;
		return (std::min)(fwd, rev);
	};

	struct GridKey {
		int x, y, z;
		bool operator==(const GridKey& o) const {
			return x == o.x && y == o.y && z == o.z;
		}
	};
	struct GridKeyHash {
		size_t operator()(const GridKey& k) const {
			size_t h = 17;
			h = h * 31 + std::hash<int>()(k.x);
			h = h * 31 + std::hash<int>()(k.y);
			h = h * 31 + std::hash<int>()(k.z);
			return h;
		}
	};

	float cellSize = threshold > 0 ? threshold : 1.0f;

	// Helper: build a spatial grid from a set of edges
	using Grid =
		std::unordered_map<GridKey, std::vector<size_t>, GridKeyHash>;

	auto buildGrid = [&](const std::vector<EdgeAttrib>& edges) -> Grid {
		Grid g;
		for (size_t i = 0; i < edges.size(); ++i) {
			auto& e = edges[i].geo;
			float mx = (e.x1 + e.x2) * 0.5f;
			float my = (e.y1 + e.y2) * 0.5f;
			float mz = (e.z1 + e.z2) * 0.5f;
			g[{static_cast<int>(std::floor(mx / cellSize)),
			   static_cast<int>(std::floor(my / cellSize)),
			   static_cast<int>(std::floor(mz / cellSize))}]
				.push_back(i);
		}
		return g;
	};

	// Helper: find closest edge in target set (no 1-to-1 constraint).
	// Multiple source edges may match the same target edge — this is
	// intentional so that resampling (3 edges → 4 edges in the same
	// spatial region) does not produce false positives.
	auto findClosest = [&](const DiffEdge& edge,
						   const std::vector<EdgeAttrib>& targetEdges,
						   const Grid& targetGrid) -> int {
		float mx = (edge.x1 + edge.x2) * 0.5f;
		float my = (edge.y1 + edge.y2) * 0.5f;
		float mz = (edge.z1 + edge.z2) * 0.5f;
		int gx = static_cast<int>(std::floor(mx / cellSize));
		int gy = static_cast<int>(std::floor(my / cellSize));
		int gz = static_cast<int>(std::floor(mz / cellSize));

		float bestDist = threshold;
		int bestIdx = -1;
		for (int dx = -1; dx <= 1; ++dx)
			for (int dy = -1; dy <= 1; ++dy)
				for (int dz = -1; dz <= 1; ++dz) {
					auto it = targetGrid.find(
						{gx + dx, gy + dy, gz + dz});
					if (it == targetGrid.end()) continue;
					for (size_t ti : it->second) {
						float d =
							edgeDist(edge, targetEdges[ti].geo);
						if (d < bestDist) {
							bestDist = d;
							bestIdx = static_cast<int>(ti);
						}
					}
				}
		return bestIdx;
	};

	Grid oldGrid = buildGrid(oldEdges);
	Grid newGrid = buildGrid(newEdges);

	// --- Step 4: Classify new edges ------------------------------------
	// Each new edge independently searches for the closest old edge.
	constexpr float kRadiusEpsilon = 1e-4f;

	for (size_t i = 0; i < newEdges.size(); ++i) {
		int closest = findClosest(newEdges[i].geo, oldEdges, oldGrid);
		if (closest < 0) {
			m_AddedEdges.push_back(newEdges[i].geo);
		} else {
			auto& oe = oldEdges[closest];
			auto& ne = newEdges[i];
			if (ne.type != oe.type ||
				std::fabs(ne.radius - oe.radius) > kRadiusEpsilon) {
				m_ModifiedEdges.push_back(ne.geo);
			} else {
				m_UnchangedEdges.push_back(ne.geo);
			}
		}
	}

	// --- Step 5: Find deleted old edges --------------------------------
	// Each old edge independently searches for the closest new edge.
	for (size_t j = 0; j < oldEdges.size(); ++j) {
		int closest = findClosest(oldEdges[j].geo, newEdges, newGrid);
		if (closest < 0) {
			m_DeletedEdges.push_back(oldEdges[j].geo);
		}
	}

	// --- Step 7: Bounding box ------------------------------------------
	m_DiffMinX = m_DiffMinY = m_DiffMinZ = std::numeric_limits<float>::max();
	m_DiffMaxX = m_DiffMaxY = m_DiffMaxZ =
		std::numeric_limits<float>::lowest();

	auto updateBoundsFromEdges = [this](const std::vector<DiffEdge>& edges) {
		for (auto& e : edges) {
			m_DiffMinX = (std::min)(m_DiffMinX, (std::min)(e.x1, e.x2));
			m_DiffMinY = (std::min)(m_DiffMinY, (std::min)(e.y1, e.y2));
			m_DiffMinZ = (std::min)(m_DiffMinZ, (std::min)(e.z1, e.z2));
			m_DiffMaxX = (std::max)(m_DiffMaxX, (std::max)(e.x1, e.x2));
			m_DiffMaxY = (std::max)(m_DiffMaxY, (std::max)(e.y1, e.y2));
			m_DiffMaxZ = (std::max)(m_DiffMaxZ, (std::max)(e.z1, e.z2));
		}
	};

	updateBoundsFromEdges(m_DeletedEdges);
	updateBoundsFromEdges(m_AddedEdges);
	updateBoundsFromEdges(m_ModifiedEdges);
	updateBoundsFromEdges(m_UnchangedEdges);

	if (m_DiffMinX > m_DiffMaxX) {
		m_DiffMinX = m_DiffMinY = m_DiffMinZ = 0.0f;
		m_DiffMaxX = m_DiffMaxY = m_DiffMaxZ = 1.0f;
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
