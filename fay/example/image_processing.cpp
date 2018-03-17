#include "fay/utility/fay.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>

#include "fay/gl/buffer.h"
#include "fay/gl/framebuffer.h"
#include "fay/gl/texture.h"
#include "fay/gl/mesh.h"
#include "fay/gl/model.h"
#include "fay/gl/camera.h"
#include "fay/gl/shader.h"
#include "fay/gui/gui.h"

using namespace std;
using namespace fay;

// -----------------------------------------------------------------------------

// 分辨率
const unsigned int Width = 1080;
const unsigned int Height = 720;

// timing
float currentFrame = 0.f;
float deltaTime = 0.f;
float lastFrame = 0.f;

// model
glm::vec3 model_scale(1.f);

// camera
Camera camera(glm::vec3(0, 5, 10));;
float lastX = Width / 2.0f;
float lastY = Height / 2.0f;
bool firstMouse = true;

//light 
glm::vec3 lightPosition = glm::vec3(0, 10, 0); //objectspace light position
float light_speed = 2.f;
glm::vec3 light_scale(0.5f, 0.5f, 0.5f);

// 鼠标移动设置与渲染设置
glm::vec3 mouse_move(0.f);
char mouse_state = 'z';
int render_state = 1;

// GUI
//background color，会自动转化为 0.f~1.f 的浮点数
static ImVec4 clear_color = ImColor(0, 0, 0);
static ImVec4 light_color = ImColor(255, 255, 255);
static int samples_PerPixel = 1;

// -----------------------------------------------------------------------------

void update()
{
	gui_updateIO();

	currentFrame = glfwGetTime();
	deltaTime = (currentFrame - lastFrame) * 10;
	lastFrame = currentFrame;

	ImGuiIO& io = gui_get_io();

	if (io.KeysDown[GLFW_KEY_1]) render_state = 1;	// raster
	if (io.KeysDown[GLFW_KEY_2]) render_state = 2;	// raycast
	if (io.KeysDown[GLFW_KEY_3]) render_state = 3;	// pathtracing

	// 鼠标移动
	float xpos = io.MousePos.x, ypos = io.MousePos.y;
	if (firstMouse) { lastX = xpos; lastY = ypos; firstMouse = false; }

	float xoffset = xpos - lastX;
	// reversed since y-coordinates go from bottom to top but z_xais form out to in
	float yoffset = lastY - ypos;
	lastX = xpos; lastY = ypos;

	// 粘滞
	// if (io.KeysDown[GLFW_KEY_SPACE] == GLFW_PRESS) mouse_state = ++mouse_state % 3;
	if (io.KeysDown[GLFW_KEY_Z]) mouse_state = 'z';
	if (io.KeysDown[GLFW_KEY_X]) mouse_state = 'x';
	if (io.KeysDown[GLFW_KEY_C]) mouse_state = 'c';

	if (mouse_state == 'z')
	{
		if (model_scale.x <= 1.f)
			model_scale -= glm::vec3(0.1f, 0.1f, 0.1f) * io.MouseWheel;
		else
			model_scale -= glm::vec3(1.f, 1.f, 1.f) * io.MouseWheel;

		if (model_scale.x < 0.f)
			model_scale = glm::vec3(0.1f, 0.1f, 0.1f);
		else if (model_scale.x > 10.f)
			model_scale = glm::vec3(10.f, 10.f, 10.f);

		mouse_move = glm::vec3(xoffset, yoffset, 0);

		camera.ProcessMouseMovement(xoffset, yoffset);
		if (io.KeysDown[GLFW_KEY_W]) camera.ProcessKeyboard(FORWARD, deltaTime);
		if (io.KeysDown[GLFW_KEY_S]) camera.ProcessKeyboard(BACKWARD, deltaTime);
		if (io.KeysDown[GLFW_KEY_A]) camera.ProcessKeyboard(LEFT, deltaTime);
		if (io.KeysDown[GLFW_KEY_D]) camera.ProcessKeyboard(RIGHT, deltaTime);
		//camera.ProcessMouseScroll(io.MouseWheel); 禁止放缩
	}
	else if (mouse_state == 'x')
	{
		// 向前滚为正，向后为后，根据速度从 -5 ~ 5
		light_scale -= glm::vec3(0.1f, 0.1f, 0.1f) * io.MouseWheel;
		if (light_scale.x < 0.f)
			light_scale = glm::vec3(0.1f, 0.1f, 0.1f);
		else if (light_scale.x > 1.f)
			light_scale = glm::vec3(1.f, 1.f, 1.f);

		light_speed -= io.MouseWheel;
		if (light_speed <= 0.f)
			light_speed = 0.f;
		else if (light_speed >= 10.f)
			light_speed = 10.f;

		if (io.KeysDown[GLFW_KEY_W]) lightPosition.z -= deltaTime * light_speed;
		if (io.KeysDown[GLFW_KEY_S]) lightPosition.z += deltaTime * light_speed;
		if (io.KeysDown[GLFW_KEY_A]) lightPosition.x -= deltaTime * light_speed;
		if (io.KeysDown[GLFW_KEY_D]) lightPosition.x += deltaTime * light_speed;
		if (io.MouseDown[0]) lightPosition.y += deltaTime * light_speed;
		if (io.MouseDown[1]) lightPosition.y -= deltaTime * light_speed;

		// if (io.MouseDown[2]) clear_color = ImColor(255, 255, 255);
	}
	else
	{
		// GUI
	}
}

