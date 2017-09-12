#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef FAY_OPENGL_OPENGL_H
#define FAY_OPENGL_OPENGL_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include "fay.h"

namespace fay
{

//OpenGL gl;
class OpenGL
{
public:
	OpenGL() : camera(glm::vec3(0.0f, 0.0f, 3.0f))
	{
		create_window();
		set_global_state();
	}

	~OpenGL() { glfwTerminate(); }

	//bool load_resources();

	void update();
	void draw();	// = 0
	void loop();

private:
	bool create_window();
	void set_global_state();

	void process_input();

	//callback
	static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
	
private:
	// window 
	GLFWwindow* window;
	size_t width, height;

	// camera
	Camera camera;
	float lastX = width / 2.0f;
	float lastY = height / 2.0f;
	bool firstMouse = true;

	// timing
	float deltaTime = 0.0f;
	float lastFrame = 0.0f;

	//VAO\VBO\IBO
};

//class LoadModel : public OpenGL


}	//namespace fay


#endif //FAY_OPENGL_OPENGL_H
