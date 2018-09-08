#include "fay/core/fay.h"

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

// ·Ö±æÂÊ
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

// md
glm::vec3 model_scale(1.f);

// camera_
camera camera_(glm::vec3(0, 5, 10));;
float lastX = Width / 2.0f;
float lastY = Height / 2.0f;
bool firstMouse = true;

//light 
glm::vec3 lightPosition = glm::vec3(0, 10, 0); //objectspace light position
float light_speed = 2.f;
glm::vec3 light_scale(0.5f, 0.5f, 0.5f);

// Êó±êÒÆ¶¯ÉèÖÃÓëäÖÈ¾ÉèÖÃ
bool some_flag = false;
char mouse_move = 'z';
int render_state = 1;

// GUI
//background color£¬»á×Ô¶¯×ª»¯Îª 0.f~1.f µÄ¸¡µãÊý
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

	// Êó±êÒÆ¶¯
	float xpos = io.MousePos.x, ypos = io.MousePos.y;
	if (firstMouse) { lastX = xpos; lastY = ypos; firstMouse = false; }
	float xoffset = xpos - lastX; lastX = xpos;
	// reversed since y-coordinates go from bottom to top but z_xais form out to in
	float yoffset = lastY - ypos; lastY = ypos;

	// Õ³ÖÍ
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

		camera_.ProcessMouseMovement(xoffset, yoffset);
		if (io.KeysDown[GLFW_KEY_W]) camera_.ProcessKeyboard(FORWARD, deltaTime);
		if (io.KeysDown[GLFW_KEY_S]) camera_.ProcessKeyboard(BACKWARD, deltaTime);
		if (io.KeysDown[GLFW_KEY_A]) camera_.ProcessKeyboard(LEFT, deltaTime);
		if (io.KeysDown[GLFW_KEY_D]) camera_.ProcessKeyboard(RIGHT, deltaTime);
		//camera_.ProcessMouseScroll(io.MouseWheel); ½ûÖ¹·ÅËõ
	}
	else if (mouse_move == 'x')
	{
		// ÏòÇ°¹öÎªÕý£¬ÏòºóÎªºó£¬¸ù¾ÝËÙ¶È´Ó -5 ~ 5
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
	// ¼ÓÔØ¸²¸ÇÕû¸öÊÓ¿ÚµÄÕý·½ÐÎ
	vertex1 v1 = { 0.f, 0.f, 0.f };
	std::vector<vertex1> vb{ {-1, -1, 0}, {1, -1, 0}, {1, 1, 0}, {-1, 1, 0} };
	std::vector<uint32_t> ib{ 0,1,2,0,2,3 };
	buffer quad{ vb, ib };
	shader sd{ "learngl/00_gui.vs", "learngl/00_gui.fs" };
	texture2d diff{ "textures/awesomeface.png" };

	void draw(glm::mat4&& MVP)
	{
		sd.enable();
		sd.bind_texture("diff", 0, diff);
		sd.set_mat4("MVP", MVP);
		quad.draw();
	}
};

// -----------------------------------------------------------------------------

struct _20_load_mesh
{
	model md{ Nanosuit };
	shader sd{ "learngl/20_load_model.vs", "learngl/20_load_model.fs" };

	void draw(glm::mat4&& MVP)
	{
		sd.enable();
		sd.set_mat4("MVP", MVP);
		md.draw(sd);
	}
};

struct _21_load_model
{
	model md{ Nanosuit };
	shader sd{ "learngl/21_load_model.vs", "learngl/21_load_model.fs" };

	void draw(glm::mat4&& MVP)
	{
		sd.enable();
		sd.set_mat4("MVP", MVP);
		md.draw(sd);
	}
};

struct _22_depth_test
{
	model md{ Blocks };
	shader sd{ "learngl/22_depth_test.vs", "learngl/22_depth_test.fs" };

	void draw(glm::mat4&& MVP)
	{
		//glDepthMask(GL_FALSE);
		//glDepthFunc(GL_ALWAYS);
		sd.enable();
		sd.set_mat4("MVP", MVP);
		md.draw(sd);
	}
};

struct _23_stencil_test
{
	model md{ Box };
	shader sd{ "learngl/23_stencil_test.vs", "learngl/23_stencil_test.fs" };

