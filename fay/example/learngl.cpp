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
const string Planet = "objects/planet/planet.obj";
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
bool some_flag = false;
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

struct Box	// gamma_correction, 
{
	// quad
	std::vector<Vertex1> vb{ { 0, 0, 0 },{ 1, 0, 0 },{ 1, 1, 0 },{ 0, 1, 0 } };
	std::vector<uint32_t> ib{ 0,1,2,2,3,0 };
	Buffer quad{ vb, ib };

	Texture2D green{ "textures/grass.png" };

	Shader gui{ "learngl/post_processing.vs", "learngl/post_processing.fs" };

	void draw_gui(const BaseTexture& tex)
	{
		gui.enable();
		glm::mat4 ortho0 = glm::ortho(0.f, 1.f, 0.f, 1.f);
		gui.set_mat4("MVP", ortho0);
		gui.set_bool("gamma_correction", false);
		gui.bind_texture("diffuse", 0, tex);
		quad.draw();
	}
};

struct Post_Processing	// gamma_correction, 
{
	// quad
	std::vector<Vertex1> vb{ { 0, 0, 0 },{ 1, 0, 0 },{ 1, 1, 0 },{ 0, 1, 0 } };
	std::vector<uint32_t> ib{ 0,1,2,2,3,0 };
	Buffer quad{ vb, ib };

	Texture2D green{ "textures/grass.png" };

	Shader gui{ "learngl/post_processing.vs", "learngl/post_processing.fs" };

	void draw_gui(const BaseTexture& tex)
	{
		gui.enable();
		glm::mat4 ortho0 = glm::ortho(0.f, 1.f, 0.f, 1.f);
		gui.set_mat4("MVP", ortho0);
		gui.set_bool("gamma_correction", false);
		gui.bind_texture("diffuse", 0, tex);
		quad.draw();
	}
};

struct _25_framebuffers : public Post_Processing
{
	Model model{ Blocks };

	FrameBuffer fb{Width, Height};	// doesn't use mulitesample, need to modify code in the main()
	Shader shader{ "learngl/25_framebuffers.vs", "learngl/25_framebuffers.fs" };

