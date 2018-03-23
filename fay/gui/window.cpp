#include "window.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

namespace fay
{

// Locate Data
static GLFWwindow*  g_Window = nullptr;

static IO io;

// callback

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	io.left = io.middle = io.right = false;
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		io.left = true;
	else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		io.middle = true;
	else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
		io.right = true;
}

void scroll_callback(GLFWwindow* window, double /*xoffset*/, double yoffset)
{
	io.wheel = yoffset;
}

// environment 

bool create_window(int width, int height, bool multisample)
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	if (multisample)
		glfwWindowHint(GLFW_SAMPLES, 4);
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif


	const GLubyte* name = glGetString(GL_VENDOR); //返回负责当前OpenGL实现厂商的名字  
	const GLubyte* biaoshifu = glGetString(GL_RENDERER); //返回一个渲染器标识符，通常是个硬件平台  
	const GLubyte* OpenGLVersion = glGetString(GL_VERSION); //返回当前OpenGL实现的版本号    

	printf("OpenGL实现厂商的名字：%s\n", name);
	printf("渲染器标识符：%s\n", biaoshifu);
	printf("OpenGL实现的版本号：%s\n", OpenGLVersion);

	// glfw window creation
	g_Window = glfwCreateWindow(width, height, "LearnOpenGL", NULL, NULL);
	if (g_Window == NULL)
	{
		LOG(ERROR) << "Failed to create GLFW window";
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(g_Window);
	glfwSetFramebufferSizeCallback(g_Window, framebuffer_size_callback);

	glfwSetMouseButtonCallback(g_Window, mouse_button_callback);
	glfwSetScrollCallback(g_Window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(g_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		LOG(ERROR) << "Failed to initialize GLAD";
		return false;
	}

	glfwGetCursorPos(g_Window, &io.xlast, &io.ylast);
	return true;
}

bool close_window()
{
	{
		if (glfwGetKey(g_Window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(g_Window, true);

		return static_cast<bool>(glfwWindowShouldClose(g_Window));
	}
}

void delete_window()
{
	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
}

void updateIO()
{
	glfwPollEvents();

	// mouse position
	io.xlast = io.xpos; io.ylast = io.ypos;
	//if (glfwGetWindowAttrib(g_Window, GLFW_FOCUSED))
	glfwGetCursorPos(g_Window, &io.xpos, &io.ypos);
	io.dx = io.xpos - io.xlast; io.dy = io.ypos - io.ylast;

	// time
	io.last_time = io.cur_time;
	io.cur_time = glfwGetTime();	// io.DeltaTime = g_Time > 0.0 ? (current_time - g_Time) : (float)(1.0f / 60.0f);
	io.delta_time = io.cur_time - io.last_time;
}

void swap_framebuffer()	// 不清除颜色，交换缓冲区
{
	glfwSwapBuffers(g_Window);
}

bool keydown(int key) { return (glfwGetKey(g_Window, key) == GLFW_PRESS); }

const IO& get_io() { return io; }

}	// namespace fay