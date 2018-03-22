#include "fay/utility/fay.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "fay/gl/buffer.h"
#include "fay/gl/framebuffer.h"
#include "fay/gl/texture.h"
#include "fay/gl/mesh.h"
#include "fay/gl/model.h"
#include "fay/gl/camera.h"
#include "fay/gl/shader.h"
#include "fay/gl/uniform.h"
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

	// 鼠标移动
	float xpos = io.MousePos.x, ypos = io.MousePos.y;
	if (firstMouse) { lastX = xpos; lastY = ypos; firstMouse = false; }
	float xoffset = xpos - lastX; lastX = xpos;
	// reversed since y-coordinates go from bottom to top but z_xais form out to in
	float yoffset = lastY - ypos; lastY = ypos;

	// 粘滞
	// if (io.KeysDown[GLFW_KEY_SPACE] == GLFW_PRESS) mouse_move = ++mouse_move % 3;
	if (io.KeysDown[GLFW_KEY_Z]) mouse_move = 'z';
	if (io.KeysDown[GLFW_KEY_X]) mouse_move = 'x';
	if (io.KeysDown[GLFW_KEY_C]) mouse_move = 'c';

	if (mouse_move == 'z')
	{
		if (model_scale.x <= 1.f)
			model_scale -= glm::vec3(0.1f, 0.1f, 0.1f) * io.MouseWheel;
		else
			model_scale -= glm::vec3(1.f, 1.f, 1.f) * io.MouseWheel;

		if (model_scale.x < 0.f)
			model_scale = glm::vec3(0.1f, 0.1f, 0.1f);
		else if (model_scale.x > 10.f)
			model_scale = glm::vec3(10.f, 10.f, 10.f);

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

// -----------------------------------------------------------------------------

// tutorials
// function<void()> _create_gui();
struct _00_create_gui;	// load buffer
// hello world
struct _10_hello_triangle;
// pipeline
struct _20_load_mesh;
struct _21_load_model;
struct _22_depth_test;
struct _23_stencil_test;	// Object Outlining
struct _24_blending;
struct _25_framebuffers;
struct _26_cubemaps;
// light
struct _30_phong_shading;
struct _31_light_caster;
//struct _32_mutilights;

// render

// other
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

	void draw(glm::mat4&& MVP)
	{
		shader.enable();
		shader.bind_texture("diff", 0, diff);
		shader.set_mat4("MVP", MVP);
		quad.draw();
	}
};

// -----------------------------------------------------------------------------

struct _20_load_mesh
{
	Model model{ Nanosuit };
	Shader shader{ "learngl/20_load_model.vs", "learngl/20_load_model.fs" };

	void draw(glm::mat4&& MVP)
	{
		shader.enable();
		shader.set_mat4("MVP", MVP);
		model.draw(shader);
	}
};

struct _21_load_model
{
	Model model{ Nanosuit };
	Shader shader{ "learngl/21_load_model.vs", "learngl/21_load_model.fs" };

	void draw(glm::mat4&& MVP)
	{
		shader.enable();
		shader.set_mat4("MVP", MVP);
		model.draw(shader);
	}
};

struct _22_depth_test
{
	Model model{ Blocks };
	Shader shader{ "learngl/22_depth_test.vs", "learngl/22_depth_test.fs" };

	void draw(glm::mat4&& MVP)
	{
		//glDepthMask(GL_FALSE);
		//glDepthFunc(GL_ALWAYS);
		shader.enable();
		shader.set_mat4("MVP", MVP);
		model.draw(shader);
	}
};

struct _23_stencil_test
{
	Model model{ Box };
	Shader shader{ "learngl/23_stencil_test.vs", "learngl/23_stencil_test.fs" };

	void draw(glm::mat4& p, glm::mat4& v, glm::mat4& m)
	{
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

		glStencilFunc(GL_ALWAYS, 1, 0xFF); // 所有的片段都应该更新模板缓冲
		glStencilMask(0xFF); // 启用模板缓冲写入
		shader.enable();
		shader.set_bool("draw_outlining", false);
		shader.set_mat4("MVP", p * v * m);
		model.draw(shader);
		
		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		glStencilMask(0x00); // 禁止模板缓冲的写入
		glDisable(GL_DEPTH_TEST);
		shader.enable();
		shader.set_bool("draw_outlining", true);
		glm::mat4 scaleModel = glm::scale(m, glm::vec3(1.1f, 1.1f, 1.1f));
		shader.set_mat4("MVP", p * v * scaleModel);
		model.draw(shader);
		glStencilMask(0xFF);
		glEnable(GL_DEPTH_TEST);
	}
};

