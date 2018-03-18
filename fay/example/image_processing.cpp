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

// clear
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
Camera camera(glm::vec3(0, 0, 10));;
float lastX = Width / 2.0f;
float lastY = Height / 2.0f;
bool firstMouse = true;

// 鼠标移动设置与渲染设置
glm::vec3 mouse_offset(0.f);
char mouse_state = 'z';

// image_processing
int processing_mode = 0;
float ip_gamma = 1.f;
bool save_image = false;
bool have_save = false;

// GUI
//background color，会自动转化为 0.f~1.f 的浮点数
static ImVec4 clear_color = ImColor(0, 0, 0);
static ImVec4 light_color = ImColor(255, 255, 255);

// -----------------------------------------------------------------------------

void update()
{
	gui_updateIO();

	currentFrame = glfwGetTime();
	deltaTime = (currentFrame - lastFrame) * 10;
	lastFrame = currentFrame;

	ImGuiIO& io = gui_get_io();

	if (io.KeysDown[GLFW_KEY_S]) save_image = true;

	if (io.KeysDown[GLFW_KEY_1]) processing_mode = 1;
	if (io.KeysDown[GLFW_KEY_2]) processing_mode = 2;
	if (io.KeysDown[GLFW_KEY_3]) processing_mode = 3;

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

		if (io.MouseDown[0])
		{
			mouse_offset.x += xoffset;
			mouse_offset.y += yoffset;
		}
	}
	else
	{
		// GUI
	}
}

void histogram(const string& imgpath, const string& filename)
{
	Image img{ imgpath };
	auto data = img.data();

	std::vector<float> v(256, 0.f);

	auto width = img.width(), height = img.height(), pixels = width * height;
	// RGB
	for (int i = 0; i < pixels; ++i, data += 3)
		v[*data] += 1.f;

	// 0.0 ~ 1.0
	float max{};
	for (auto& i : v)
	{ 
		i /= pixels;
		if (i > max)
			max = i;
	}
	//max += 0.1;

	std::ofstream ppm(filename);
	CHECK(!ppm.fail()) << "\ncan't create file";

	int w = 2, h = 256;
	ppm << "P3\n" << w * 256 << " " << h << "\n255\n";

	//char r{}, g{}, b{};
	//uint8_t r{}, g{}, b{};
	int r{}, g{}, b{};
	for (int y = 0; y < h; ++y)
	{
		float level = (float)(h - y) / h * max;
		for (int x = 0; x < 256; ++x)
		{
			if (v[x] >= level)
				r = g = b = 255;
			else
				r = g = b = 0;

			for (int i = 0; i < w; ++i)
				ppm << r << " " << g << " " << b << " ";
		}
		ppm << std::endl;
	}
}

void gamma_transformation(const string& imgpath, const string& filename, float gamma)
{
	Image img{ imgpath };
	auto data = img.data();
	auto width = img.width(), height = img.height(), pixels = width * height;

	//std::vector<uint8_t> v(data, data + pixels * 3);
	vector<uint8_t> v(data, data + pixels * 3);

	int i{};
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x, i += 3)
		{
			//v[i + 0] = (uint8_t)std::pow((float)v[i + 0], gamma);
			//v[i + 1] = (uint8_t)std::pow((float)v[i + 1], gamma);
			//v[i + 2] = (uint8_t)std::pow((float)v[i + 2], gamma);

			for (int j = 0; j < 3; ++j)
			{
				float c = (float)v[i + j] / 256;
				c = pow(c, gamma);
				v[i + j] = (uint8_t)(c * 256);
			}
		}
	}

	save_ppm(filename, v.data(), width, height);
}

void median_filter(const string& imgpath, const string& filename)
{
	Image img{ imgpath };
	auto data = img.data();
	auto width = img.width(), height = img.height(), pixels = width * height;

	// grey
	constexpr int n = 3;
	uint8_t f[9];
	vector<uint8_t> v(pixels);

	int m{};
	for (size_t y = 0; y < height; ++y)
	{
		for (size_t x = 0; x < width; ++x, ++m)
		{
			for (size_t i = 0; i < 3; ++i)
			{
				f[0 + i] = img(x - 1 + i, y - 1);
				f[3 + i] = img(x - 1 + i, y - 0);
				f[6 + i] = img(x - 1 + i, y + 1);
				sort(f, f + 9);
				v[m] = f[4];
			}
		}
	}

	save_pgm(filename, v.data(), width, height);
}