	void draw(glm::mat4& p, glm::mat4& v, glm::mat4& m)
	{
		// TOOD：注�?
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

		glStencilFunc(GL_ALWAYS, 1, 0xFF); // ËùÓÐµÄÆ¬¶Î¶¼Ó¦¸Ã¸üÐÂÄ£°å»º³å
		glStencilMask(0xFF); // ÆôÓÃÄ£°å»º³åÐ´Èë
		sd.enable();
		sd.set_bool("draw_outlining", false);
		sd.set_mat4("MVP", p * v * m);
		md.draw(sd);
		
		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		glStencilMask(0x00); // ½ûÖ¹Ä£°å»º³åµÄÐ´Èë
		glDisable(GL_DEPTH_TEST);
		sd.enable();
		sd.set_bool("draw_outlining", true);
		glm::mat4 scaleModel = glm::scale(m, glm::vec3(1.1f, 1.1f, 1.1f));
		sd.set_mat4("MVP", p * v * scaleModel);
		md.draw(sd);
		glStencilMask(0xFF);
		glEnable(GL_DEPTH_TEST);
	}
};

struct _24_blending
{
	// plane
	std::vector<vertex1> planevb{ { -1, 0, 1 },{ 1, 0, 1 },{ 1, 0, -1 },{ -1, 0, -1 } };
	std::vector<uint32_t> planeib{ 0,1,2,2,3,0 };
	buffer planeQuad{ planevb, planeib };
	// grass
	std::vector<vertex1> vb{ { 0, 0, 0 },{ 1, 0, 0 },{ 1, 1, 0 },{ 0, 1, 0 } };
	std::vector<uint32_t> ib{ 0,1,2,2,3,0 };
	buffer quad{ vb, ib };

	shader sd{ "learngl/24_blending.vs", "learngl/24_blending.fs" };
	texture2d green{ "textures/grass.png" };
	texture2d marble{ "textures/marble.jpg" };
	texture2d window{ "textures/window.png" };

	void draw(glm::mat4& p, glm::mat4& v, glm::mat4&)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		sd.enable();
		glm::mat4 m(1.f);

		sd.bind_texture("diff", 0, marble);
		sd.set_bool("texture_xz", true);
		glm::mat4 m0 = glm::scale(m, glm::vec3(10.f, 10.f, 10.f));
		sd.set_mat4("MVP", p * v * m0);
		planeQuad.draw();

		glDisable(GL_CULL_FACE);
		sd.bind_texture("diff", 0, green);
		sd.set_bool("texture_xz", false);

		glm::mat4 m1 = glm::scale(m, glm::vec3(3.f, 3.f, 3.f));
		sd.set_mat4("MVP", p * v * m1);
		quad.draw();

		//glm::mat4 m2 = glm::translate(m, glm::vec3(-1, 0, 0));
		glm::mat4 m2 = glm::translate(m, glm::vec3(-3, 0, 0));
		m2 = glm::scale(m2, glm::vec3(3.f, 3.f, 3.f));
		sd.set_mat4("MVP", p * v * m2);
		quad.draw();
		
		sd.bind_texture("diff", 0, window);
		glm::mat4 m3 = glm::translate(m, glm::vec3(-0.5f, 0, 1));
		m3 = glm::scale(m3, glm::vec3(3.f, 3.f, 3.f));
		sd.set_mat4("MVP", p * v * m3);
		quad.draw();
		glEnable(GL_CULL_FACE);
	}
};

struct Box	// gamma_correction, 
{
	// quad
	std::vector<vertex1> vb{ { 0, 0, 0 },{ 1, 0, 0 },{ 1, 1, 0 },{ 0, 1, 0 } };
	std::vector<uint32_t> ib{ 0,1,2,2,3,0 };
	buffer quad{ vb, ib };

	texture2d green{ "textures/grass.png" };

	shader gui{ "learngl/post_processing.vs", "learngl/post_processing.fs" };

	void draw_gui(const base_texture& tex)
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
	std::vector<vertex1> vb{ { 0, 0, 0 },{ 1, 0, 0 },{ 1, 1, 0 },{ 0, 1, 0 } };
	std::vector<uint32_t> ib{ 0,1,2,2,3,0 };
	buffer quad{ vb, ib };

	texture2d green{ "textures/grass.png" };

	shader gui{ "learngl/post_processing.vs", "learngl/post_processing.fs" };

	void draw_gui(const base_texture& tex)
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
	model md{ Blocks };

