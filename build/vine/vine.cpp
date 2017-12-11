#include "fay/gui/gui.h"

using namespace fay;

int main()
{
	bool close_window = false;
	gui_create_window(800, 600);
	ImVec4 clear_color = ImColor(114, 144, 154);

	char come[256] {}, go[256] {};

	while ((!gui_close_window()) || close_window )
	{
		gui_updateIO();

		glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);

		ImGui::Text("Chat Window");
		ImGui::ColorEdit3("clear color", (float*)&clear_color);
		if (ImGui::Button("Test Window")) close_window ^= 1;

		ImGui::Text(go);
		ImGui::InputText("##Text", go, 255);

		gui_drawGUI();
	}

	gui_delete_window();
	return 0;
}