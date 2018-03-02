#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>

#include "fay/utility/fay.h"
#include "fay/gl/buffer.h"
#include "fay/gl/texture.h"
#include "fay/gl/model.h"
#include "fay/gl/camera.h"
#include "fay/gl/shader.h"
#include "fay/gui/gui.h"

using namespace std;

namespace fay
{

// 分辨率
const unsigned int WIDTH = 1080;
const unsigned int HEIGHT = 720;

// camera
Camera camera(glm::vec3(-3, 3, 10));
float lastX = WIDTH / 2.0f;
float lastY = HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float currentFrame = 0.f;
float deltaTime = 0.f;
float lastFrame = 0.f;

////////////////////////////////////////////////////////////////////////////////////

// 鼠标移动设置与渲染设置
bool move_light = false;
int render_state = 1;

const string blocks = "resources/objects/blocks/blocks.obj";
const string Rei = "resources/objects/Rei/Rei.obj";
const string CornellBox = "resources/objects/CornellBox/CornellBox.obj";
const string rock = "resources/objects/rock/rock.obj";
const string fairy = "resources/objects/fairy/fairy.obj";
const string mesh_filename = blocks;

//background color
glm::vec4 bg = glm::vec4(0.5, 0.5, 1, 1);

//scene axially aligned bounding box
struct BBox { glm::vec3 min, max; } aabb{ { -1000, -1000, -1000 },{ 1000, 1000, 1000 } };

//light crosshair gizmo vetex array and buffer object IDs
GLuint lightVAOID;
GLuint lightVerticesVBO;
glm::vec3 lightPosition = glm::vec3(0, 2, 0); //objectspace light position

											  //spherical cooridate variables for light rotation
float theta = 0.66f;
float phi = -1.0f;
float ligth_radius = 70;

// GUI
static ImVec4 clear_color = ImColor(127, 127, 255);
static int samples_PerPixel = 1;

/////////////////////////////////////////////////////////////////////////////////////

void update()
{
	lightPosition.x = ligth_radius * cos(theta)*sin(phi);
	lightPosition.y = ligth_radius * cos(phi);
	lightPosition.z = ligth_radius * sin(theta)*sin(phi);

	currentFrame = glfwGetTime();
	deltaTime = (currentFrame - lastFrame) * 10;
	lastFrame = currentFrame;

	ImGuiIO& io = gui_get_io();

	if (io.KeysDown[GLFW_KEY_W]) camera.ProcessKeyboard(FORWARD, deltaTime);
	if (io.KeysDown[GLFW_KEY_S]) camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (io.KeysDown[GLFW_KEY_A]) camera.ProcessKeyboard(LEFT, deltaTime);
	if (io.KeysDown[GLFW_KEY_D]) camera.ProcessKeyboard(RIGHT, deltaTime);

	if (io.KeysDown[GLFW_KEY_1]) render_state = 1;	// raster
	if (io.KeysDown[GLFW_KEY_2]) render_state = 2;	// raycast
	if (io.KeysDown[GLFW_KEY_3]) render_state = 3;	// pathtracing

	if (io.KeysDown[GLFW_KEY_SPACE] == GLFW_PRESS) move_light ^= 1;	// pathtracing

																	// 鼠标移动
	float xpos = io.MousePos.x, ypos = io.MousePos.y;
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	// reversed since y-coordinates go from bottom to top but z_xais form out to in
	float yoffset = lastY - ypos;
	lastX = xpos; lastY = ypos;

	if (move_light)
	{
		theta += xoffset / 60.0f;
		phi += yoffset / 60.0f;
		ligth_radius -= 10 * io.MouseWheel;
	}
	else
	{
		camera.ProcessMouseMovement(xoffset, yoffset);
		//camera.ProcessMouseScroll(io.MouseWheel); 禁止放缩
	}
}

int raytracing()
{
	gui_create_window(WIDTH, HEIGHT);

	// 加载覆盖整个视口的正方形
	std::vector<Vertex1> quadVerts{ {-1, -1, 0}, {1, -1, 0}, {1, 1, 0}, {-1, 1, 0} };
	std::vector<uint32_t> quadIndices{ 0,1,2,0,2,3 };
	Buffer quad(quadVerts, quadIndices);

	// 加载着色器
	Shader texShader("shader/fay_gui.vs", "shader/fay_gui.fs");

	Texture2D texture0("resources/face.png");

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	while (!gui_close_window())
	{
		gui_updateIO();

		update();

		bg = glm::vec4(clear_color.x, clear_color.y, clear_color.z, 1.f);
		glClearColor(bg.r, bg.g, bg.b, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::vec3 position = { 1.1f, 1.1f, 0.1f }, center = { 0, 0, 0 }, up = { -1, 1, -1 };
		glm::mat4 view = glm::lookAt(position, center, up);
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), 
			(float)WIDTH / (float)HEIGHT, 0.1f, 10000.0f);

		glm::mat4 MVP = projection * view;

		texShader.enable();
		texShader.set_mat4("MVP", MVP);
		texShader.bind_texture("texture0", 0, texture0.id());
		quad.draw();

		// GUI
		// Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
		{
			ImGui::Text("Info");
			ImGui::SliderInt("samples_Perpixel", &samples_PerPixel, 1, 16);
			ImGui::ColorEdit3("clear color", (float*)&clear_color);
			ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		}

		gui_drawGUI();
	}

	gui_delete_window();
	return 0;
}

}	// namespace fay

int main(int argc, char** argv)
{
	google::InitGoogleLogging(argv[0]);
	//--stderrthreshold=0 --logtostderr=true
	//FLAGS_logtostderr = true;
	//FLAGS_stderrthreshold = 0;
	//FLAGS_v = 2;

	fay::raytracing();

	return 0;
}
