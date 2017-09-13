#include "raytracing.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>

#include "fay.h"
#include "gl/buffer.h"
#include "gl/texture.h"
#include "gl/model.h"
#include "gl/camera.h"
#include "gl/shader.h"

#include "tmp/Obj.h"

using namespace std;
using namespace fay;

// environment

bool create_window();
void destroy_window();
void update();

// callback
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void process_keyboard(GLFWwindow *window);

// windows
GLFWwindow* window {};
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0, 20, 0));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

////////////////////////////////////////////////////////////////////////////////////

//Objloader instance
vector<tmp::Mesh*> meshes;					//all meshes 
vector<tmp::Material*> materials;			//all materials 
vector<unsigned short> indices;			//all mesh indices 
vector<tmp::Vertex> vertices;				//all mesh vertices  
vector<GLuint> textures;				//all textures

// 鼠标移动设置与渲染设置
int move_state = 0;
int render_state = 1;

const std::string mesh_filename = "resources/objects/blocks/blocks.obj";

//background color
glm::vec4 bg = glm::vec4(0.5, 0.5, 1, 1);

//scene axially aligned bounding box
tmp::BBox aabb;

//light crosshair gizmo vetex array and buffer object IDs
GLuint lightVAOID;
GLuint lightVerticesVBO;
glm::vec3 lightPosition = glm::vec3(0, 2, 0); //objectspace light position

//spherical cooridate variables for light rotation
float theta = 0.66f;
float phi = -1.0f;
float ligth_radius = 70;

//FPS related variables
int total_frames = 0;
float fps = 0;
float lastTime = 0;

/////////////////////////////////////////////////////////////////////////////////////

void update_lightPos()
{
	lightPosition.x = ligth_radius * cos(theta)*sin(phi);
	lightPosition.y = ligth_radius * cos(phi);
	lightPosition.z = ligth_radius * sin(theta)*sin(phi);
}

