#pragma once

#include "fay/core/fay.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#ifdef FAY_IN_WINDOWS
    #define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include "GLFW/glfw3native.h"

#include "fay/app/input.h"
#include "fay/core/config.h"
#include "fay/core/fay.h"
#include "fay/render/define.h"

namespace fay
{

// window is responsible for managing the display of the window,
// updating user input and creating 3d graphics api context/device 

struct window_desc	// TODO: merge to config?
{
	uint32_t width{ 1080 };
	uint32_t height{ 720 };
	std::string title{ "test" };

    bool resizable = false; // resize the window.
    bool accept_drop_files = false; // drag-and-drop files into the window

	cursor_mode cursor_mode_v{ cursor_mode::hidden };

    //
    bool is_backend_opengl{ false };
};

class window
{
public:
	window() {}
	window(const window_desc& desc) 
        : desc_{ desc }
	{
	}
    virtual ~window() = default;

    virtual void* native_handle() { return nullptr; }

    // TODO

    // update: update input...

    // show : swap buffer...

    // clear

	// virtual void open() = 0;
	virtual bool should_close() = 0;
	virtual void update_input() = 0;

protected:
	window_desc desc_{};
};



// -------------------------------------------------------------------------------------------------
// glfw



namespace glfw_detail
{

// TODO£º
// inline std::unorderd_map window_input;

// glfw callback functions

inline void error_callback(int error, const char* description)
{
	LOG(ERROR) << "GLFW error " << error << ": " << description;
}

inline void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.

	// TODO: render_viewport((0, 0, width, height);
	glViewport(0, 0, width, height);
}

inline void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE); // then glfwWindowShouldClose(GLFWwindow* window) return true
    // TODO... catch other event
    //else if (action == GLFW_PRESS)
    //    input_.key = key;
}

inline void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    input_.left_down = input_.middle_down = input_.right_down = false;

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		input_.left_down = true;
	else if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS)
		input_.middle_down = true;
	else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
		input_.right_down = true;
}

inline void scroll_callback(GLFWwindow* /*window*/, double /*xoffset*/, double yoffset)
{
	input_.wheel = yoffset;
}

}

// TODO: every window has it's own glfw_ctx and render_device
class window_glfw : public window
{
public:
	// using window::window;
	window_glfw() {}
    window_glfw(const window_desc& windowd, render_desc& renderd) : window(windowd)
    {
        glfwSetErrorCallback(glfw_detail::error_callback);
        glfwInit();

        // feature
        if (renderd.anti_aliasing == anti_aliasing::MSAA)
            glfwWindowHint(GLFW_SAMPLES, renderd.multiple_sample_count);
        // GLFW_VISIBLE
        glfwWindowHint(GLFW_RESIZABLE, desc_.resizable);

        // create window and render api context
        switch (renderd.backend)
        {
            case render_backend_type::opengl:
                desc_.is_backend_opengl = true;
                create_window_and_glcontext(4, 3); // TODO: choose opengl version by itself
                break;
            case render_backend_type::opengl_dsa:
                desc_.is_backend_opengl = true;
                create_window_and_glcontext(4, 5);
                break;
            case render_backend_type::d3d11:
                create_window();
                break;
            default:
                LOG(ERROR);
                break;
        }

        // render_desc
        renderd.d3d_handle = glfwGetWin32Window(window_);
        renderd.width = desc_.width;
        renderd.height = desc_.height;

        // glfwSetWindowUserPointer(window_, this);

        // capture mouse
        switch (desc_.cursor_mode_v)
        {
            case cursor_mode::normal:
                glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                break;
            case cursor_mode::hidden:
                glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
                break;
            case cursor_mode::disabled:
                glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                break;
            default:
                LOG(ERROR);
                break;
        }

        // callback
        //glfwSetWindowSizeCallback(pGLFWWindow, ApiCallbacks::windowSizeCallback);
        glfwSetFramebufferSizeCallback(window_, glfw_detail::framebuffer_size_callback);
        glfwSetKeyCallback(window_, glfw_detail::key_callback);
        //glfwSetCursorPosCallback(window_, ApiCallbacks::mouseMoveCallback);
        glfwSetMouseButtonCallback(window_, glfw_detail::mouse_button_callback);
        glfwSetScrollCallback(window_, glfw_detail::scroll_callback);
        //glfwSetCharCallback(window_, ApiCallbacks::charInputCallback);
        //glfwSetDropCallback(window_, ApiCallbacks::droppedFileCallback);

        // init input
        input_.this_time = glfwGetTime();
        glfwGetCursorPos(window_, &input_.lastx, &input_.lasty);
        glfwGetCursorPos(window_, &input_.x, &input_.y);
    }
	~window_glfw() override
	{
		// Destroys the specified window and its context.
		glfwDestroyWindow(window_);
		// clearing all previously allocated GLFW resources.
		glfwTerminate();
	}

