// Fall 2019

#include "A3.hpp"
#include "scene_lua.hpp"
using namespace std;

#include "cs488-framework/GlErrorCheck.hpp"
#include "cs488-framework/MathUtils.hpp"
#include "GeometryNode.hpp"
#include "JointNode.hpp"

#include <imgui/imgui.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

static bool show_gui = true;

const size_t CIRCLE_PTS = 48;

int current_col = 0;
int m_mouseButtonActive = 0;
bool leftPressed = false;
bool middlePressed = false;
bool rightPressed = false;
bool drawCircle = false;
float gxPos = 0;
float gyPos = 0;
float oldxPosT = 0;
float oldyPosT = 0;
float oldzPosT = 0;
float oldxPosR = 0;
float oldyPosR = 0;
float oldzPosR = 0;
float sxPos = 0;
float syPos = 0;
float exPos = 0;
float eyPos = 0;
float gx = 0;
float gy = 0;
float gz = 0;
float grx = 0;
float gry = 0;
float grz = 0;
vec3 t;
bool selected[60] = { false };
bool isSelected = false;
//----------------------------------------------------------------------------------------
// Constructor
A3::A3(const std::string & luaSceneFile)
	: m_luaSceneFile(luaSceneFile),
	  m_positionAttribLocation(0),
	  m_normalAttribLocation(0),
	  m_vao_meshData(0),
	  m_vbo_vertexPositions(0),
	  m_vbo_vertexNormals(0),
	  m_vao_arcCircle(0),
	  m_vbo_arcCircle(0)
{

}

//----------------------------------------------------------------------------------------
// Destructor
A3::~A3()
{

}

//----------------------------------------------------------------------------------------
void A3::translateNodes(SceneNode & cur, glm::vec3 t) {
	for (SceneNode * node : cur.children) {
		if (node->m_nodeType != NodeType::GeometryNode) {
			translateNodes(*node, t);
			continue;
		}
		node->translate(t);
	}
}

void A3::rotateNodes(SceneNode & cur, float eyPos, float syPos, bool isSelected) {
	for (SceneNode * node : cur.children) {
		float x, y, z;
		if (selected[node->m_nodeId] || isSelected) {
			isSelected = true;
		}
		else if (cur.m_nodeId == 2 || cur.m_nodeId == 22) {
			isSelected = false;
			gx = gy = gz = 0;
		}
		else {
			isSelected = false;
		}
		if (node->m_nodeType != NodeType::GeometryNode) {
			// non Geometry node
			rotateNodes(*node, eyPos, syPos, isSelected);
			continue;
		}

		// Geomtry node
		if (isSelected && gx != 0) {
			node->translate(vec3(-gx, -gy, 0));
			node->rotate('x', (eyPos - syPos)/4);
			node->translate(vec3(gx, gy, 0));
		}

		if (selected[node->m_nodeId-1]) {
			x = node->trans[3][0];
			y = node->trans[3][1] + node->trans[1][1];
			z = node->trans[3][2];
			node->translate(vec3(-x, -y, 0));
			node->rotate('x', (eyPos - syPos)/4);
			node->translate(vec3(x, y, 0));
			gx += x;
			gy += y;
			gz += z;
		}
	}
}

void A3::gRotateNodes(SceneNode & cur, float exPos, float sxPos, float eyPos, float syPos) {
	for (SceneNode * node : cur.children) {
		if (node->m_nodeType != NodeType::GeometryNode) {
			// non Geometry node
			gRotateNodes(*node, exPos, sxPos, eyPos, syPos);
			continue;
		}
		node->rotate('x', (eyPos - syPos)/4);
		node->rotate('y', (exPos - sxPos)/4);
	}
}

void A3::gRotateNodesZ(SceneNode & cur, float eyPos, float syPos) {
	for (SceneNode * node : cur.children) {
		if (node->m_nodeType != NodeType::GeometryNode) {
			// non Geometry node
			gRotateNodesZ(*node, eyPos, syPos);
			continue;
		}
		node->rotate('z', (eyPos - syPos)/4);
	}
}
void A3::gRotateNodesO(SceneNode & cur) {
	for (SceneNode * node : cur.children) {
		if (node->m_nodeType != NodeType::GeometryNode) {
			// non Geometry node
			gRotateNodesO(*node);
			continue;
		}
		node->rotate('x', -oldxPosR);
		node->rotate('y', -oldyPosR);
		node->rotate('z', -oldzPosR);
	}
}

/*
 * Called once, at program start.
 */