void ray_cast()
{
	create_window();

	//setup fullscreen quad geometry
	std::vector<float> quadVerts{ -1, -1, 0, 1, -1, 0, 1, 1, 0, -1, 1, 0};
	std::vector<uint32_t> quadIndices{ 0,1,2,0,2,3 };
	Buffer quad(quadVerts, quadIndices);
	
	//get the mesh path for loading of textures	
	std::string mesh_path = mesh_filename.substr(0, mesh_filename.find_last_of("/") + 1);

	//load the obj model
	vector<unsigned short> indices2;
	vector<glm::vec3> vertices2;
	if (!tmp::Load(mesh_filename.c_str(), meshes, vertices, indices, materials, aabb, vertices2, indices2)) {
		LOG(ERROR) << "Cannot load the 3ds mesh";
		exit(EXIT_FAILURE);
	}

	gl_check_errors();

	// 将所有纹理存储在一个纹理数组中
	vector<string> material_names;
	for (size_t k = 0; k<materials.size(); k++) 
		if (materials[k]->map_Kd != "")
			material_names.push_back(materials[k]->map_Kd);

	TextureArray material_arrays(material_names, mesh_path);
	
	// 加载着色器
	Shader flatShader("raytracing/flat.vert", "raytracing/flat.frag");

	Shader rasterShader("raytracing/shader.vert", "raytracing/shader.frag");

	Shader raytraceShader("raytracing/raycast.vert", "raytracing/raycast.frag");
	raytraceShader.enable();
	raytraceShader.set_float("VERTEX_TEXTURE_SIZE", (float)vertices2.size());
	raytraceShader.set_float("TRIANGLE_TEXTURE_SIZE", (float)indices2.size() / 4);
	raytraceShader.set_vec3("aabb.min", aabb.min);
	raytraceShader.set_vec3("aabb.max", aabb.max);
	raytraceShader.set_vec4("backgroundColor", bg);
	raytraceShader.disable();
	
	// 加载用于光栅化的模型
	Model model("resources/objects/blocks/blocks.obj");

	// 加载光之十字
	glm::vec3 crossHairVertices[6];
	crossHairVertices[0] = glm::vec3(-0.5f, 0, 0);
	crossHairVertices[1] = glm::vec3(0.5f,  0, 0);
	crossHairVertices[2] = glm::vec3(0, -0.5f, 0);
	crossHairVertices[3] = glm::vec3(0,  0.5f, 0);
	crossHairVertices[4] = glm::vec3(0, 0, -0.5f);
	crossHairVertices[5] = glm::vec3(0, 0,  0.5f);

	//setup light gizmo vertex array and vertex buffer object IDs
	glGenVertexArrays(1, &lightVAOID);
	glGenBuffers(1, &lightVerticesVBO);
	glBindVertexArray(lightVAOID);

	glBindBuffer(GL_ARRAY_BUFFER, lightVerticesVBO);
	//pass crosshair vertices to the buffer object
	glBufferData(GL_ARRAY_BUFFER, sizeof(crossHairVertices), &(crossHairVertices[0].x), GL_STATIC_DRAW);
	gl_check_errors();
		//enable vertex attribute array for vertex position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	gl_check_errors();

	// 将位置、索引打包成一维纹理传入片元着色器
	vector<GLfloat> VerData(vertices2.size() * 4);
	int count = 0;
	for (size_t i = 0; i<vertices2.size(); i++) {
		VerData[count++] = vertices2[i].x;
		VerData[count++] = vertices2[i].y;
		VerData[count++] = vertices2[i].z;
		VerData[count++] = 0;
	}
	TextureData<GLfloat> texVertices(GL_RGBA32F, vertices2.size(), GL_RGBA, GL_FLOAT, VerData);

	vector<GLushort> indData(indices2.size());
	count = 0;
	for (size_t i = 0; i<indices2.size(); i += 4) {
		indData[count++] = (indices2[i]);
		indData[count++] = (indices2[i + 1]);
		indData[count++] = (indices2[i + 2]);
		indData[count++] = (indices2[i + 3]);
	}
	TextureData<GLushort> texTriangles(GL_RGBA16I, indices2.size() / 4, GL_RGBA_INTEGER, GL_UNSIGNED_SHORT, indData);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glClearColor(bg.r, bg.g, bg.b, 1.0f);

	while (!glfwWindowShouldClose(window))
	{
		update();
		update_lightPos();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 modelMat(1.f);
		glm::mat4 MV = view * modelMat;
		glm::mat4 MVP = projection * MV;
		glm::mat4 invMVP = glm::inverse(MVP);
		//glm::vec3 eyePos = glm::vec3(invMV[3][0], invMV[3][1], invMV[3][2]);
		/*
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
		glm::mat4 view = camera.GetViewMatrix();

		glm::mat4 model(1.f);
		//model = glm::translate(model, glm::vec3(0.0f, -2.f, 10.0f));
		glm::mat4 MV = view * model;
		glm::mat4 MVP = projection * MV;
		glm::mat4 invMVP = glm::inverse(MVP);
		//glm::mat4 mvp = model * view;
		//mvp = mvp * projection;
		//glm::mat4 invMVP = glm::inverse(mvp);
		//set the camera transformation
		*/
		if (render_state == 1)
		{
			rasterShader.enable();

			rasterShader.set_mat4("MV", MV);
			rasterShader.set_mat3("N", glm::inverseTranspose(glm::mat3(MV)));
			rasterShader.set_mat4("P", projection);
			rasterShader.set_vec3("light_position", lightPosition);

			model.draw(rasterShader);
		}
		else if (render_state == 2) 
		{
			raytraceShader.enable();

			raytraceShader.bind_texture("textureMaps", 0, material_arrays.id());
			raytraceShader.bind_texture("vertex_positions", 1, texVertices.id());
			raytraceShader.bind_texture("triangles_list", 2, texTriangles.id());

			raytraceShader.set_vec3("eyePos", camera.Position);
			raytraceShader.set_mat4("invMVP", invMVP);
			raytraceShader.set_vec3("light_position", lightPosition);

			quad.draw();	// 渲染整个视口
		}

		// 渲染小十字
		//disable depth testing
		glDisable(GL_DEPTH_TEST);

		//draw the light gizmo, set the light vertexx array object
		glBindVertexArray(lightVAOID); {
			glm::mat4 MV = glm::mat4(1);
			//set the modelling transform for the light crosshair gizmo
			glm::mat4 T = glm::translate(glm::mat4(1), lightPosition);
			//bind the shader
			flatShader.enable();
			//set shader uniforms and draw lines
			//P*MV*T
			flatShader.set_mat4("MVP", MV*T);
			glDrawArrays(GL_LINES, 0, 6);

			glPointSize(1000.0);
			glDrawArrays(GL_POINTS, 0, 6);

			//unbind the shader
			flatShader.disable();
		}
		//enable depth test
		glEnable(GL_DEPTH_TEST);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	destroy_window();
}

// environment 

bool create_window()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

	// glfw window creation
	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		LOG(ERROR) << "Failed to create GLFW window";
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		LOG(ERROR) << "Failed to initialize GLAD";
		return false;
	}
	return true;
}

void destroy_window()
{
	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
}

void update()
{
	// per-frame time logic
	// --------------------
	float currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	deltaTime *= 10;
	lastFrame = currentFrame;

	// input
	// -----
	process_keyboard(window);
}

void process_keyboard(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);

	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)	// raster
		render_state = 1;
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)	// raycast
		render_state = 2;
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)	// pathtracing
		render_state = 3;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
		move_state = 1;	// light
	else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		move_state = 2;	// scene
	else
		move_state = 0;	// camera
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	// reversed since y-coordinates go from bottom to top
	// but z_xais form out to in
	float yoffset = lastY - ypos;

	if(move_state == 0) 
		camera.ProcessMouseMovement(xoffset, yoffset);
	else if (move_state == 1)
	{
		// 在球上运动
		theta += xoffset / 60.0f;
		phi   += yoffset / 60.0f;
	}

	lastX = xpos;
	lastY = ypos;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (move_state == 0)
		camera.ProcessMouseScroll(yoffset);
	else if (move_state == 1)
		ligth_radius -= 10 * yoffset;
}