void image_processing(const std::string& imgpath)
{
	Texture2D img{ imgpath };
	uint32_t width = img.width(), height = img.height();
	float ratio = (float)height / (float)width;

	// quad
	// left, right, bottom, top
	glm::vec4 p = { -0.5f, 0.5f, -ratio / 2.f, ratio / 2.f };
	std::vector<Vertex1> vb{ { p.x, p.z, 0 },{ p.y, p.z, 0 },{ p.y, p.w, 0 },{ p.x, p.w, 0 } };
	std::vector<uint32_t> ib{ 0,1,2,2,3,0 };
	Buffer quad{ vb, ib };

	Framebuffer fb{ width, height };
	Shader ip{ "image_processing/processing.vs", "image_processing/processing.fs" };
	Shader gui{ "image_processing/gui.vs", "image_processing/gui.fs" };

	//glViewport(0, 0, width, height);
	glm::vec3 mouse_offset(0.f);
	while (!gui_close_window())
	{
		update();

		//glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 ortho0 = glm::ortho(p.x, p.y, p.z, p.w);
		glm::mat4 model0(1.f), model1(1.f);

		fb.enable(glm::vec3(1.f, 0.f, 0.f));
		ip.enable();
		ip.set_mat4("MVP", ortho0 * model0);
		ip.bind_texture("diff", 0, img.id());
		quad.draw();

		gl_enable_framebuffer(0, Width, Height, glm::vec3(clear_color.x, clear_color.y, clear_color.z));
		gui.enable();
		mouse_offset += mouse_move;
		model1 = glm::translate(model1,
			glm::vec3(mouse_offset.x / Width, mouse_offset.y / Height, 0));
		model1 = glm::scale(model1, model_scale);
		gui.set_mat4("MVP", ortho0 * model1);
		gui.bind_texture("diff", 0, fb.tex_id());
		quad.draw();

		// GUI
		ImGui::Text("Info");
		ImGui::Text("mouse move: %c", mouse_state);
		ImGui::Text("light speed: %.1f", light_speed);
		ImGui::Text("light scale: %.1f", light_scale.x);
		ImGui::ColorEdit3("clear color", (float*)&clear_color);
		ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::SliderInt("samples_Perpixel", &samples_PerPixel, 1, 16);
		ImGui::ColorEdit3("light color", (float*)&light_color);
		gui_drawGUI();
	}
}

int main(int argc, char** argv)
{
	google::InitGoogleLogging(argv[0]);
	gui_create_window(Width, Height);

	image_processing("image_processing/1.png");

	gui_delete_window();
	return 0;
}