void A3::init()
{
	// Set the background colour.
	glClearColor(0.75, 0.75, 0.75, 1.0);

	createShaderProgram();

	glGenVertexArrays(1, &m_vao_arcCircle);
	glGenVertexArrays(1, &m_vao_meshData);
	enableVertexShaderInputSlots();

	processLuaSceneFile(m_luaSceneFile);

	// Load and decode all .obj files at once here.  You may add additional .obj files to
	// this list in order to support rendering additional mesh types.  All vertex
	// positions, and normals will be extracted and stored within the MeshConsolidator
	// class.
	unique_ptr<MeshConsolidator> meshConsolidator (new MeshConsolidator{
			getAssetFilePath("cube.obj"),
			getAssetFilePath("sphere.obj"),
			getAssetFilePath("suzanne.obj")
	});


	// Acquire the BatchInfoMap from the MeshConsolidator.
	meshConsolidator->getBatchInfoMap(m_batchInfoMap);

	// Take all vertex data within the MeshConsolidator and upload it to VBOs on the GPU.
	uploadVertexDataToVbos(*meshConsolidator);

	mapVboDataToVertexShaderInputLocations();

	initPerspectiveMatrix();

	initViewMatrix();

	initLightSources();


	// Exiting the current scope calls delete automatically on meshConsolidator freeing
	// all vertex data resources.  This is fine since we already copied this data to
	// VBOs on the GPU.  We have no use for storing vertex data on the CPU side beyond
	// this point.
}

//----------------------------------------------------------------------------------------
void A3::processLuaSceneFile(const std::string & filename) {
	// This version of the code treats the Lua file as an Asset,
	// so that you'd launch the program with just the filename
	// of a puppet in the Assets/ directory.
	// std::string assetFilePath = getAssetFilePath(filename.c_str());
	// m_rootNode = std::shared_ptr<SceneNode>(import_lua(assetFilePath));

	// This version of the code treats the main program argument
	// as a straightforward pathname.
	m_rootNode = std::shared_ptr<SceneNode>(import_lua(filename));
	if (!m_rootNode) {
		std::cerr << "Could Not Open " << filename << std::endl;
	}
}

//----------------------------------------------------------------------------------------
void A3::createShaderProgram()
{
	m_shader.generateProgramObject();
	m_shader.attachVertexShader( getAssetFilePath("VertexShader.vs").c_str() );
	m_shader.attachFragmentShader( getAssetFilePath("FragmentShader.fs").c_str() );
	m_shader.link();

	m_shader_arcCircle.generateProgramObject();
	m_shader_arcCircle.attachVertexShader( getAssetFilePath("arc_VertexShader.vs").c_str() );
	m_shader_arcCircle.attachFragmentShader( getAssetFilePath("arc_FragmentShader.fs").c_str() );
	m_shader_arcCircle.link();
}

//----------------------------------------------------------------------------------------
void A3::enableVertexShaderInputSlots()
{
	//-- Enable input slots for m_vao_meshData:
	{
		glBindVertexArray(m_vao_meshData);

		// Enable the vertex shader attribute location for "position" when rendering.
		m_positionAttribLocation = m_shader.getAttribLocation("position");
		glEnableVertexAttribArray(m_positionAttribLocation);

		// Enable the vertex shader attribute location for "normal" when rendering.
		m_normalAttribLocation = m_shader.getAttribLocation("normal");
		glEnableVertexAttribArray(m_normalAttribLocation);

		CHECK_GL_ERRORS;
	}


	//-- Enable input slots for m_vao_arcCircle:
	{
		glBindVertexArray(m_vao_arcCircle);

		// Enable the vertex shader attribute location for "position" when rendering.
		m_arc_positionAttribLocation = m_shader_arcCircle.getAttribLocation("position");
		glEnableVertexAttribArray(m_arc_positionAttribLocation);

		CHECK_GL_ERRORS;
	}

	// Restore defaults
	glBindVertexArray(0);
}