struct _24_blending
{
	// plane
	std::vector<Vertex1> planevb{ { -1, 0, 1 },{ 1, 0, 1 },{ 1, 0, -1 },{ -1, 0, -1 } };
	std::vector<uint32_t> planeib{ 0,1,2,2,3,0 };
	Buffer planeQuad{ planevb, planeib };
	// grass
	std::vector<Vertex1> vb{ { 0, 0, 0 },{ 1, 0, 0 },{ 1, 1, 0 },{ 0, 1, 0 } };
	std::vector<uint32_t> ib{ 0,1,2,2,3,0 };
	Buffer quad{ vb, ib };

	Shader shader{ "learngl/24_blending.vs", "learngl/24_blending.fs" };
	Texture2D green{ "textures/grass.png" };
	Texture2D marble{ "textures/marble.jpg" };
	Texture2D window{ "textures/window.png" };

	void draw(glm::mat4& p, glm::mat4& v, glm::mat4& m)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		shader.enable();
		glm::mat4 model(1.f);

		shader.bind_texture("diff", 0, marble);
		shader.set_bool("texture_xz", true);
		glm::mat4 m0 = glm::scale(model, glm::vec3(10.f, 10.f, 10.f));
		shader.set_mat4("MVP", p * v * m0);
		planeQuad.draw();

		glDisable(GL_CULL_FACE);
		shader.bind_texture("diff", 0, green);
		shader.set_bool("texture_xz", false);

		glm::mat4 m1 = glm::scale(model, glm::vec3(3.f, 3.f, 3.f));
		shader.set_mat4("MVP", p * v * m1);
		quad.draw();

		//glm::mat4 m2 = glm::translate(model, glm::vec3(-1, 0, 0));
		glm::mat4 m2 = glm::translate(model, glm::vec3(-3, 0, 0));
		m2 = glm::scale(m2, glm::vec3(3.f, 3.f, 3.f));
		shader.set_mat4("MVP", p * v * m2);
		quad.draw();
		
		shader.bind_texture("diff", 0, window);
		glm::mat4 m3 = glm::translate(model, glm::vec3(-0.5f, 0, 1));
		m3 = glm::scale(m3, glm::vec3(3.f, 3.f, 3.f));
		shader.set_mat4("MVP", p * v * m3);
		quad.draw();
		glEnable(GL_CULL_FACE);
	}
};

struct _25_framebuffers
{
	Model model{ Blocks };
	// quad
	std::vector<Vertex1> vb{ { 0, 0, 0 },{ 1, 0, 0 },{ 1, 1, 0 },{ 0, 1, 0 } };
	std::vector<uint32_t> ib{ 0,1,2,2,3,0 };
	Buffer quad{ vb, ib };

	Framebuffer fb{Width, Height};
	Texture2D green{ "textures/grass.png" };
	Shader shader{ "learngl/25_framebuffers.vs", "learngl/25_framebuffers.fs" };
	Shader gui{ "learngl/00_gui.vs", "learngl/00_gui.fs" };

	void draw(glm::mat4& p, glm::mat4& v, glm::mat4& m)
	{
		fb.enable(glm::vec3(0.f, 0.f, 0.f));
		shader.enable();
		shader.set_mat4("MVP", p * v * m);
		model.draw(shader);

		gl_enable_framebuffer(0, Width, Height, glm::vec3(clear_color.x, clear_color.y, clear_color.z));
		gui.enable();
		glm::mat4 m0(1.f);
		m0 = glm::translate(m0, glm::vec3(-4, -4, -4));
		m0 = glm::scale(m0, glm::vec3(8.f, 8.f, 8.f));
		shader.set_mat4("MVP", p * m0);
		gui.bind_texture("diff", 0, fb.tex());
		quad.draw();
	}
};

struct _26_cubemaps
{
	Model model{ Nier_2b };
	Model box{ Box };

