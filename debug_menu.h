#pragma once

#ifndef PUPPET_DEBUGMENU
#define PUPPET_DEBUGMENU

#include "GameObject.h"
#include "UI.h"

class DebugMenu : public GameObject {
	std::vector<Button*> buttons_;

	Button test_button_;
	//Button show_hitboxes_;

	void onKeyPress(int key) override {
		if (key == GLFW_KEY_F3) {
			toggleHidden();
			for (auto& b : buttons_) {
				b->setHideState(isHidden());
			}
		}
	}

public:

	void addButton(Button* button)
		{
		buttons_.push_back(button);
	}

	DebugMenu(GLFWwindow* window, Default2d* graphics): GameObject("debug_menu"),
		test_button_(.1, .2, "test_button") {

		test_button_.activateMouseInput(window);
		test_button_.moveTo(-.8, .9, 0);
		addButton(&test_button_);
		graphics->add(test_button_);
	}


};

#endif