//----------------------------------------------------------------------------------------
void A3::uploadVertexDataToVbos (
		const MeshConsolidator & meshConsolidator
) {
	// Generate VBO to store all vertex position data
	{
		glGenBuffers(1, &m_vbo_vertexPositions);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexPositions);

		glBufferData(GL_ARRAY_BUFFER, meshConsolidator.getNumVertexPositionBytes(),
				meshConsolidator.getVertexPositionDataPtr(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}

	// Generate VBO to store all vertex normal data
	{
		glGenBuffers(1, &m_vbo_vertexNormals);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexNormals);

		glBufferData(GL_ARRAY_BUFFER, meshConsolidator.getNumVertexNormalBytes(),
				meshConsolidator.getVertexNormalDataPtr(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}

	// Generate VBO to store the trackball circle.
	{
		glGenBuffers( 1, &m_vbo_arcCircle );
		glBindBuffer( GL_ARRAY_BUFFER, m_vbo_arcCircle );

		float *pts = new float[ 2 * CIRCLE_PTS ];
		for( size_t idx = 0; idx < CIRCLE_PTS; ++idx ) {
			float ang = 2.0 * M_PI * float(idx) / CIRCLE_PTS;
			pts[2*idx] = cos( ang );
			pts[2*idx+1] = sin( ang );
		}

		glBufferData(GL_ARRAY_BUFFER, 2*CIRCLE_PTS*sizeof(float), pts, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
void A3::mapVboDataToVertexShaderInputLocations()
{
	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao_meshData);

	// Tell GL how to map data from the vertex buffer "m_vbo_vertexPositions" into the
	// "position" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexPositions);
	glVertexAttribPointer(m_positionAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	// Tell GL how to map data from the vertex buffer "m_vbo_vertexNormals" into the
	// "normal" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexNormals);
	glVertexAttribPointer(m_normalAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;

	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao_arcCircle);

	// Tell GL how to map data from the vertex buffer "m_vbo_arcCircle" into the
	// "position" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_arcCircle);
	glVertexAttribPointer(m_arc_positionAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A3::initPerspectiveMatrix()
{
	float aspect = ((float)m_windowWidth) / m_windowHeight;
	m_perpsective = glm::perspective(degreesToRadians(60.0f), aspect, 0.1f, 100.0f);
}


//----------------------------------------------------------------------------------------
void A3::initViewMatrix() {
	m_view = glm::lookAt(vec3(2.5f, 0.8f, 10.0f), vec3(2.0f, 0.0f, -2.0f),
			vec3(0.0f, 1.0f, 0.0f));
}

//----------------------------------------------------------------------------------------
void A3::initLightSources() {
	// World-space position
	m_light.position = vec3(0.0f, 5.0f, 5.0f);
	m_light.rgbIntensity = vec3(1.5f); // light
}

//----------------------------------------------------------------------------------------
void A3::uploadCommonSceneUniforms() {
	m_shader.enable();
	{
		//-- Set Perpsective matrix uniform for the scene:
		GLint location = m_shader.getUniformLocation("Perspective");
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(m_perpsective));
		CHECK_GL_ERRORS;


		//-- Set LightSource uniform for the scene:
		{
			location = m_shader.getUniformLocation("light.position");
			glUniform3fv(location, 1, value_ptr(m_light.position));
			location = m_shader.getUniformLocation("light.rgbIntensity");
			glUniform3fv(location, 1, value_ptr(m_light.rgbIntensity));
			CHECK_GL_ERRORS;
		}

		//-- Set background light ambient intensity
		{
			location = m_shader.getUniformLocation("ambientIntensity");
			vec3 ambientIntensity(0.25f);
			glUniform3fv(location, 1, value_ptr(ambientIntensity));
			CHECK_GL_ERRORS;
		}
	}
	m_shader.disable();
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A3::appLogic()
{
	// Place per frame, application logic here ...

	uploadCommonSceneUniforms();
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A3::guiLogic()
{
	if( !show_gui ) {
		return;
	}

	static bool firstRun(true);
	if (firstRun) {
		ImGui::SetNextWindowPos(ImVec2(50, 50));
		firstRun = false;
	}

	static bool showDebugWindow(true);
	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
	float opacity(0.5f);

	ImGui::Begin("Properties", &showDebugWindow, ImVec2(100,100), opacity,
			windowFlags);


		// Add more gui elements here here ...
		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("Application")) {
				if (ImGui::MenuItem("Reset Position (I)")) {
					for (SceneNode * node : m_rootNode->children) {
						t = vec3(-oldxPosT, -oldyPosT, -oldzPosT);
						if (node->m_nodeType != NodeType::GeometryNode) {
							translateNodes(*node, t);
							continue;
						}
						node->translate(t);
					}
					oldxPosT = 0;
					oldyPosT = 0;
					oldzPosT = 0;
				}
				if (ImGui::MenuItem("Reset Orientation (O)")) {
					for (SceneNode * node : m_rootNode->children) {
						if (node->m_nodeType != NodeType::GeometryNode) {
							// non Geometry node
							gRotateNodesO(*node);
							continue;
						}
						node->rotate('x', -oldxPosR);
						node->rotate('y', -oldyPosR);
						node->rotate('z', -oldzPosR);
					}
					oldxPosR = oldyPosR = oldzPosR = 0;
				}
				if (ImGui::MenuItem("Reset Joints (S)")) {}
				if (ImGui::MenuItem("Reset All (A)")) {}
				if(ImGui::MenuItem("Quit (Q)")) {
					glfwSetWindowShouldClose(m_window, GL_TRUE);
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Edit")) {
				if (ImGui::MenuItem("Undo (U)")) {}
				if (ImGui::MenuItem("Redo (R)")) {}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Options")) {
				if (ImGui::MenuItem("Circle (C)")) {
					drawCircle = !drawCircle;
				}
				if (ImGui::MenuItem("Z-buffer (Z)")) {}
				if (ImGui::MenuItem("Backface culling (B)")) {}
				if (ImGui::MenuItem("Frontface culling (F)")) {}
				ImGui::EndMenu();
			}

			// picking menu
			if (ImGui::BeginMenu("Picking")) {
				if (ImGui::MenuItem("Left upper arm")) {
					selected[5] = !selected[5];
				}
				if (ImGui::MenuItem("Left forearm")) {
					selected[8] = !selected[8];
				}
				if (ImGui::MenuItem("Left hand")) {
					selected[11] = !selected[11];
				}
				if (ImGui::MenuItem("Right upper arm")) {
					selected[14] = !selected[14];
				}
				if (ImGui::MenuItem("Right forearm")) {
					selected[17] = !selected[17];
				}
				if (ImGui::MenuItem("Right hand")) {
					selected[20] = !selected[20];
				}

				if (ImGui::MenuItem("Left thigh")) {
					selected[25] = !selected[25];
				}
				if (ImGui::MenuItem("Left calf")) {
					selected[28] = !selected[28];
				}
				if (ImGui::MenuItem("Left foot")) {
					selected[31] = !selected[31];
				}
				if (ImGui::MenuItem("Right thigh")) {
					selected[34] = !selected[34];
				}
				if (ImGui::MenuItem("Right calf")) {
					selected[37] = !selected[37];
				}
				if (ImGui::MenuItem("Right foot")) {
					selected[40] = !selected[40];
				}
				if (ImGui::MenuItem("Neck")) {
					selected[43] = !selected[43];
				}
				if (ImGui::MenuItem("Head")) {
					selected[46] = !selected[46];
				}
				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}

		ImGui::RadioButton( "Position/Orientation (P)", &current_col, 0 );
		ImGui::RadioButton( "Joints (J)", &current_col, 1 );


	ImGui::End();
}

//----------------------------------------------------------------------------------------
// Update mesh specific shader uniforms:
static void updateShaderUniforms(
		const ShaderProgram & shader,
		const GeometryNode & node,
		const glm::mat4 & viewMatrix
) {

	shader.enable();
	{
		//-- Set ModelView matrix:
		GLint location = shader.getUniformLocation("ModelView");
		mat4 modelView = viewMatrix * node.trans;
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(modelView));
		CHECK_GL_ERRORS;

		//-- Set NormMatrix:
		location = shader.getUniformLocation("NormalMatrix");
		mat3 normalMatrix = glm::transpose(glm::inverse(mat3(modelView)));
		glUniformMatrix3fv(location, 1, GL_FALSE, value_ptr(normalMatrix));
		CHECK_GL_ERRORS;


		//-- Set Material values:
		location = shader.getUniformLocation("material.kd");
		vec3 kd = node.material.kd;
		if (selected[node.m_nodeId-1]) {
			glm::vec3 col = glm::vec3( 1.0, 1.0, 0.0 );
			glUniform3fv(location, 1, value_ptr(col));
		} else {
			glUniform3fv(location, 1, value_ptr(kd));
		}
		CHECK_GL_ERRORS;
	}
	shader.disable();

}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A3::draw() {

	glEnable( GL_DEPTH_TEST );
	renderSceneGraph(*m_rootNode);


	glDisable( GL_DEPTH_TEST );
	if (drawCircle) {
		renderArcCircle();
	}
}

//----------------------------------------------------------------------------------------
void A3::drawNodes(const SceneNode & cur) {

	for (const SceneNode * node : cur.children) {
		if (node->m_nodeType != NodeType::GeometryNode) {
			drawNodes(*node);
			continue;
		}

		const GeometryNode * geometryNode = static_cast<const GeometryNode *>(node);
		// cout << node << endl;

		updateShaderUniforms(m_shader, *geometryNode, m_view);

		// Get the BatchInfo corresponding to the GeometryNode's unique MeshId.
		BatchInfo batchInfo = m_batchInfoMap[geometryNode->meshId];

		//-- Now render the mesh:
		m_shader.enable();
		glDrawArrays(GL_TRIANGLES, batchInfo.startIndex, batchInfo.numIndices);
		m_shader.disable();
	}
}

void A3::renderSceneGraph(const SceneNode & root) {

	// Bind the VAO once here, and reuse for all GeometryNode rendering below.
	glBindVertexArray(m_vao_meshData);

	// This is emphatically *not* how you should be drawing the scene graph in
	// your final implementation.  This is a non-hierarchical demonstration
	// in which we assume that there is a list of GeometryNodes living directly
	// underneath the root node, and that we can draw them in a loop.  It's
	// just enough to demonstrate how to get geometry and materials out of
	// a GeometryNode and onto the screen.

	// You'll want to turn this into recursive code that walks over the tree.
	// You can do that by putting a method in SceneNode, overridden in its
	// subclasses, that renders the subtree rooted at every node.  Or you
	// could put a set of mutually recursive functions in this class, which
	// walk down the tree from nodes of different types.

	for (const SceneNode * node : root.children) {

		if (node->m_nodeType != NodeType::GeometryNode) {
			drawNodes(*node);
			continue;
		}

		const GeometryNode * geometryNode = static_cast<const GeometryNode *>(node);

		updateShaderUniforms(m_shader, *geometryNode, m_view);

		// Get the BatchInfo corresponding to the GeometryNode's unique MeshId.
		BatchInfo batchInfo = m_batchInfoMap[geometryNode->meshId];

		//-- Now render the mesh:
		m_shader.enable();
		glDrawArrays(GL_TRIANGLES, batchInfo.startIndex, batchInfo.numIndices);
		m_shader.disable();
	}

	glBindVertexArray(0);
	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
// Draw the trackball circle.
void A3::renderArcCircle() {
	glBindVertexArray(m_vao_arcCircle);

	m_shader_arcCircle.enable();
		GLint m_location = m_shader_arcCircle.getUniformLocation( "M" );
		float aspect = float(m_framebufferWidth)/float(m_framebufferHeight);
		glm::mat4 M;
		if( aspect > 1.0 ) {
			M = glm::scale( glm::mat4(), glm::vec3( 0.5/aspect, 0.5, 1.0 ) );
		} else {
			M = glm::scale( glm::mat4(), glm::vec3( 0.5, 0.5*aspect, 1.0 ) );
		}
		glUniformMatrix4fv( m_location, 1, GL_FALSE, value_ptr( M ) );
		glDrawArrays( GL_LINE_LOOP, 0, CIRCLE_PTS );
	m_shader_arcCircle.disable();

	glBindVertexArray(0);
	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A3::cleanup()
{

}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A3::cursorEnterWindowEvent (
		int entered
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------


/*
 * Event handler.  Handles mouse cursor movement events.
 */
bool A3::mouseMoveEvent (double xPos, double yPos) {
	bool eventHandled(false);

	// Fill in with event handling code...
	gxPos = exPos = xPos;
	gyPos = eyPos = yPos;
	if (current_col == 0) {
		if (m_mouseButtonActive = 1) {
			if (leftPressed) {
				for (SceneNode * node : m_rootNode->children) {
					t = vec3((exPos - sxPos)/64, -(eyPos - syPos)/64, 0);
					if (node->m_nodeType != NodeType::GeometryNode) {
						translateNodes(*node, t);
						continue;
					}
					node->translate(t);
					oldxPosT += (exPos - sxPos)/64;
					oldyPosT -= (eyPos - syPos)/64;
				}

				sxPos = exPos;
				syPos = eyPos;
			}
			if (middlePressed) {
				for (SceneNode * node : m_rootNode->children) {
					t = vec3(0, 0, (eyPos - syPos)/32);
					if (node->m_nodeType != NodeType::GeometryNode) {
						translateNodes(*node, t);
						continue;
					}
					node->translate(t);
					oldzPosT += (eyPos - syPos)/32;
				}
				syPos = eyPos;
			}
			if (rightPressed) {
				// cout << m_framebufferWidth << " " << m_framebufferHeight << endl;
				// cout << xPos << " " << yPos << endl;
				if (xPos > 320 && xPos < 702 && yPos > 191 && yPos < 576 && drawCircle) {
					for (SceneNode * node : m_rootNode->children) {
						if (node->m_nodeType != NodeType::GeometryNode) {
							// non Geometry node
							gRotateNodes(*node, exPos, sxPos, eyPos, syPos);
							continue;
						}
						node->rotate('x', (eyPos - syPos)/4);
						node->rotate('y', (exPos - sxPos)/4);
					}
					oldxPosR += (eyPos - syPos)/4;
					oldyPosR += (exPos - sxPos)/4;
				}
				else if ((xPos < 320 || xPos > 702 || yPos < 191 || yPos > 576) && drawCircle) {
					for (SceneNode * node : m_rootNode->children) {
						if (node->m_nodeType != NodeType::GeometryNode) {
							// non Geometry node
							gRotateNodesZ(*node, eyPos, syPos);
							continue;
						}
						node->rotate('z', (eyPos - syPos)/4);
					}
					oldzPosR += (eyPos - syPos)/4;
				}
				syPos = eyPos;
				sxPos = exPos;
			}
		}
	}
	else if (current_col == 1) {
		if (m_mouseButtonActive = 1) {
			if (middlePressed) {
				gx = gy = gz = 0;
				for (SceneNode * node : m_rootNode->children) {
					if (node->m_nodeType != NodeType::GeometryNode) {
						rotateNodes(*node, eyPos, syPos, false);
						continue;
					}
				}
				sxPos = exPos;
				syPos = eyPos;
			}
		}
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A3::mouseButtonInputEvent (int button, int actions, int mods) {
	bool eventHandled(false);

	// Fill in with event handling code...
	if (actions == GLFW_PRESS) {
		m_mouseButtonActive = 1;
		sxPos = gxPos;
		syPos = gyPos;
		if (!ImGui::IsMouseHoveringAnyWindow()) {
			if (button == GLFW_MOUSE_BUTTON_LEFT) {
				leftPressed = true;
			}
			if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
				middlePressed = true;
			}
			if (button == GLFW_MOUSE_BUTTON_RIGHT) {
				rightPressed = true;
			}
		}
	}
	if (actions == GLFW_RELEASE) {
		m_mouseButtonActive = 0;
		if (leftPressed) {
			if (current_col == 0) {
				oldxPosT = (exPos - sxPos)/64 + oldxPosT;
				oldyPosT = (eyPos - syPos)/64 + oldyPosT;
			}
			sxPos = 0;
			syPos = 0;
			leftPressed = false;
		}
		if (middlePressed) {
			if (current_col == 1) {
				oldzPosT = (eyPos - syPos)/64 + oldzPosT;
			}
			middlePressed = false;
		}
		if (rightPressed) {
			if (current_col == 0) {

			}
			rightPressed = false;
		}
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A3::mouseScrollEvent (
		double xOffSet,
		double yOffSet
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles window resize events.
 */
bool A3::windowResizeEvent (
		int width,
		int height
) {
	bool eventHandled(false);
	initPerspectiveMatrix();
	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A3::keyInputEvent (
		int key,
		int action,
		int mods
) {
	bool eventHandled(false);

	if( action == GLFW_PRESS ) {
		if( key == GLFW_KEY_M ) {
			show_gui = !show_gui;
			eventHandled = true;
		}
		if ( key == GLFW_KEY_I ) {
			for (SceneNode * node : m_rootNode->children) {
				t = vec3(-oldxPosT, -oldyPosT, -oldzPosT);
				if (node->m_nodeType != NodeType::GeometryNode) {
					translateNodes(*node, t);
					continue;
				}
				node->translate(t);
			}
			oldxPosT = 0;
			oldyPosT = 0;
			oldzPosT = 0;
		}
		if (key ==GLFW_KEY_O) {
			for (SceneNode * node : m_rootNode->children) {
				if (node->m_nodeType != NodeType::GeometryNode) {
					// non Geometry node
					gRotateNodesO(*node);
					continue;
				}
				node->rotate('x', -oldxPosR);
				node->rotate('y', -oldyPosR);
				node->rotate('z', -oldzPosR);
			}
			oldxPosR = oldyPosR = oldzPosR = 0;
		}
		if (key == GLFW_KEY_P) {
			current_col = 0;
			eventHandled = true;
		}
		if (key == GLFW_KEY_J) {
			current_col = 1;
			eventHandled = true;
		}
		if (key == GLFW_KEY_Q) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
			eventHandled = true;
		}
	}
	// Fill in with event handling code...

	return eventHandled;
}