    void* native_handle() override
    {
        return window_;
    }

	void update()
	{
        update_input();
        float frame_rate = 1.0 / input.delta_time;
        //glfwSetWindowTitle(window_, std::to_string(frame_rate).c_str());
	}

	// TODO:remove
	void show()	// don't clear framebuffer
	{

        if(desc_.is_backend_opengl)
            glfwSwapBuffers(window_);

        //glfwShowWindow(window_);
        //glfwFocusWindow(window_);

        // http://www.glfw.org/docs/latest/input_guide.html#events
        // Event processing must be done regularly while you have any windows and is normally done each frame after buffer swapping.
        //glfwPollEvents(); -> update input at the begin in every frame
	}

    // TODO: broadcast(event::close_window);
    bool should_close() override
    {
        return glfwWindowShouldClose(window_);
    }

private:
	// create glcontext by glfw and glad, or you want to handle it by yourself, deal with wgl, glx...?
	void create_window_and_glcontext(int major, int minor)
	{
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    #ifdef FAY_DEBUG
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    #endif // FAY_DEBUG

	#ifdef FAY_IN_APPLE
		// uncomment this statement to fix compilation on OS X
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	#endif

		window_ = glfwCreateWindow(desc_.width, desc_.height, desc_.title.c_str(), nullptr, nullptr);
		if (window_ == nullptr)
		{
			LOG(ERROR) << "window_glfw: Failed to create GLFW window";
			glfwTerminate();
		}
		glfwMakeContextCurrent(window_);

		// glad: load all OpenGL function pointers
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			LOG(ERROR) << "Failed to initialize GLAD";
		}
	}

    void create_window()
    {
    #ifdef FAY_IN_WINDOWS
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window_ = glfwCreateWindow(desc_.width, desc_.height, desc_.title.c_str(), nullptr, nullptr);
        if (window_ == nullptr)
        {
            LOG(ERROR) << "window_glfw: Failed to create GLFW window";
            glfwTerminate();
        }
        // glfwMakeContextCurrent(window_);
    #else
        #error "unknown platform"
    #endif
    }

    void update_input()
    {
        // reset value
        // input_.left_down = input_.middle_down = input_.right_down = false;
        input_.wheel = 0.0;
        input_.key = {};

        // Processing events will cause the window
        // and input callbacks associated with those events to be called.
        glfwPollEvents();

        // time
        // TODO: improve
        input_.last_time = input_.this_time;
        input_.this_time = glfwGetTime();
        input_.delta_time = input_.this_time - input_.last_time;
        if (input_.delta_time == 0.0)
            input_.delta_time = 1.0 / 60.0;	// TODO: depend by screen

        // mouse
        input_.lastx = input_.x; input_.lasty = input_.y;
        //if (glfwGetWindowAttrib(window_, GLFW_FOCUSED))
        glfwGetCursorPos(window_, &input_.x, &input_.y);
        input_.dx = input_.x - input_.lastx;
        // reversed since y-coordinates go from bottom to top but z_xais form out to in
        input_.dy = input_.lasty - input_.y;

        // keyboard
        // TODO: do it by keyboard_cb
        if (glfwGetKey(window_, GLFW_KEY_W) == GLFW_PRESS)
            input_.key['w'] = true; //input_.key == 'w';
        /*else*/if (glfwGetKey(window_, GLFW_KEY_S) == GLFW_PRESS)
            input_.key['s'] = true;
        if (glfwGetKey(window_, GLFW_KEY_A) == GLFW_PRESS)
            input_.key['a'] = true;
        if (glfwGetKey(window_, GLFW_KEY_D) == GLFW_PRESS)
            input_.key['d'] = true;
        if (glfwGetKey(window_, GLFW_KEY_Z) == GLFW_PRESS)
            input_.key['z'] = true;
        if (glfwGetKey(window_, GLFW_KEY_X) == GLFW_PRESS)
            input_.key['x'] = true;
        if (glfwGetKey(window_, GLFW_KEY_C) == GLFW_PRESS)
            input_.key['c'] = true;

        if (glfwGetKey(window_, GLFW_KEY_0) == GLFW_PRESS)
            input_.key['0'] = true; 
        if (glfwGetKey(window_, GLFW_KEY_1) == GLFW_PRESS)
            input_.key['1'] = true;
        if (glfwGetKey(window_, GLFW_KEY_2) == GLFW_PRESS)
            input_.key['2'] = true;
        if (glfwGetKey(window_, GLFW_KEY_3) == GLFW_PRESS)
            input_.key['3'] = true;
        if (glfwGetKey(window_, GLFW_KEY_4) == GLFW_PRESS)
            input_.key['4'] = true;
    }

	bool keydown(int key) { return (glfwGetKey(window_, key) == GLFW_PRESS); }

private:
	GLFWwindow*  window_{ nullptr };
};



// -------------------------------------------------------------------------------------------------
// imgui



class window_imgui
{

};

} // namespace fay