void image_processing(const std::string& imgpath)
{
	Texture2D img{ imgpath };

	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	// 为当前绑定的纹理对象设置环绕、过滤方式
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// 加载并生成纹理
	int wwidth, wheight, nrChannels;
	unsigned char *data = stbi_load("image/7.jpg", &wwidth, &wheight, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, wwidth, wheight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	uint32_t width = img.width(), height = img.height();
	float ratio = (float)height / (float)width;

	// quad
	// left, right, bottom, top
	glm::vec4 p = { -1.f, 1.f, -ratio, ratio };
	std::vector<Vertex1> vb{ { p.x, p.z, 0 },{ p.y, p.z, 0 },{ p.y, p.w, 0 },{ p.x, p.w, 0 } };
	std::vector<uint32_t> ib{ 0,1,2,2,3,0 };
	Buffer quad{ vb, ib };

	//Framebuffer fb{ width, height };
	//Shader ip{ "image_processing/processing.vs", "image_processing/processing.fs" };
	Shader gui{ "image_processing/gui.vs", "image_processing/gui.fs" };

	//glViewport(0, 0, width, height);
	while (!gui_close_window())
	{
		update();

		glm::mat4 ortho0 = glm::ortho(-1.f, 1.f, -ratio, ratio);
		float wratio = (float)Height / (float)Width;
		glm::mat4 ortho1 = glm::ortho(-1.f, 1.f, -wratio, wratio);
		glm::mat4 model0(1.f), model1(1.f);
		/*
		fb.enable(glm::vec3(1.f, 0.f, 0.f));
		ip.enable();
		ip.set_mat4("MVP", ortho0 * model0);
		ip.bind_texture("diff", 0, img.id());
		ip.set_int("processing_mode", processing_mode);
		ip.set_float("gamma", ip_gamma);
		quad.draw();
		*/
		//gl_enable_framebuffer(0, Width, Height, glm::vec3(clear_color.x, clear_color.y, clear_color.z));
		glClearColor(clear_color.x, clear_color.y, clear_color.z, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		gui.enable();
		model1 = glm::translate(model1,
			glm::vec3(mouse_offset.x / Width, mouse_offset.y / Height, 0));
		model1 = glm::scale(model1, model_scale);
		gui.set_mat4("MVP", ortho1 * model1);
		gui.bind_texture("diff", 0, texture);
		quad.draw();

		// GUI
		ImGui::Text("current processing mode: %d", processing_mode);
		ImGui::SliderFloat("gamma", &ip_gamma, 0.f, 25.f);
		ImGui::Text("Info");
		//ImGui::Text("mouse move: %c", mouse_state);
		ImGui::ColorEdit3("clear color", (float*)&clear_color);
		ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		//ImGui::SliderInt("samples_Perpixel", &samples_PerPixel, 1, 16);
		gui_drawGUI();
		/*
		if (save_image)
		{
			save_image = false;

			if(!have_save)
			{
				std::cout << "save" << std::endl;
				std::vector<uint8_t> data(width * height * 3);	//RGB
				//glBindTexture(GL_TEXTURE_2D, fb.tex_id());
				glBindFramebuffer(GL_FRAMEBUFFER, fb.fbo_id());
				glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, data.data());
				save_ppm("test.ppm", data.data(), width, height);
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
			}
			have_save = true;
		}
		*/
	}
}

int main(int argc, char** argv)
{
	//if (argc < 2)
	//{
	//	std::cout << "no input\n";
	//	return 0;
	//}
	google::InitGoogleLogging(argv[0]);
	gui_create_window(Width, Height);

	image_processing("image/2.png");
	//image_processing(argv[1]);
	//histogram("image/1.png", "1.ppm");
	//gamma_transformation("image/3.png", "3.ppm", 3.0);
	//median_filter("image/4.jpg", "4.pgm");
	gui_delete_window();
	return 0;
}