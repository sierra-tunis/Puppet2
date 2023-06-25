#include "GameObject.h"
#include "UI.h"
#include "textbox_object.hpp"

std::unordered_set<GameObject*> GameObject::global_game_objects;
/*
void GameObject::openDebugUI(const GameObject* UI_container, GLFWwindow* window, GraphicsRaw<GameObject>& graphics_2d, GraphicsRaw<Textbox>& text_graphics) {
	TextboxObject* position_display = new TextboxObject();
	position_display->box_width = 1;
	position_display->box_height = .5;
	const Eigen::Matrix4f& M = getPosition();
	constexpr std::string fs = "{:.3}";
	position_display->text = std::format(fs, M(0, 0)) + "\t " + std::format(fs, M(0, 1)) + "\t " + std::format(fs, M(0, 2)) + "\t " + std::format(fs, M(0, 3)) + "\n" +
							 std::format(fs, M(1, 0)) + "\t " + std::format(fs, M(1, 1)) + "\t " + std::format(fs, M(1, 2)) + "\t " + std::format(fs, M(1, 3)) + "\n" +
							 std::format(fs, M(2, 0)) + "\t " + std::format(fs, M(2, 1)) + "\t " + std::format(fs, M(2, 2)) + "\t " + std::format(fs, M(2, 3)) + "\n" +
							 std::format(fs, M(3, 0)) + "\t " + std::format(fs, M(3, 1)) + "\t " + std::format(fs, M(3, 2)) + "\t " + std::format(fs, M(3, 3)) + "\n";
	position_display->moveTo(-.5, -.75,0);
	addDependent(position_display);
	position_display->clampTo(UI_container);
	text_graphics.add(*position_display);
	position_display_ = position_display;

}

void GameObject::closeDebugUI(const GameObject* UI_container, GLFWwindow* window, GraphicsRaw<GameObject>& graphics_2d, GraphicsRaw<Textbox>& text_graphics) {
	TextboxObject* position_display = static_cast<TextboxObject*>(position_display_);
	removeDependent(position_display);
	text_graphics.unload(*position_display);
	delete position_display_;
	position_display_ = nullptr;


}
*/