	framebuffer fb{Width, Height};	// doesn't use mulitesample, need to modify code in the main()
	shader sd{ "learngl/25_framebuffers.vs", "learngl/25_framebuffers.fs" };

	void draw(glm::mat4& p, glm::mat4& v, glm::mat4& m)
	{
		fb.enable(glm::vec3(0.f, 0.f, 0.f));
		sd.enable();
		sd.set_mat4("MVP", p * v * m);
		md.draw(sd);

		gl_enable_framebuffer(0, Width, Height, glm::vec3(clear_color.x, clear_color.y, clear_color.z));
		draw_gui(fb.tex());
	}
};

struct _26_cubemaps
{
	model md{ Nier_2b };
	model box{ Box };

	const string files[7] = { "skybox/blue_sky/",
		"right.jpg", "left.jpg",
		"top.jpg", "bottom.jpg",
		"back.jpg", "front.jpg" };
	texture_cube sky{files};

	shader sd{ "learngl/26_environment_map.vs", "learngl/26_environment_map.fs" };
	shader cubemap{ "learngl/26_cubemaps.vs", "learngl/26_cubemaps.fs" };

	void draw(glm::mat4& p, glm::mat4& v, glm::mat4& m)
	{
		sd.enable();

		glm::mat4 MV = v * m;
		glm::mat3 NormalMV = glm::mat3(glm::transpose(glm::inverse(MV)));
		sd.enable();
		sd.set_mat4("MV", MV);
		sd.set_mat3("NormalMV", NormalMV);		// Ê§È¥Î»ÒÆÊôÐÔ
		sd.set_mat4("MVP", p * v * m);
		md.draw(sd);

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
	model md{ Box };
	unifrom ubo{ 128 };	// 2 * mat4, mat4 = 4 * vec4 = 16 * float = 64 * bytes
	shader shader1{ "learngl/27_uniform.vs", "learngl/27_uniform.fs" };
	shader shader2{ "learngl/27_uniform.vs", "learngl/27_uniform.fs" };

	void draw(glm::mat4& p, glm::mat4& v, glm::mat4& m)
	{
		// struct Mat { mat4 p; mat v; }
		// uniform.set(OFFSET(p), sizeof(v), &v);
		CHECK(sizeof(glm::mat4) == 64);
		ubo.set(0, 64, glm::value_ptr(p));
		ubo.set(64, 64, glm::value_ptr(v));

		shader1.enable();
		shader1.set_mat4("model", m);
		shader1.bind_uniform("Mat", 0, ubo);
		shader1.set_bool("inverse", false);
		md.draw(shader1);

		shader2.enable();
		shader2.set_mat4("model", glm::translate(m, lightPosition));
		shader2.bind_uniform("Mat", 0, ubo);
		shader2.set_bool("inverse", true);
		md.draw(shader2);
	}
};

struct _28_geometry_shader
{
	float time{};
	model md{ Fairy };
	shader bomb{ "learngl/28_geometry_shader.vs", "learngl/28_gs_bomb.fs", "learngl/28_gs_bomb.gs" };
	shader normal{ "learngl/28_geometry_shader.vs", "learngl/28_gs_normal.fs", "learngl/28_gs_normal.gs" };

	void draw(glm::mat4& p, glm::mat4& v, glm::mat4& m)
	{
		bomb.enable();
		bomb.set_mat4("MVP", p * v * m);
		if (mouse_move != 'z')
		{
			time += deltaTime / 10;
			bomb.set_float("time", time);
		}
		md.draw(bomb);

		normal.enable();
		normal.set_mat3("NormalMV", glm::mat3(glm::transpose(glm::inverse(v * m))));
		normal.set_mat4("P", p);
		normal.set_mat4("MVP", p * v * m);
		normal.set_float("Length", 1.2f);
		md.draw(normal);
	}
};

struct _29_instancing
{
	model planet{ Planet };
	model rock{ Rock };
	shader s1{ "learngl/21_load_model.vs", "learngl/21_load_model.fs" };
	shader s2{ "learngl/29_instancing.vs", "learngl/29_instancing.fs" };
	// Ê¹ÓÃ¶ÀÏÔÔËÐÐ
	const int amount = 100000;
	std::vector<glm::mat4> mat4s{ amount };

