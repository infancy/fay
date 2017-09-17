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
Camera camera(glm::vec3(-3, 3, 10));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float currentFrame = 0.f;
float deltaTime = 0.f;
float lastFrame = 0.f;

////////////////////////////////////////////////////////////////////////////////////

// 鼠标移动设置与渲染设置
int move_state = 0;
int render_state = 1;

const string blocks = "resources/objects/blocks/blocks.obj";
const string Rei = "resources/objects/Rei/Rei.obj";
const string CornellBox = "resources/objects/CornellBox/CornellBox.obj";
const string rock = "resources/objects/rock/rock.obj";
const string mesh_filename = CornellBox;

//background color
glm::vec4 bg = glm::vec4(0.5, 0.5, 1, 1);

//scene axially aligned bounding box
struct BBox { glm::vec3 min, max; } aabb{{-1000, -1000, -1000}, {1000, 1000, 1000}};

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

void obj_to_TextureData(const string& obj_path,
	vector<float>& positions, vector<uint32_t>& indices, vector<string>& texpaths)
{
	std::ifstream is(obj_path);
	CHECK(!is.fail());

	int total_positions{};
	uint32_t tex_index{};		// 纹理在纹理数组 texpaths 中的下标
	map<string, string> materila_texture;	// 材质与其对应的漫反射纹理

	string line;
	float x, y, z;
	uint32_t i, j, k, l;
	char dummy;

	while (getline(is, line))
	{
		if (line.length() < 2) continue;

		istringstream iss(line);
		string token;
		iss >> token;

		if (token.compare("v") == 0)		// load postion. "v -0.983024 -0.156077 0.0964607"
		{
			iss >> x >> y >> z;
			positions.push_back(x); positions.push_back(y); positions.push_back(z); positions.push_back(0.f);
			++total_positions;
		}
		else if (token.compare("f") == 0)
		{
			if (line.find("//") != std::string::npos)	// pos//normal, no uv. "f 181//176 182//182 209//208"
			{
				iss >>
					i >> dummy >> dummy >> l >>
					j >> dummy >> dummy >> l >>
					z >> dummy >> dummy >> l;
			}
			else	
			{
				size_t count = 0, pos = line.find('/');
				while (pos != std::string::npos) { count++; pos = line.find('/', pos + 1); }
				if ((count == 6) || (count == 8))		// "f 181/292/176 182/250/182 209/210/208"
				{
					iss >>
						i >> dummy >> l >> dummy >> l >>
						j >> dummy >> l >> dummy >> l >>
						k >> dummy >> l >> dummy >> l;
				}
				else if ((count == 3) || (count == 4))	// pos/uv, no normal. "f 181/176 182/182 209/208"
				{
					iss >>
						i >> dummy >> l >>
						j >> dummy >> l >>
						k >> dummy >> l;
				}
				else					// pos, no uv/normal. "f 181 182 209"
					iss >> i >> j >> k;
			}

			// deal with negative index
			if (i < 0) i = i + total_positions + 1;
			if (j < 0) j = j + total_positions + 1;
			if (k < 0) k = k + total_positions + 1;

			indices.push_back(i - 1);	// obj 模型中索引下标从一开始，而 OpenGL 中则从零开始
			indices.push_back(j - 1);
			indices.push_back(k - 1);
			indices.push_back(tex_index);

			if (!iss.eof())	// 若还未到达这一行的结尾，则这是一个长方形的面，分拆成两个三角形
			{
				iss >> l;	
				if (l < 0) l = l + total_positions + 1;
				indices.push_back(i - 1);
				indices.push_back(k - 1);
				indices.push_back(l - 1);
				indices.push_back(tex_index);
			}
		}
		else if (token.compare("usemtl") == 0)
		{
			// 更新 tex_index
			string material_name;
			iss >> material_name;
			material_name = line.substr(line.find(material_name));
			if (materila_texture.find(material_name) == materila_texture.end())	// 如果该材质没有漫反射纹理
				tex_index = 255;
			else
			{
				auto tex_filepath = materila_texture[material_name];
				for (tex_index = 0; tex_index < texpaths.size(); ++tex_index)
					if (texpaths[tex_index] == tex_filepath)
						break;
			}
		}
		else if (token.compare("mtllib") == 0)
		{
			string mtl_path, mtl_line;
			string material_name, texture_name;

			iss >> mtl_path;
			mtl_path = line.substr(line.find(mtl_path));	//deal with "mtllib Rei Ayanami School Clothes.mtl"
			ifstream mtl_file(obj_path.substr(0, obj_path.find_last_of("/") + 1) + mtl_path);
			CHECK(!mtl_file.fail());

			while (getline(mtl_file, mtl_line))
			{
				if (line.length() < 2) continue;

				istringstream mtl_iss(mtl_line);
				string mtl_token;
				mtl_iss >> mtl_token;

				if (mtl_token.compare("newmtl") == 0)
				{ 
					mtl_iss >> material_name;
					material_name = mtl_line.substr(mtl_line.find(material_name));
				}
				else if (mtl_token.compare("map_Kd") == 0)
				{
					mtl_iss >> texture_name;
					texture_name = mtl_line.substr(mtl_line.find(texture_name));
					materila_texture.insert({ material_name, texture_name });	// 添加新的“材质-纹理”对
				}
			}
			// 不重复的向纹理数组中添加纹理
			for(auto& mat_tex : materila_texture)
				if (std::find(texpaths.begin(), texpaths.end(), mat_tex.second) == texpaths.end())
					texpaths.push_back(mat_tex.second);
		}
		//else if (line[0] == 'g') ;	// do nothing
	}
}

