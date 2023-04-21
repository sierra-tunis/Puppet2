#pragma once

#ifndef PUPPET_DEBUGMENU
#define PUPPET_DEBUGMENU

#include "GameObject.h"
#include "UI.h"
#include "zdata.hpp"

class DebugMenu : public GameObject {
	std::vector<Button*> buttons_;

	Button test_button_;
	Textbox test_tbox_;
	TextGraphics& text_graphics_;

	const GameObject* debug_target_;
	Textbox target_dbg_info_;

	//Button show_hitboxes_;

	void onKeyPress(int key) override {
		if (key == GLFW_KEY_F3) {
			toggleHidden();
			for (auto& b : buttons_) {
				b->setHideState(isHidden());
			}
			if (!isHidden()) {
				text_graphics_.add(test_tbox_);
				text_graphics_.add(target_dbg_info_);
			} else {
				text_graphics_.remove(test_tbox_);
				text_graphics_.remove(target_dbg_info_);
			}
		}
	}

public:

	void addButton(Button* button){
		buttons_.push_back(button);
	}

	DebugMenu(GLFWwindow* window, Default2d& graphics, TextGraphics& text_graphics): GameObject("debug_menu"),
		test_button_(.1, .2, "test_button"),
		test_tbox_(),
		text_graphics_(text_graphics){

		test_button_.activateMouseInput(window);
		test_button_.moveTo(-.8, .9, 0);
		addButton(&test_button_);
		graphics.add(test_button_);

		test_tbox_.text = "this is a test, abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
		test_tbox_.box_height = .5;
		test_tbox_.box_width = .5;
		test_tbox_.left = .5;
		test_tbox_.top = .5;
		text_graphics.add(test_tbox_);//for some reason removing this and beginning with the menu hidden causes an error

		target_dbg_info_.box_width = 2;
		target_dbg_info_.top = .5;
		target_dbg_info_.left = 0;
		text_graphics.add(target_dbg_info_);
	}

	void update(GLFWwindow* window) override {
		if (debug_target_ != nullptr && !isHidden()) {
			std::string dbg_info = debug_target_->getDebugInfo();
			//std::cout << dbg_info << "\n";
			//std::cout << target_dbg_info_.text << "\n";
			if (debug_target_->getDebugInfo() != target_dbg_info_.text) {
				text_graphics_.remove(target_dbg_info_);
				target_dbg_info_.text = dbg_info;
				text_graphics_.add(target_dbg_info_);
			}
		}
	}

	void setDebugTarget(const GameObject* target) {
		debug_target_ = target;
	}


};

#endif