	_29_instancing()
	{
		srand(glfwGetTime());
		float radius = 100.0;
		float offset = 25.0f;

		for (int i = 0; i < amount; ++i)
		{
			glm::mat4 m{ 1 };
			// 1. translation: displace along circle with 'radius' in range [-offset, offset]
			float angle = (float)i / (float)amount * 360.0f;
			float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
			float x = sin(angle) * radius + displacement;
			displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
			float y = displacement * 0.4f; // keep height of asteroid field smaller compared to width of x and z
			displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
			float z = cos(angle) * radius + displacement;
			m = glm::translate(m, glm::vec3(x, y, z));

			// 2. scale: Scale between 0.05 and 0.25f
			float scale = (rand() % 20) / 100.0f + 0.05;
			m = glm::scale(m, glm::vec3(scale));

			// 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector
			float rotAngle = (rand() % 360);
			m = glm::rotate(m, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

			// 4. now add to list of matrices
			mat4s[i] = m;
		}

		unsigned int instanceVBO;
		glGenBuffers(1, &instanceVBO);
		glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
		glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), mat4s.data(), GL_STATIC_DRAW);

		for (unsigned int i = 0; i < rock.meshes.size(); ++i)
		{
			glBindVertexArray(rock.meshes[i].buf.id());

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
		s1.enable();
		s1.set_mat4("MVP", p * v * m);
		planet.draw(s1, amount);
		
		s2.enable();
		s2.set_mat4("PV", p * v);
		rock.draw(s2, amount);
	}
};

struct _2a_anti_aliasing : public Post_Processing
{
	model md{ Blocks };

	multisample_framebuffer fb{ Width, Height };	// use mulitesample

	shader sd{ "learngl/25_framebuffers.vs", "learngl/25_framebuffers.fs" };

	void draw(glm::mat4& p, glm::mat4& v, glm::mat4& m)
	{
		fb.enable(glm::vec3(0.f, 0.f, 0.f));
		sd.enable();
		sd.set_mat4("MVP", p * v * m);
		md.draw(sd);

		fb.blit();

		gl_enable_framebuffer(0, Width, Height, glm::vec3(clear_color.x, clear_color.y, clear_color.z));
		draw_gui(fb.tex());
	}
};

// -----------------------------------------------------------------------------

struct _30_phong_shading
{
	model md{ Nier_2b };
	shader sd{ "learngl/30_phong_shading.vs", "learngl/30_phong_shading.fs" };
	// shader sd{ "learngl/light.vs", "learngl/light.fs" };

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

		sd.enable();
		sd.set_bool("blinn_phong", some_flag);
		sd.set_mat4("MV", MV);
		//sd.set_mat4("NorMV", NormalMV);	// Ð¡ÐÄ´«Êä×ÅÉ«Æ÷±äÁ¿¡¢ÏÈ±£´æÔÙ±àÒë
		sd.set_mat3("NormalMV", NormalMV);		// Ê§È¥Î»ÒÆÊôÐÔ
		sd.set_mat4("MVP", p * v * m);
		sd.set_vec3("vLightPos", glm::vec3(v * glm::vec4(lightPosition, 1.f)));
		sd.set_vec3("Lightcolor", glm::vec3(light_color.x, light_color.y, light_color.z));
		sd.set_float("sa", sAmbient);
		sd.set_float("sd", sDiffuse);
		sd.set_float("ss", sSpeclar);
		sd.set_int("shininess", shininess);
		md.draw(sd);
		// md.draw();
		gl_check_errors();
	}
};

struct _31_light_caster
{
	model md{ Blocks };
	shader sd{ "learngl/31_light_caster.vs", "learngl/31_light_caster.fs" };
	// shader sd{ "learngl/light.vs", "learngl/light.fs" };

	float sAmbient = 1.f;
	float sDiffuse = 1.f;
	float sSpeclar = 1.f;
	int   shininess = 32;