	void draw(glm::mat4& p, glm::mat4& v, glm::mat4& m)
	{
		fb.enable(glm::vec3(0.f, 0.f, 0.f));
		shader.enable();
		shader.set_mat4("MVP", p * v * m);
		model.draw(shader);

		gl_enable_framebuffer(0, Width, Height, glm::vec3(clear_color.x, clear_color.y, clear_color.z));
		draw_gui(fb.tex());
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

struct _28_geometry_shader
{
	float time{};
	Model model{ Fairy };
	Shader bomb{ "learngl/28_geometry_shader.vs", "learngl/28_gs_bomb.fs", "learngl/28_gs_bomb.gs" };
	Shader normal{ "learngl/28_geometry_shader.vs", "learngl/28_gs_normal.fs", "learngl/28_gs_normal.gs" };

	void draw(glm::mat4& p, glm::mat4& v, glm::mat4& m)
	{
		bomb.enable();
		bomb.set_mat4("MVP", p * v * m);
		if (mouse_move != 'z')
		{
			time += deltaTime / 10;
			bomb.set_float("time", time);
		}
		model.draw(bomb);

		normal.enable();
		normal.set_mat3("NormalMV", glm::mat3(glm::transpose(glm::inverse(v * m))));
		normal.set_mat4("P", p);
		normal.set_mat4("MVP", p * v * m);
		normal.set_float("Length", 1.2f);
		model.draw(normal);
	}
};

struct _29_instancing
{
	Model planet{ Planet };
	Model rock{ Rock };
	Shader model{ "learngl/21_load_model.vs", "learngl/21_load_model.fs" };
	Shader shader{ "learngl/29_instancing.vs", "learngl/29_instancing.fs" };
	// 使用独显运行
	const int amount = 100000;
	std::vector<glm::mat4> mat4s{ amount };

	_29_instancing()
	{
		srand(glfwGetTime());
		float radius = 100.0;
		float offset = 25.0f;

		for (unsigned int i = 0; i < amount; i++)
		{
			glm::mat4 model{ 1 };
			// 1. translation: displace along circle with 'radius' in range [-offset, offset]
			float angle = (float)i / (float)amount * 360.0f;
			float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
			float x = sin(angle) * radius + displacement;
			displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
			float y = displacement * 0.4f; // keep height of asteroid field smaller compared to width of x and z
			displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
			float z = cos(angle) * radius + displacement;
			model = glm::translate(model, glm::vec3(x, y, z));

			// 2. scale: Scale between 0.05 and 0.25f
			float scale = (rand() % 20) / 100.0f + 0.05;
			model = glm::scale(model, glm::vec3(scale));

			// 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector
			float rotAngle = (rand() % 360);
			model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

			// 4. now add to list of matrices
			mat4s[i] = model;
		}

		unsigned int instanceVBO;
		glGenBuffers(1, &instanceVBO);
		glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
		glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), mat4s.data(), GL_STATIC_DRAW);

		for (unsigned int i = 0; i < rock.meshes.size(); ++i)
		{
			glBindVertexArray(rock.meshes[i].buffer.id());

			GLsizei vec4Size = sizeof(glm::vec4);
			glEnableVertexAttribArray(3);
			glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)0);
			glEnableVertexAttribArray(4);
			glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(vec4Size));
			glEnableVertexAttribArray(5);
			glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(2 * vec4Size));
			glEnableVertexAttribArray(6);
			glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(3 * vec4Size));

			glVertexAttribDivisor(3, 1);
			glVertexAttribDivisor(4, 1);
			glVertexAttribDivisor(5, 1);
			glVertexAttribDivisor(6, 1);

			glBindVertexArray(0);
		}
	}

	void draw(glm::mat4& p, glm::mat4& v, glm::mat4& m)
	{
		model.enable();
		model.set_mat4("MVP", p * v * m);
		planet.draw(model, amount);
		
		shader.enable();
		shader.set_mat4("PV", p * v);
		rock.draw(shader, amount);
	}
};

struct _2a_anti_aliasing : public Post_Processing
{
	Model model{ Blocks };

	MultiSampleFrameBuffer fb{ Width, Height };	// use mulitesample

	Shader shader{ "learngl/25_framebuffers.vs", "learngl/25_framebuffers.fs" };

	void draw(glm::mat4& p, glm::mat4& v, glm::mat4& m)
	{
		fb.enable(glm::vec3(0.f, 0.f, 0.f));
		shader.enable();
		shader.set_mat4("MVP", p * v * m);
		model.draw(shader);

		fb.blit();

		gl_enable_framebuffer(0, Width, Height, glm::vec3(clear_color.x, clear_color.y, clear_color.z));
		draw_gui(fb.tex());
	}
};

// -----------------------------------------------------------------------------

struct _30_phong_shading
{
	Model model{ Nier_2b };
	Shader shader{ "learngl/30_phong_shading.vs", "learngl/30_phong_shading.fs" };
	// Shader shader{ "learngl/light.vs", "learngl/light.fs" };

	float sAmbient = 1.f;
	float sDiffuse = 1.f;
	float sSpeclar = 1.f;
	int   shininess = 32;