void update_lightPos()
{
	lightPosition.x = ligth_radius * cos(theta)*sin(phi);
	lightPosition.y = ligth_radius * cos(phi);
	lightPosition.z = ligth_radius * sin(theta)*sin(phi);
}

void ray_cast()
{
	create_window();

	// 加载用于光栅化的模型
	Model model(mesh_filename);

	// 加载覆盖整个视口的正方形
	std::vector<float> quadVerts{ -1, -1, 0, 1, -1, 0, 1, 1, 0, -1, 1, 0};
	std::vector<uint32_t> quadIndices{ 0,1,2,0,2,3 };
	Buffer quad(quadVerts, quadIndices);

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

	// 加载光线跟踪中的数据
	vector<float> positions; vector<uint32_t> indices; vector<string> texpaths;
	obj_to_TextureData(mesh_filename, positions, indices, texpaths);

	std::string mesh_path = mesh_filename.substr(0, mesh_filename.find_last_of("/") + 1);
	TextureArray material_arrays(texpaths, mesh_path);

	// 将位置、索引打包成一维纹理传入片元着色器
	TextureData<float> texVertices(GL_RGBA32F, positions.size() / 4, GL_RGBA, GL_FLOAT, positions);

	TextureData<uint32_t> texTriangles(GL_RGBA32UI, indices.size() / 4, GL_RGBA_INTEGER, GL_UNSIGNED_INT, indices);

	// 加载着色器
	Shader flatShader("raytracing/flat.vert", "raytracing/flat.frag");

	Shader rasterShader("raytracing/shader.vert", "raytracing/shader.frag");

	Shader raytraceShader("raytracing/raycast.vert", "raytracing/raycast.frag");
	raytraceShader.enable();
	raytraceShader.set_float("VERTEX_TEXTURE_SIZE", (float)(positions.size() / 4));
	raytraceShader.set_float("TRIANGLE_TEXTURE_SIZE", (float)(indices.size() / 4));
	raytraceShader.set_vec3("aabb.min", aabb.min);
	raytraceShader.set_vec3("aabb.max", aabb.max);
	raytraceShader.set_vec4("backgroundColor", bg);
	raytraceShader.disable();

	Shader pathtraceShader("raytracing/pathtrace.vert", "raytracing/pathtrace.frag");
	pathtraceShader.enable();
	pathtraceShader.set_float("VERTEX_TEXTURE_SIZE", (float)(positions.size() / 4));
	pathtraceShader.set_float("TRIANGLE_TEXTURE_SIZE", (float)(indices.size() / 4));
	pathtraceShader.set_vec3("aabb.min", aabb.min);
	pathtraceShader.set_vec3("aabb.max", aabb.max);
	pathtraceShader.set_vec4("backgroundColor", bg);
	pathtraceShader.disable();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glClearColor(bg.r, bg.g, bg.b, 1.0f);

	while (!glfwWindowShouldClose(window))
	{
		update();
		update_lightPos();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 1.f, 10000.0f);
		glm::mat4 view = camera.GetViewMatrix();
		//glm::mat4 modelMat = glm::scale(glm::mat4(1.f), glm::vec3(10.f, 10.f, 10.f));
		glm::mat4 modelMat(1.f);

		glm::mat4 MV = view * modelMat;
		glm::mat4 invMV = glm::inverse(MV);
		glm::mat4 MVP = projection * MV;
		glm::mat4 invMVP = glm::inverse(MVP);

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

			pathtraceShader.set_float("time", currentFrame);
			raytraceShader.set_vec3("eyePos", camera.Position);
			raytraceShader.set_mat4("invMVP", invMV);
			raytraceShader.set_vec3("light_position", lightPosition);

			quad.draw();	// 渲染整个视口
		}
		else if (render_state == 3)
		{
			pathtraceShader.enable();

			pathtraceShader.bind_texture("textureMaps", 0, material_arrays.id());
			pathtraceShader.bind_texture("vertex_positions", 1, texVertices.id());
			pathtraceShader.bind_texture("triangles_list", 2, texTriangles.id());

			pathtraceShader.set_vec3("eyePos", camera.Position);
			pathtraceShader.set_mat4("invMVP", invMV);
			pathtraceShader.set_vec3("light_position", lightPosition);

			quad.draw();	// 渲染整个视口
		}

		// 渲染小十字
		//disable depth testing
		glDisable(GL_DEPTH_TEST);

		//draw the light gizmo, set the light vertexx array object
		glBindVertexArray(lightVAOID);
		{
			//set the modelling transform for the light crosshair gizmo
			glm::mat4 T = glm::translate(glm::mat4(1), lightPosition);
			//bind the shader
			flatShader.enable();
			//set shader uniforms and draw lines
			//P*MV*T
			flatShader.set_mat4("MVP", projection * view * T);
			glDrawArrays(GL_LINES, 0, 6);

			glPointSize(2.0);
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
	currentFrame = glfwGetTime();
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