	void draw(glm::mat4& p, glm::mat4& v, glm::mat4& m)
	{
		// TODO: bind imgui to sd
		ImGui::SliderFloat("ambient strength", &sAmbient, 0.f, 10.f);
		ImGui::SliderFloat("diffuse strength", &sDiffuse, 0.f, 100.f);
		ImGui::SliderFloat("speclar strength", &sSpeclar, 0.f, 10.f);
		ImGui::SliderInt("shininess", &shininess, 1, 256);

		glm::mat4 MV = v * m;
		glm::mat3 NormalMV = glm::mat3(glm::transpose(glm::inverse(MV)));

		sd.enable();
		sd.set_mat4("MV", MV);
		//sd.set_mat4("NorMV", NormalMV);	// Ð¡ÐÄ´«Êä×ÅÉ«Æ÷±äÁ¿¡¢ÏÈ±£´æÔÙ±àÒë
		sd.set_mat3("NormalMV", NormalMV);		// Ê§È¥Î»ÒÆÊôÐÔ
		sd.set_mat4("MVP", p * v * m);

		// directlight
		sd.set_vec3("dLight.direct", glm::vec3(-1.f, -1.f, -1.f));
		sd.set_vec3("dLight.color", glm::vec3(light_color.x, light_color.y, light_color.z));
		// pointlight
		sd.set_vec3("pLight.pos", glm::vec3(v * glm::vec4(lightPosition, 1.f)));
		sd.set_vec3("pLight.color", glm::vec3(light_color.x, light_color.y, light_color.z));
		sd.set_vec3("pLight.falloff", glm::vec3(1.0, 0.022, 0.0019));	// 100 ¸öµ¥Î»
		// spotlight
		sd.set_vec3("sLight.pos", camera_.Position);
		sd.set_vec3("sLight.direct", camera_.Front);
		sd.set_vec3("sLight.color", glm::vec3(light_color.x, light_color.y, light_color.z));
		sd.set_vec2("sLight.cutoff", glm::vec2(
			glm::cos(glm::radians(10.f)), glm::cos(glm::radians(15.f)) ));
		sd.set_vec3("sLight.falloff", glm::vec3(1.0, 0.022, 0.0019));	// 100 ¸öµ¥Î»
		
		sd.set_float("sa", sAmbient);
		sd.set_float("sd", sDiffuse);
		sd.set_float("ss", sSpeclar);
		sd.set_int("shininess", shininess);
		md.draw(sd);
		// md.draw();
		gl_check_errors();
	}
};

struct _32_shadow_map : public Post_Processing
{
	model md{ Blocks };
	shader sd{ "learngl/32_shadow_map.vs", "learngl/32_shadow_map.fs" };
	shader shadowModel{ "learngl/32_shadow_model.vs", "learngl/32_shadow_model.fs" };
	shadowmap_framebuffer smfb;

	void draw(glm::mat4& p, glm::mat4& v, glm::mat4& m)
	{
		GLfloat near_plane = 1.0f, far_plane = 512.f;

		// Ê¹ÓÃÕý½»Ïà»úÊ±£¬Éî¶ÈÊÇÏßÐÔµÄ
		glm::mat4 lightOrtho = glm::ortho(-512.0f, 512.0f, -512.0f, 512.0f, near_plane, far_plane);
		// Ê¹ÓÃÍ¶Ó°Ïà»úÊ±£¬Ö»ÓÐ½Ó½ü½üÆ½ÃæµÄµØ·½Ð§¹û±È½ÏºÃ
		glm::mat4 lightProj = glm::perspective(glm::radians(120.f),
			1.f / 1.f, 10.f, 512.f);
		
		glm::mat4 lightView = glm::lookAt(
			lightPosition, glm::vec3(0.0f), glm::vec3(0.f, 1.f, 0.f));
		glm::mat4 lightSpace = lightProj * lightView;
		
		glCullFace(GL_FRONT);
		smfb.enable(glm::vec3(0.f, 0.f, 0.f));
		sd.enable();
		sd.set_mat4("MVP", lightSpace * m);
		md.draw(sd);
		glCullFace(GL_BACK);

		gl_enable_framebuffer(0, Width, Height, glm::vec3(clear_color.x, clear_color.y, clear_color.z));
		shadowModel.enable();
		shadowModel.set_mat4("Proj", p);
		shadowModel.set_mat4("View", v);
		shadowModel.set_mat4("Model", m);
		shadowModel.set_mat4("LightSpace", lightSpace);
		shadowModel.set_vec3("LightPos", lightPosition);
		shadowModel.set_vec3("ViewPos", camera_.Position);
		shadowModel.bind_texture("Shadowmap", 3, smfb.tex());
		md.draw(shadowModel);
	}
};

struct Light_Parameter
{
	float sAmbient = 0.2f;
	float sDiffuse = 1.f;
	float sSpeclar = 0.f;
	int   shininess = 32;

