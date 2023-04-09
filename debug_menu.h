#pragma once

#ifndef PUPPET_DEBUGMENU
#define PUPPET_DEBUGMENU

#include "GameObject.h"
#include "UI.h"

class DebugMenu : public GameObject {
	std::vector<Button*> buttons_;

	void onKeyPress(int key) override {
		if (key == GLFW_KEY_F3) {
			toggleHidden();
			for (auto& b : buttons_) {
				b->setHideState(isHidden());
			}
		}
	}

public:

	void addButton(Button* button) {
		buttons_.push_back(button);
	}

	DebugMenu(): GameObject("debug_menu") {

	}


};

#endif