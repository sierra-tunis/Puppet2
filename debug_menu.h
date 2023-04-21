#pragma once

#ifndef PUPPET_DEBUGMENU
#define PUPPET_DEBUGMENU

#include "GameObject.h"
#include "UI.h"
#include "zdata.hpp"

class DebugMenu : public GameObject {
	std::vector<Button*> buttons_;

	Button test_button_;
	Button next_target_;
	Button prev_target_;
	Textbox test_tbox_;
	TextGraphics& text_graphics_;

	const GameObject* debug_target_;
	Textbox target_name_;
	Textbox target_dbg_info_;

	const Level* target_level_;

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
		next_target_(.2,.2,"next_target"),
		prev_target_(.2, .2, "prev_target"),
		test_tbox_(),
		text_graphics_(text_graphics){

		test_button_.activateMouseInput(window);
		test_button_.moveTo(-.8, .9, 0);
		//addButton(&test_button_);
		graphics.add(test_button_);

		test_tbox_.text = "this is a test, abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
		test_tbox_.box_height = .5;
		test_tbox_.box_width = .5;
		test_tbox_.left = 0;
		test_tbox_.top = 1;
		text_graphics.add(test_tbox_);//for some reason removing this and beginning with the menu hidden causes an error

		target_dbg_info_.box_width = 2;
		target_dbg_info_.top = 0;
		target_dbg_info_.left = -1.;
		text_graphics.add(target_dbg_info_);

		prev_target_.moveTo(0, -.4, 0);
		prev_target_.activateMouseInput(window);
		graphics.add(prev_target_);
		next_target_.moveTo(.8, -.4, 0);
		next_target_.activateKeyInput(window);
		graphics.add(next_target_);

		target_name_.box_width = .6;
		target_name_.box_height = .2;
		target_name_.font_size = 1.5;
		target_name_.left = .2;
		target_name_.top = -.4;
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
			std::string name_text = debug_target_->getName();
			if (name_text == InternalObject::no_name) {
				name_text = "unnamed";
			}
			if (name_text != target_name_.text) {
				text_graphics_.remove(target_name_);
				target_name_.text = name_text;
				text_graphics_.add(target_name_);
			}
		}
	}

	void setDebugTarget(const GameObject* target) {
		debug_target_ = target;
	}


};

#endif