	void set(glm::mat4& p, glm::mat4& v, glm::mat4& m, shader& sd)
	{
		if (ImGui::Button("normal map"))
			some_flag ^= 1;
		if (some_flag)
			ImGui::Text("use_normal_map");

		ImGui::SliderFloat("ambient strength", &sAmbient, 0.f, 10.f);
		ImGui::SliderFloat("diffuse strength", &sDiffuse, 0.f, 10.f);
		ImGui::SliderFloat("speclar strength", &sSpeclar, 0.f, 10.f);
		ImGui::SliderInt("shininess", &shininess, 1, 256);

		sd.enable();
		sd.set_mat4("Model", m);
		sd.set_mat4("MVP", p * v * m);
		sd.set_vec3("LightPos", lightPosition);
		sd.set_vec3("ViewPos", camera_.Position);

		sd.set_bool("use_normal_map", some_flag);
		sd.set_float("sa", sAmbient);
		sd.set_float("sd", sDiffuse);
		sd.set_float("ss", sSpeclar);
		sd.set_int("shininess", shininess);
	}
};

struct _34_normal_map : public Light_Parameter
{
	model md{ Nier_2b };
	shader sd{ "learngl/34_normal_map.vs", "learngl/34_normal_map.fs" };

	void draw(glm::mat4& p, glm::mat4& v, glm::mat4& m)
	{
		set(p, v, m, sd);
		md.draw(sd);
	}
};

struct _35_parallax_map : public Light_Parameter
{
	model md{ Box };
	// TODO£º¸Ä½ø·½·¨
	shader sd{ "learngl/34_normal_map.vs", "learngl/35_parallax_map.fs" };

	texture2d diffuse{ "textures/bricks2.jpg" };
	texture2d normal{  "textures/bricks2_normal.jpg" };
	texture2d height{  "textures/bricks2_disp.jpg" };

	float heightScale = 0.1;

	void draw(glm::mat4& p, glm::mat4& v, glm::mat4& m)
	{
		ImGui::SliderFloat("heightScale", &heightScale, 0.f, 1.f);

		set(p, v, m, sd);
		sd.bind_texture("Diffuse", 0, diffuse);
		sd.bind_texture("Normal", 1, normal);
		sd.bind_texture("Depth", 2, height);
		sd.set_float("heightScale", heightScale);
		md.draw();
	}
};

struct _38_deferred_shading
{
	// quad
	std::vector<vertex1> vb{ { 0, 0, 0 },{ 1, 0, 0 },{ 1, 1, 0 },{ 0, 1, 0 } };
	std::vector<uint32_t> ib{ 0,1,2,2,3,0 };
	buffer quad{ vb, ib };

	model md{ Nanosuit };
	model box{ Box };
	shader geometryPass{ "learngl/30_phong_shading.vs", "learngl/38_g_buffer.fs" };
	shader LightPass{ "learngl/post_processing.vs", "learngl/38_deferred_shading.fs" };
	shader lightshader{ "learngl/light.vs", "learngl/light.fs" };
	gbuffer_framebuffer gbuffer{ Width, Height };

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
			md.draw(geometryPass);
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
		LightPass.set_vec3("viewPos", camera_.Position);
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
	model md{ Blocks };
	shader sd{ "learngl/xxxxxxxxxxxx.vs", "learngl/xxxxxxxxxxxxxx.fs" };

	void draw(glm::mat4& p, glm::mat4& v, glm::mat4& m)
	{
		sd.enable();
		sd.set_mat4("MVP", p * v * m);
		md.draw(sd);
	}
};

struct _fay_obj_model
{
	objmodel md{ Fairy };
	// shader sd{ "learngl/31_load_model.vs", "learngl/31_load_model.fs" };
	shader sd{ "learngl/light.vs", "learngl/light.fs" };

	void draw(glm::mat4&& MVP)
	{
		sd.enable();
		sd.set_mat4("MVP", MVP);
		sd.set_vec3("lightcolor", glm::vec3(1, 1, 1));
		// md.draw(sd);
		md.draw();
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

	model light{ Box };
	shader lightshader{ "learngl/light.vs", "learngl/light.fs" };

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
		glm::mat4 view = camera_.GetViewMatrix();
		glm::mat4 projection = glm::perspective(glm::radians(camera_.Zoom),
			(float)Width / (float)Height, 0.1f, 10000.0f);

		// draw
		glm::mat4 m(1.f);
		m = glm::scale(m, model_scale);
		object.draw(projection, view, m);

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