	void draw(glm::mat4& p, glm::mat4& v, glm::mat4& m)
	{
		// TODO: bind imgui to shader
		if (ImGui::Button("blinn_phong"))
			some_flag ^= 1;
		//cout << (some_flag ? "true\n" : "false\n");
		ImGui::SliderFloat("ambient strength", &sAmbient, 0.f, 10.f);
		ImGui::SliderFloat("diffuse strength", &sDiffuse, 0.f, 10.f);
		ImGui::SliderFloat("speclar strength", &sSpeclar, 0.f, 10.f);
		ImGui::SliderInt("shininess", &shininess, 1, 256);

		glm::mat4 MV = v * m;
		glm::mat3 NormalMV = glm::mat3(glm::transpose(glm::inverse(MV)));

		shader.enable();
		shader.set_bool("blinn_phong", some_flag);
		shader.set_mat4("MV", MV);
		//shader.set_mat4("NorMV", NormalMV);	// 小心传输着色器变量、先保存再编译
		shader.set_mat3("NormalMV", NormalMV);		// 失去位移属性
		shader.set_mat4("MVP", p * v * m);
		shader.set_vec3("vLightPos", glm::vec3(v * glm::vec4(lightPosition, 1.f)));
		shader.set_vec3("Lightcolor", glm::vec3(light_color.x, light_color.y, light_color.z));
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

struct _32_shadow_map : public Post_Processing
{
	Model model{ Blocks };
	Shader shader{ "learngl/32_shadow_map.vs", "learngl/32_shadow_map.fs" };
	Shader shadowModel{ "learngl/32_shadow_model.vs", "learngl/32_shadow_model.fs" };
	ShadowMapFrameBuffer smfb;

	void draw(glm::mat4& p, glm::mat4& v, glm::mat4& m)
	{
		GLfloat near_plane = 1.0f, far_plane = 512.f;

		// 使用正交相机时，深度是线性的
		glm::mat4 lightOrtho = glm::ortho(-512.0f, 512.0f, -512.0f, 512.0f, near_plane, far_plane);
		// 使用投影相机时，只有接近近平面的地方效果比较好
		glm::mat4 lightProj = glm::perspective(glm::radians(120.f),
			1.f / 1.f, 10.f, 512.f);
		
		glm::mat4 lightView = glm::lookAt(
			lightPosition, glm::vec3(0.0f), glm::vec3(0.f, 1.f, 0.f));
		glm::mat4 lightSpace = lightProj * lightView;
		
		glCullFace(GL_FRONT);
		smfb.enable(glm::vec3(0.f, 0.f, 0.f));
		shader.enable();
		shader.set_mat4("MVP", lightSpace * m);
		model.draw(shader);
		glCullFace(GL_BACK);

		gl_enable_framebuffer(0, Width, Height, glm::vec3(clear_color.x, clear_color.y, clear_color.z));
		shadowModel.enable();
		shadowModel.set_mat4("Proj", p);
		shadowModel.set_mat4("View", v);
		shadowModel.set_mat4("Model", m);
		shadowModel.set_mat4("LightSpace", lightSpace);
		shadowModel.set_vec3("LightPos", lightPosition);
		shadowModel.set_vec3("ViewPos", camera.Position);
		shadowModel.bind_texture("Shadowmap", 3, smfb.tex());
		model.draw(shadowModel);
	}
};

struct Light_Parameter
{
	float sAmbient = 0.2f;
	float sDiffuse = 1.f;
	float sSpeclar = 0.f;
	int   shininess = 32;

	void set(glm::mat4& p, glm::mat4& v, glm::mat4& m, Shader& shader)
	{
		if (ImGui::Button("normal map"))
			some_flag ^= 1;
		if (some_flag)
			ImGui::Text("use_normal_map");

		ImGui::SliderFloat("ambient strength", &sAmbient, 0.f, 10.f);
		ImGui::SliderFloat("diffuse strength", &sDiffuse, 0.f, 10.f);
		ImGui::SliderFloat("speclar strength", &sSpeclar, 0.f, 10.f);
		ImGui::SliderInt("shininess", &shininess, 1, 256);

		shader.enable();
		shader.set_mat4("Model", m);
		shader.set_mat4("MVP", p * v * m);
		shader.set_vec3("LightPos", lightPosition);
		shader.set_vec3("ViewPos", camera.Position);

		shader.set_bool("use_normal_map", some_flag);
		shader.set_float("sa", sAmbient);
		shader.set_float("sd", sDiffuse);
		shader.set_float("ss", sSpeclar);
		shader.set_int("shininess", shininess);
	}
};

struct _34_normal_map : public Light_Parameter
{
	Model model{ Nier_2b };
	Shader shader{ "learngl/34_normal_map.vs", "learngl/34_normal_map.fs" };

	void draw(glm::mat4& p, glm::mat4& v, glm::mat4& m)
	{
		set(p, v, m, shader);
		model.draw(shader);
	}
};

struct _35_parallax_map : public Light_Parameter
{
	Model model{ Box };
	// TODO：改进方法
	Shader shader{ "learngl/34_normal_map.vs", "learngl/35_parallax_map.fs" };

	Texture2D diffuse{ "textures/bricks2.jpg" };
	Texture2D normal{  "textures/bricks2_normal.jpg" };
	Texture2D height{  "textures/bricks2_disp.jpg" };

	float heightScale = 0.1;

	void draw(glm::mat4& p, glm::mat4& v, glm::mat4& m)
	{
		ImGui::SliderFloat("heightScale", &heightScale, 0.f, 1.f);

		set(p, v, m, shader);
		shader.bind_texture("Diffuse", 0, diffuse);
		shader.bind_texture("Normal", 1, normal);
		shader.bind_texture("Depth", 2, height);
		shader.set_float("heightScale", heightScale);
		model.draw();
	}
};

struct _38_deferred_shading
{
	// quad
	std::vector<Vertex1> vb{ { 0, 0, 0 },{ 1, 0, 0 },{ 1, 1, 0 },{ 0, 1, 0 } };
	std::vector<uint32_t> ib{ 0,1,2,2,3,0 };
	Buffer quad{ vb, ib };

	Model model{ Nanosuit };
	Model box{ Box };
	Shader geometryPass{ "learngl/30_phong_shading.vs", "learngl/38_g_buffer.fs" };
	Shader LightPass{ "learngl/post_processing.vs", "learngl/38_deferred_shading.fs" };
	Shader lightshader{ "learngl/light.vs", "learngl/light.fs" };
	GBufferFrameBuffer gbuffer{ Width, Height };

	std::vector<glm::vec3> objectPositions;
	const unsigned int NR_LIGHTS = 32;
	std::vector<glm::vec3> lightPositions;
	std::vector<glm::vec3> lightColors;

	_38_deferred_shading()
	{
		objectPositions.push_back(glm::vec3(-3.0, -3.0, -3.0));
		objectPositions.push_back(glm::vec3(0.0, -3.0, -3.0));
		objectPositions.push_back(glm::vec3(3.0, -3.0, -3.0));
		objectPositions.push_back(glm::vec3(-3.0, -3.0, 0.0));
		objectPositions.push_back(glm::vec3(0.0, -3.0, 0.0));
		objectPositions.push_back(glm::vec3(3.0, -3.0, 0.0));
		objectPositions.push_back(glm::vec3(-3.0, -3.0, 3.0));
		objectPositions.push_back(glm::vec3(0.0, -3.0, 3.0));
		objectPositions.push_back(glm::vec3(3.0, -3.0, 3.0));

		srand(glfwGetTime());
		for (unsigned int i = 0; i < NR_LIGHTS; i++)
		{
			// calculate slightly random offsets
			float xPos = ((rand() % 100) / 100.0) * 6.0 - 3.0;
			float yPos = ((rand() % 100) / 100.0) * 6.0 - 4.0;
			float zPos = ((rand() % 100) / 100.0) * 6.0 - 3.0;
			lightPositions.push_back(glm::vec3(xPos, yPos, zPos));
			// also calculate random color
			float rColor = ((rand() % 100) / 200.0f) + 0.5; // between 0.5 and 1.0
			float gColor = ((rand() % 100) / 200.0f) + 0.5; // between 0.5 and 1.0
			float bColor = ((rand() % 100) / 200.0f) + 0.5; // between 0.5 and 1.0
			lightColors.push_back(glm::vec3(rColor, gColor, bColor));
		}
	}

	void draw(glm::mat4& p, glm::mat4& v, glm::mat4& m)
	{
		gbuffer.enable(glm::vec3(0.f, 0.f, 0.f));
		geometryPass.enable();
		for (unsigned int i = 0; i < objectPositions.size(); i++)
		{
			glm::mat4 objectmodel = glm::mat4(1);
			objectmodel = glm::translate(objectmodel, objectPositions[i]);
			objectmodel = glm::scale(objectmodel, glm::vec3(0.25f));

			glm::mat4 MV = v * objectmodel;
			glm::mat3 NormalMV = glm::mat3(glm::transpose(glm::inverse(MV)));

			geometryPass.set_mat4("MV", MV);
			geometryPass.set_mat3("NormalMV", NormalMV);
			geometryPass.set_mat4("MVP", p * MV);
			model.draw(geometryPass);
		}

		// light pass
		gl_enable_framebuffer(0, Width, Height, glm::vec3(clear_color.x, clear_color.y, clear_color.z));
		LightPass.enable();
		glm::mat4 ortho0 = glm::ortho(0.f, 1.f, 0.f, 1.f);
		LightPass.set_mat4("MVP", ortho0);
		auto& texs = gbuffer.texs();
		LightPass.bind_texture("gPosition", 0, texs[0]);
		LightPass.bind_texture("gNormal", 1, texs[1]);
		LightPass.bind_texture("gAlbedoSpec", 2, texs[2]);

		for (unsigned int i = 0; i < lightPositions.size(); i++)
		{
			LightPass.set_vec3("lights[" + std::to_string(i) + "].Position", lightPositions[i]);
			LightPass.set_vec3("lights[" + std::to_string(i) + "].Color", lightColors[i]);
			// update attenuation parameters and calculate radius
			const float constant = 1.0; // note that we don't send this to the shader, we assume it is always 1.0 (in our case)
			const float linear = 0.7;
			const float quadratic = 1.8;
			LightPass.set_float("lights[" + std::to_string(i) + "].Linear", linear);
			LightPass.set_float("lights[" + std::to_string(i) + "].Quadratic", quadratic);
		}
		LightPass.set_vec3("viewPos", camera.Position);
		quad.draw();
		// render light

		// 2.5. copy content of geometry's depth buffer to default framebuffer's depth buffer
		// ----------------------------------------------------------------------------------
		glBindFramebuffer(GL_READ_FRAMEBUFFER, gbuffer.id());
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer
		// blit to default framebuffer. Note that this may or may not work as the internal formats of both the FBO and default framebuffer have to match.
		// the internal formats are implementation defined. This works on all of my systems, but if it doesn't on yours you'll likely have to write to the 		
		// depth buffer in another shader stage (or somehow see to match the default framebuffer's internal format with the FBO's internal format).
		glBlitFramebuffer(0, 0, Width, Height, 0, 0, Width, Height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// 3. render lights on top of scene
		// --------------------------------
		lightshader.enable();
		for (unsigned int i = 0; i < lightPositions.size(); i++)
		{
			glm::mat4 lightmodel = glm::mat4(1);
			lightmodel = glm::translate(lightmodel, lightPositions[i]);
			lightmodel = glm::scale(lightmodel, glm::vec3(0.125f));
			lightshader.set_mat4("MVP", p * v * lightmodel);
			lightshader.set_vec3("lightcolor", lightColors[i]);
			box.draw(lightshader);
		}
	}
};

// -----------------------------------------------------------------------------

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

	_38_deferred_shading object;

	Model light{ Box };
	Shader lightshader{ "learngl/light.vs", "learngl/light.fs" };

	//glEnable(GL_MULTISAMPLE);
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