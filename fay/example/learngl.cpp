#include "fay/utility/fay.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>

#include "fay/gl/buffer.h"
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

// objects
const string Nier_2b = "objects/Nier_2b/2b.obj";
const string Box = "objects/box/box.obj";
const string Blocks = "objects/blocks/blocks.obj";
const string Rei = "objects/Rei/Rei.obj";
const string CornellBox = "objects/CornellBox/CornellBox.obj";
const string Rock = "objects/rock/rock.obj";
const string Fairy = "objects/fairy/fairy.obj";
const string Nanosuit = "objects/nanosuit/nanosuit.obj";
const string silly_dancing = "objects/silly_dancing.fbx";
const string nierautomata_2b = "objects/nierautomata_2b/scene.gltf";
const string ftm_sketchfab = "objects/ftm/ftm_sketchfab.blend";
const string Nier_2b_ik_rigged = "objects/Nier_2b_ik_rigged/scene.gltf";
// const string mesh_filename = Blocks;

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
char mouse_move = 'z';
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
	// if (io.KeysDown[GLFW_KEY_SPACE] == GLFW_PRESS) mouse_move = ++mouse_move % 3;
	if (io.KeysDown[GLFW_KEY_Z]) mouse_move = 'z';
	if (io.KeysDown[GLFW_KEY_X]) mouse_move = 'x';
	if (io.KeysDown[GLFW_KEY_C]) mouse_move = 'c';

	if (mouse_move == 'z')
	{
		camera.ProcessMouseMovement(xoffset, yoffset);
		if (io.KeysDown[GLFW_KEY_W]) camera.ProcessKeyboard(FORWARD, deltaTime);
		if (io.KeysDown[GLFW_KEY_S]) camera.ProcessKeyboard(BACKWARD, deltaTime);
		if (io.KeysDown[GLFW_KEY_A]) camera.ProcessKeyboard(LEFT, deltaTime);
		if (io.KeysDown[GLFW_KEY_D]) camera.ProcessKeyboard(RIGHT, deltaTime);
		//camera.ProcessMouseScroll(io.MouseWheel); 禁止放缩
	}
	else if (mouse_move == 'x')
	{
		// 向前滚为正，向后为后，根据速度从 -5 ~ 5
		light_scale -= glm::vec3(0.1f, 0.1f, 0.1f) * io.MouseWheel;
		if (light_scale.x < 0.f)
			light_scale = glm::vec3(0.1f, 0.1f, 0.1f);
		if (light_scale.x > 1.f)
			light_scale = glm::vec3(1.f, 1.f, 1.f);

		light_speed -= io.MouseWheel;
		if (light_speed <= 0.f)
			light_speed = 0.f;
		if (light_speed >= 10.f)
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

// -----------------------------------------------------------------------------

// tutorials
// function<void()> _create_gui();
struct _00_create_gui;

struct _10_hello_triangle;

struct _20_color;
struct _21_basic_light;
struct _22_material;
struct _23_light_map;

struct _30_load_mesh;
struct _31_load_model;

struct _fay_obj_model;

// -----------------------------------------------------------------------------

struct _00_create_gui
{
	// 加载覆盖整个视口的正方形
	std::vector<Vertex1> vb{ {-1, -1, 0}, {1, -1, 0}, {1, 1, 0}, {-1, 1, 0} };
	std::vector<uint32_t> ib{ 0,1,2,0,2,3 };
	Buffer quad{ vb, ib };
	Shader shader{ "learngl/00_gui.vs", "learngl/00_gui.fs" };
	Texture2D diff{ "textures/awesomeface.png" };

	void draw(glm::mat4& MVP)
	{
		shader.enable();
		shader.bind_texture("diff", 0, diff.id());
		shader.set_mat4("MVP", MVP);
		quad.draw();
	}
};

// -----------------------------------------------------------------------------

struct _20_color
{
	Model model{ Rei };
	Shader shader{ "learngl/20_color.vs", "learngl/20_color.fs" };
	// Shader shader{ "learngl/light.vs", "learngl/light.fs" };

	float sAmbient = 0.1f;
	float strDiffuse = 1.f;
	float sSpeclar = 0.1f;

	void draw(glm::mat4& p, glm::mat4& v, glm::mat4& m)
	{
		// TODO: bind imgui to shader
		ImGui::SliderFloat("ambient strength", &sAmbient, 0.f, 1.f);
		ImGui::SliderFloat("diffuse strength", &strDiffuse, 0.f, 10.f);
		ImGui::SliderFloat("speclar strength", &sSpeclar, 0.f, 1.f);

		glm::mat4 mat4_normal = glm::transpose(glm::inverse(m));
		// glm::mat3 m_normal = mat4_normal;
		glm::mat3 m_normal(mat4_normal);

		shader.enable();
		shader.set_mat4("model", m);
		shader.set_mat3("NorModel", m_normal);
		shader.set_mat4("MVP", p * v * m);
		shader.set_vec3("lightPos", lightPosition);
		shader.set_vec3("lightcolor", glm::vec3(light_color.x, light_color.y, light_color.z));
		shader.set_float("sAmbient", sAmbient);
		shader.set_float("strDiffuse", strDiffuse);
		shader.set_float("sSpeclar", sSpeclar);
		// shader.set_vec3("lightcolor", glm::vec3(1, 1, 1));
		model.draw(shader);
		// model.draw();
	}
};

// -----------------------------------------------------------------------------

struct _30_load_mesh
{
	Model model{ Nanosuit };
	Shader shader{ "learngl/30_load_model.vs", "learngl/30_load_model.fs" };

	void draw(glm::mat4& MVP)
	{
		shader.enable();
		shader.set_mat4("MVP", MVP);
		model.draw(shader);
	}
};

struct _31_load_model
{
	Model model{ Nanosuit };
	Shader shader{ "learngl/31_load_model.vs", "learngl/31_load_model.fs" };

	void draw(glm::mat4& MVP)
	{
		shader.enable();
		shader.set_mat4("MVP", MVP);
		model.draw(shader);
	}
};

// -----------------------------------------------------------------------------

struct _fay_obj_model
{
	obj_Model model{ Fairy };
	// Shader shader{ "learngl/31_load_model.vs", "learngl/31_load_model.fs" };
	Shader shader{ "learngl/light.vs", "learngl/light.fs" };

	void draw(glm::mat4& MVP)
	{
		shader.enable();
		shader.set_mat4("MVP", MVP);
		shader.set_vec3("lightcolor", glm::vec3(1, 1, 1));
		// model.draw(shader);
		model.draw();
	}
};

// -----------------------------------------------------------------------------

int main(int argc, char** argv)
{
	google::InitGoogleLogging(argv[0]);
	//--stderrthreshold=0 --logtostderr=true
	//FLAGS_logtostderr = true;
	//FLAGS_stderrthreshold = 0;
	//FLAGS_v = 2;

	gui_create_window(Width, Height);

	_20_color object;

	Model light{ Box };
	Shader lightshader{ "learngl/light.vs", "learngl/light.fs" };

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	while (!gui_close_window())
	{
		update();

		glClearColor(clear_color.x, clear_color.y, clear_color.z, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// glm::vec3 position = { 0.f, 0.f, 1.f }, center = { 0, 0, 0 }, up = { 0, 1, 0 };
		// glm::mat4 view = glm::lookAt(position, center, up);
		// TODO：获得对象的大小并压缩到一个固定的方位内
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
			(float)Width / (float)Height, 0.1f, 10000.0f);

		// draw
		glm::mat4 model(1.f);
		// 1.0f, 1.0f, 1.0f
		// 10.f, 10.f, 10.f
		// 0.5f, 0.5f, 0.5f
		// 0.3f, 0.3f, 0.3f
		model = glm::scale(model, glm::vec3(10.f, 10.f, 10.f));
		glm::mat4 MVP = projection * view * model;
		object.draw(projection, view, model);

		// draw light
		// glDisable(GL_DEPTH_TEST);
		{
			glDisable(GL_DEPTH_TEST);
			lightshader.enable();
			glm::mat4 mLight{ 1 };
			mLight = glm::translate( mLight, lightPosition);
			mLight = glm::scale(mLight, light_scale);
			lightshader.set_mat4("MVP", projection * view * mLight);
			// TODO: print error
			// lightshader.set_vec3("lihgtcolor", glm::vec3(1, 1, 1));
			lightshader.set_vec3("lightcolor", glm::vec3(light_color.x, light_color.y, light_color.z));
			light.draw();
			// lightshader.disable();
			glEnable(GL_DEPTH_TEST);
		}

		// GUI
		// Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
		{
			ImGui::Text("Info"); 
			ImGui::Text("mouse move: %c", mouse_move);
			ImGui::Text("light speed: %.1f", light_speed);
			ImGui::Text("light scale: %.1f", light_scale.x);
			ImGui::ColorEdit3("clear color", (float*)&clear_color);
			ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::SliderInt("samples_Perpixel", &samples_PerPixel, 1, 16);
			ImGui::ColorEdit3("light color", (float*)&light_color);
		}

		gui_drawGUI();
	}

	gui_delete_window();
	return 0;
}