	const string files[7] = { "skybox/blue_sky/",
		"right.jpg", "left.jpg",
		"top.jpg", "bottom.jpg",
		"back.jpg", "front.jpg" };
	TextureCube sky{files};

	Shader shader{ "learngl/26_environment_map.vs", "learngl/26_environment_map.fs" };
	Shader cubemap{ "learngl/26_cubemaps.vs", "learngl/26_cubemaps.fs" };

	void draw(glm::mat4& p, glm::mat4& v, glm::mat4& m)
	{
		shader.enable();

		glm::mat4 MV = v * m;
		glm::mat3 NormalMV = glm::mat3(glm::transpose(glm::inverse(MV)));
		shader.enable();
		shader.set_mat4("MV", MV);
		shader.set_mat3("NormalMV", NormalMV);		// 失去位移属性
		shader.set_mat4("MVP", p * v * m);
		model.draw(shader);

		glCullFace(GL_FRONT);
		glDepthFunc(GL_LEQUAL);
		//glDepthMask(GL_FALSE);
		cubemap.enable();

		glm::mat4 view_withoutMove = glm::mat4(glm::mat3(v));
		cubemap.set_mat4("MVP", p * view_withoutMove);
		cubemap.bind_texture("cubemap", 0, sky);

		box.draw();
		glDepthFunc(GL_LESS);
		//glDepthMask(GL_TRUE);
		glCullFace(GL_BACK);
	}
};

struct _27_uniform
{
	Model model{ Box };
	Uniform uniform{ 128 };	// 2 * mat4, mat4 = 4 * vec4 = 16 * float = 64 * bytes
	Shader shader1{ "learngl/27_uniform.vs", "learngl/27_uniform.fs" };
	Shader shader2{ "learngl/27_uniform.vs", "learngl/27_uniform.fs" };

	void draw(glm::mat4& p, glm::mat4& v, glm::mat4& m)
	{
		// struct Mat { mat4 p; mat v; }
		// uniform.set(OFFSET(p), sizeof(v), &v);
		CHECK(sizeof(glm::mat4) == 64);
		uniform.set(0, 64, glm::value_ptr(p));
		uniform.set(64, 64, glm::value_ptr(v));

		shader1.enable();
		shader1.set_mat4("model", m);
		shader1.bind_uniform("Mat", 0, uniform);
		shader1.set_bool("inverse", false);
		model.draw(shader1);

		shader2.enable();
		shader2.set_mat4("model", glm::translate(m, lightPosition));
		shader2.bind_uniform("Mat", 0, uniform);
		shader2.set_bool("inverse", true);
		model.draw(shader2);
	}
};

struct _xx
{
	Model model{ Blocks };
	Shader shader{ "learngl/xxxxxxxxxxxx.vs", "learngl/xxxxxxxxxxxxxx.fs" };

	void draw(glm::mat4& p, glm::mat4& v, glm::mat4& m)
	{
		shader.enable();
		shader.set_mat4("MVP", p * v * m);
		model.draw(shader);
	}
};

// -----------------------------------------------------------------------------

struct _30_phong_shading
{
	Model model{ Box };
	Shader shader{ "learngl/30_light_ADS.vs", "learngl/30_light_ADS.fs" };
	// Shader shader{ "learngl/light.vs", "learngl/light.fs" };

	float sAmbient = 1.f;
	float sDiffuse = 1.f;
	float sSpeclar = 1.f;
	int   shininess = 32;

	void draw(glm::mat4& p, glm::mat4& v, glm::mat4& m)
	{
		// TODO: bind imgui to shader
		ImGui::SliderFloat("ambient strength", &sAmbient, 0.f, 10.f);
		ImGui::SliderFloat("diffuse strength", &sDiffuse, 0.f, 10.f);
		ImGui::SliderFloat("speclar strength", &sSpeclar, 0.f, 10.f);
		ImGui::SliderInt("shininess", &shininess, 1, 256);

		glm::mat4 MV = v * m;
		glm::mat3 NormalMV = glm::mat3(glm::transpose(glm::inverse(MV)));

		shader.enable();
		shader.set_mat4("MV", MV);
		//shader.set_mat4("NorMV", NormalMV);	// 小心传输着色器变量、先保存再编译
		shader.set_mat3("NormalMV", NormalMV);		// 失去位移属性
		shader.set_mat4("MVP", p * v * m);
		shader.set_vec3("vLightPos", glm::vec3(v * glm::vec4(lightPosition, 1.f)));
		shader.set_vec3("lightcolor", glm::vec3(light_color.x, light_color.y, light_color.z));
		shader.set_float("sa", sAmbient);
		shader.set_float("sd", sDiffuse);
		shader.set_float("ss", sSpeclar);
		shader.set_int("shininess", shininess);
		model.draw(shader);
		// model.draw();
		gl_check_errors();
	}
};

