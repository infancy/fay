#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef FAY_GUI_GUI_H
#define FAY_GUI_GUI_H

#include "fay.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>

namespace fay
{

bool gui_create_window(int width = 1280, int height = 720);
bool gui_close_window();
void gui_delete_window();

void gui_updateIO();
void gui_drawGUI();

ImGuiIO& gui_get_io();	// const ImGuiIO& gui_get_io();	

}	//namespace fay

#endif //FAY_GUI_GUI_H
