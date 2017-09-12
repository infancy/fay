#include "raytracing.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

/*
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>
*/
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>

#include "fay.h"

#include "gl/buffer.h"
#include "gl/texture.h"
//#include "gl/model.h"
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
Camera camera(glm::vec3(0.0f, 50.0f, 200.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

void stencil()
{
	create_window();

	glEnable(GL_DEPTH_TEST);

	//Shader ourShader("raytracing/raytracing.vert", "raytracing/raytracing.frag");

	//Model ourModel("resources/objects/nanosuit/nanosuit.obj");

	while (!glfwWindowShouldClose(window))
	{
		update();

		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//ourShader.use();
		//ourModel.Draw(ourShader);

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	destroy_window();
}

//////////////////////////////////////////////////////

//Shader shader, raytraceShader, flatShader;

//IDs for vertex array and buffer object
GLuint vaoID;
GLuint vboVerticesID;
GLuint vboIndicesID;

//projection and modelview matrices
glm::mat4  P = glm::mat4(1);
glm::mat4 MV = glm::mat4(1);

//Objloader instance
vector<tmp::Mesh*> meshes;					//all meshes 
vector<tmp::Material*> materials;			//all materials 
vector<unsigned short> indices;			//all mesh indices 
vector<tmp::Vertex> vertices;				//all mesh vertices  
vector<GLuint> textures;				//all textures

//camera transformation variables
int move_state = 0, oldX = 0, oldY = 0;
float rX = 22, rY = 116, dist = -120;

//OBJ mesh filename to load
const std::string mesh_filename = "resources/objects/blocks/blocks.obj";

//flag to enable raytracing
bool bRaytrace = true;

//background color
glm::vec4 bg = glm::vec4(0.5, 0.5, 1, 1);

//eye position
glm::vec3 eyePos;
glm::vec3 eye_XYZ(0, 20, 0);

//scene axially aligned bounding box
tmp::BBox aabb;

GLuint texVerticesID; //texture storing vertex positions
GLuint texTrianglesID; //texture storing triangles list 

					   //light crosshair gizmo vetex array and buffer object IDs
GLuint lightVAOID;
GLuint lightVerticesVBO;
glm::vec3 lightPosOS = glm::vec3(0, 2, 0); //objectspace light position

//spherical cooridate variables for light rotation
float theta = 0.66f;
float phi = -1.0f;
float ligth_radius = 70;

//FPS related variables
int total_frames = 0;
float fps = 0;
float lastTime = 0;

//texture ID for array texture
GLuint textureID;

void check_errors() { CHECK(glGetError() == GL_NO_ERROR); }

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
		cout << "Cannot load the 3ds mesh" << endl;
		exit(EXIT_FAILURE);
	}

	check_errors();

	int total = 0; 
	for (size_t k = 0; k<materials.size(); k++) 
		if (materials[k]->map_Kd != "") 
			total++;

	// 	//creare a single array texture to store all textures
	for (size_t k = 0; k<materials.size(); k++) 
	{
		//if the diffuse texture name is not empty
		if (materials[k]->map_Kd != "") {
			if (k == 0) {
				//generate a new OpenGL array texture
				glGenTextures(1, &textureID);
				glBindTexture(GL_TEXTURE_2D_ARRAY, textureID);
				glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
			}
			int texture_width = 0, texture_height = 0, channels = 0;

			const string& filename = materials[k]->map_Kd;

			std::string full_filename = mesh_path;
			full_filename.append(filename);

			//use SOIL to load the texture
			GLubyte* pData = stbi_load(full_filename.c_str(), &texture_width, &texture_height, &channels, 0);
			if (pData == NULL) {
				cerr << "Cannot load image: " << full_filename.c_str() << endl;
				exit(EXIT_FAILURE);
			}

			//Flip the image on Y axis
			int i, j;
			for (j = 0; j * 2 < texture_height; ++j)
			{
				int index1 = j * texture_width * channels;
				int index2 = (texture_height - 1 - j) * texture_width * channels;
				for (i = texture_width * channels; i > 0; --i)
				{
					GLubyte temp = pData[index1];
					pData[index1] = pData[index2];
					pData[index2] = temp;
					++index1;
					++index2;
				}
			}
			//get the image format
			GLenum format = GL_RGBA;
			switch (channels) {
			case 2:	format = GL_RG32UI; break;
			case 3: format = GL_RGB;	break;
			case 4: format = GL_RGBA;	break;
			}

			//if this is the first texture, allocate the array texture
			if (k == 0) {
				// 创建一个 2D 纹理数组
				glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, format, texture_width, texture_height, total, 0, format, GL_UNSIGNED_BYTE, NULL);
			}
			//modify the existing texture
			glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, k, texture_width, texture_height, 1, format, GL_UNSIGNED_BYTE, pData);

			//release the SOIL image data
			stbi_image_free(pData);
		}
	}
	check_errors();

	//load flat shader
	Shader flatShader("raytracing/flat.vert", "raytracing/flat.frag");

	Shader raytraceShader("raytracing/raycast.vert", "raytracing/raycast.frag");
	raytraceShader.enable();
	raytraceShader.set_float("VERTEX_TEXTURE_SIZE", (float)vertices2.size());
	raytraceShader.set_float("TRIANGLE_TEXTURE_SIZE", (float)indices2.size() / 4);
	raytraceShader.set_vec3("aabb.min", aabb.min);
	raytraceShader.set_vec3("aabb.max", aabb.max);
	raytraceShader.set_vec4("backgroundColor", bg);
	raytraceShader.set_int("vertex_positions", 1);
	raytraceShader.set_int("triangles_list", 2);
	raytraceShader.disable();

	check_errors();

	//load mesh rendering shader
	/*
	shader.LoadFromFile(GL_VERTEX_SHADER, "shaders/shader.vert");
	shader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/shader.frag");
	//compile and link shader
	shader.CreateAndLinkProgram();
	shader.Use();
	//add attribute and uniform
	shader.AddAttribute("vVertex");
	shader.AddAttribute("vNormal");
	shader.AddAttribute("vUV");
	shader.AddUniform("MV");
	shader.AddUniform("N");
	shader.AddUniform("P");
	shader.AddUniform("textureMap");
	shader.AddUniform("textureIndex");
	shader.AddUniform("useDefault");
	shader.AddUniform("diffuse_color");
	shader.AddUniform("light_position");
	//set values of constant uniforms as initialization	
	glUniform1i(shader("textureMap"), 0);
	shader.UnUse();
	*/
	check_errors();
		/*
	//setup the vertex array object and vertex buffer object for the mesh
	//geometry handling 
	glGenVertexArrays(1, &vaoID);
	glGenBuffers(1, &vboVerticesID);
	glGenBuffers(1, &vboIndicesID);

	glBindVertexArray(vaoID);
	glBindBuffer(GL_ARRAY_BUFFER, vboVerticesID);
	//pass mesh vertices
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*vertices.size(), &(vertices[0].pos.x), GL_STATIC_DRAW);
	check_errors();

		glGetUniformLocation(shader.program_id, name.c_str());
		//enable vertex attribute array for vertex position
	glEnableVertexAttribArray(shader["vVertex"]);
	glVertexAttribPointer(shader["vVertex"], 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	check_errors();

		//enable vertex attribute array for vertex normal
	glEnableVertexAttribArray(shader["vNormal"]);
	glVertexAttribPointer(shader["vNormal"], 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)(offsetof(Vertex, normal)));

	check_errors();
		//enable vertex attribute array for vertex texture coordinates
	glEnableVertexAttribArray(shader["vUV"]);
	glVertexAttribPointer(shader["vUV"], 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)(offsetof(Vertex, uv)));

	check_errors();

		//if we have a single material, it means the 3ds model contains one mesh
		//we therefore load it into an element array buffer
		if (materials.size() == 1) {
			//pass indices to the element array buffer if there is a single material			
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndicesID);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort)*indices.size(), &(indices[0]), GL_STATIC_DRAW);
		}
	check_errors();

		glBindVertexArray(0);
	*/
	//setup vao and vbo stuff for the light position crosshair
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
	check_errors();
		//enable vertex attribute array for vertex position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	check_errors();

	//use spherical coordinates to get the light position
	lightPosOS.x = ligth_radius * cos(theta)*sin(phi);
	lightPosOS.y = ligth_radius * cos(phi);
	lightPosOS.z = ligth_radius * sin(theta)*sin(phi);



	//pass position to 1D texture bound to texture unit 1
	glGenTextures(1, &texVerticesID);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texVerticesID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	GLfloat* pData = new GLfloat[vertices2.size() * 4];
	int count = 0;
	for (size_t i = 0; i<vertices2.size(); i++) {
		pData[count++] = vertices2[i].x;
		pData[count++] = vertices2[i].y;
		pData[count++] = vertices2[i].z;
		pData[count++] = 0;
	}
	//allocate a floating point texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, vertices2.size(), 1, 0, GL_RGBA, GL_FLOAT, pData);

	//delete the data pointer
	delete[] pData;

	check_errors();

		//store the mesh topology in another texture bound to texture unit 2
		glGenTextures(1, &texTrianglesID);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, texTrianglesID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	GLushort* pData2 = new GLushort[indices2.size()];
	count = 0;
	for (size_t i = 0; i<indices2.size(); i += 4) {
		pData2[count++] = (indices2[i]);
		pData2[count++] = (indices2[i + 1]);
		pData2[count++] = (indices2[i + 2]);
		pData2[count++] = (indices2[i + 3]);
	}
	//allocate an integer format texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16I, indices2.size() / 4, 1, 0, GL_RGBA_INTEGER, GL_UNSIGNED_SHORT, pData2);

	//delete heap allocated buffer
	delete[] pData2;

	check_errors();

		//set texture unit 0 as active texture unit
		glActiveTexture(GL_TEXTURE0);

	//enable depth test and culling
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	//set the background colour
	glClearColor(bg.x, bg.y, bg.z, bg.w);

	cout << "Initialization successfull" << endl;


	rX = 0, rY = 0;

	while (!glfwWindowShouldClose(window))
	{
		update();

		//update the light position
		lightPosOS.x = ligth_radius * cos(theta)*sin(phi);
		lightPosOS.y = ligth_radius * cos(phi);
		lightPosOS.z = ligth_radius * sin(theta)*sin(phi);

		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//set the camera transformation
		
		//rX = 0, rY = 0;
		dist = 0;
		glm::mat4 T_scene = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, dist));
		glm::mat4 Rx = glm::rotate(T_scene, rX, glm::vec3(1.0f, 0.0f, 0.0f));	// 旋转
		glm::mat4 MV = glm::rotate(Rx, rY, glm::vec3(0.0f, 1.0f, 0.0f));

		//MV = glm::rotate(MV, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));

		//get the eye position and inverse of MVP matrix
		glm::mat4 invMV = glm::inverse(MV);
		glm::vec3 eyePos = glm::vec3(invMV[3][0], invMV[3][1], invMV[3][2]);
		glm::mat4 invMVP = glm::inverse(P*MV);
		
		/*
		// view/projection transformations
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
		glm::mat4 view = camera.GetViewMatrix();

		// render the loaded model
		glm::mat4 model(1.f);
		model = glm::translate(model, glm::vec3(0.0f, -2.f, 10.0f)); // translate it down so it's at the center of the scene
		//model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));	// it's a bit too big for our scene, so scale it down
		glm::mat4 MV = view * model;
		glm::mat4 MVP = projection * MV;
		glm::mat4 invMVP = glm::inverse(MVP);
		//glm::mat4 mvp = model * view;
		//mvp = mvp * projection;
		//glm::mat4 invMVP = glm::inverse(mvp);
		//set the camera transformation
		*/
		//if raytracing is enabled
		if (bRaytrace) {
			//set the raytracing shader
			raytraceShader.enable();
			//pass shader uniforms
			// camera.Position
			//eyePos = { 0, 50, 0 };	//以y轴为垂直轴
			raytraceShader.set_vec3("eyePos", eye_XYZ);
			raytraceShader.set_mat4("invMVP", invMVP);
			raytraceShader.set_vec3("light_position", lightPosOS);

			//draw a fullscreen quad
			//DrawFullScreenQuad();
			quad.draw();
			//unbind raytracing shader
			raytraceShader.disable();
		}


		// 渲染小十字
		//disable depth testing
		glDisable(GL_DEPTH_TEST);

		//draw the light gizmo, set the light vertexx array object
		glBindVertexArray(lightVAOID); {
			MV = glm::rotate(MV, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			//set the modelling transform for the light crosshair gizmo
			glm::mat4 T = glm::translate(glm::mat4(1), lightPosOS);
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


		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
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

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
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

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		bRaytrace = !bRaytrace;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
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
	{ 
		camera.ProcessMouseMovement(xoffset, yoffset);

		//rY += xoffset / 180.0f;	// 旋转
		//rX += yoffset / 180.0f;
		eye_XYZ.x += xoffset / 80.0f;	// 旋转
		eye_XYZ.z += yoffset / 80.0f;
	}
	else if (move_state == 1)
	{
		//水平移动
		//lightPosOS.x += xoffset;
		//lightPosOS.z += yoffset;

		// 或在球上运动
		theta += xoffset / 60.0f;
		phi   += yoffset / 60.0f;
	}

	lastX = xpos;
	lastY = ypos;
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

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);

	ligth_radius -= 10 * yoffset;
}