struct _31_light_caster
{
	Model model{ Blocks };
	Shader shader{ "learngl/31_light_caster.vs", "learngl/31_light_caster.fs" };
	// Shader shader{ "learngl/light.vs", "learngl/light.fs" };

	float sAmbient = 1.f;
	float sDiffuse = 1.f;
	float sSpeclar = 1.f;
	int   shininess = 32;

	void draw(glm::mat4& p, glm::mat4& v, glm::mat4& m)
	{
		// TODO: bind imgui to shader
		ImGui::SliderFloat("ambient strength", &sAmbient, 0.f, 10.f);
		ImGui::SliderFloat("diffuse strength", &sDiffuse, 0.f, 100.f);
		ImGui::SliderFloat("speclar strength", &sSpeclar, 0.f, 10.f);
		ImGui::SliderInt("shininess", &shininess, 1, 256);

		glm::mat4 MV = v * m;
		glm::mat3 NormalMV = glm::mat3(glm::transpose(glm::inverse(MV)));

		shader.enable();
		shader.set_mat4("MV", MV);
		//shader.set_mat4("NorMV", NormalMV);	// 小心传输着色器变量、先保存再编译
		shader.set_mat3("NormalMV", NormalMV);		// 失去位移属性
		shader.set_mat4("MVP", p * v * m);

		// directlight
		shader.set_vec3("dLight.direct", glm::vec3(-1.f, -1.f, -1.f));
		shader.set_vec3("dLight.color", glm::vec3(light_color.x, light_color.y, light_color.z));
		// pointlight
		shader.set_vec3("pLight.pos", glm::vec3(v * glm::vec4(lightPosition, 1.f)));
		shader.set_vec3("pLight.color", glm::vec3(light_color.x, light_color.y, light_color.z));
		shader.set_vec3("pLight.falloff", glm::vec3(1.0, 0.022, 0.0019));	// 100 个单位
		// spotlight
		shader.set_vec3("sLight.pos", camera.Position);
		shader.set_vec3("sLight.direct", camera.Front);
		shader.set_vec3("sLight.color", glm::vec3(light_color.x, light_color.y, light_color.z));
		shader.set_vec2("sLight.cutoff", glm::vec2(
			glm::cos(glm::radians(10.f)), glm::cos(glm::radians(15.f)) ));
		shader.set_vec3("sLight.falloff", glm::vec3(1.0, 0.022, 0.0019));	// 100 个单位
		
		shader.set_float("sa", sAmbient);
		shader.set_float("sd", sDiffuse);
		shader.set_float("ss", sSpeclar);
		shader.set_int("shininess", shininess);
		model.draw(shader);
		// model.draw();
		gl_check_errors();
	}
};

// -----------------------------------------------------------------------------

struct _fay_obj_model
{
	obj_Model model{ Fairy };
	// Shader shader{ "learngl/31_load_model.vs", "learngl/31_load_model.fs" };
	Shader shader{ "learngl/light.vs", "learngl/light.fs" };

	void draw(glm::mat4&& MVP)
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

	_27_uniform object;

	Model light{ Box };
	Shader lightshader{ "learngl/light.vs", "learngl/light.fs" };

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glEnable(GL_CULL_FACE);

	while (!gui_close_window())
	{
		update();

		glClearColor(clear_color.x, clear_color.y, clear_color.z, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		// glm::vec3 position = { 0.f, 0.f, 1.f }, center = { 0, 0, 0 }, up = { 0, 1, 0 };
		// glm::mat4 view = glm::lookAt(position, center, up);
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
			(float)Width / (float)Height, 0.1f, 10000.0f);

		// draw
		glm::mat4 model(1.f);
		model = glm::scale(model, model_scale);
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