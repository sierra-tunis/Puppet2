#pragma once

#ifndef PUPPET_DEBUGMENU
#define PUPPET_DEBUGMENU

#include "GameObject.h"
#include "motion_constraint.h"
#include "UI.h"
#include "zdata.hpp"

class DebugMenu : public GameObject {
	std::vector<Button*> buttons_;

	Button test_button_;
	Button next_target_;
	Button prev_target_;
	Textbox fps_tbox_;
	TextGraphics& text_graphics_;
	Slider test_slider_;

	DebugCamera& debug_cam_;
	OffsetConnector cam_clamp_;

	GameObject* debug_target_;
	Textbox target_name_;
	Textbox target_dbg_info_;

	Button next_level_;
	Button prev_level_;
	Textbox level_display_;
	
	float avg_fps_;
	float time_since_last_fps_avg_;
	int frame_counter_;

	GLFWwindow* window_;
	Default2d& graphics_2d_;

	//Button show_hitboxes_;

	void onKeyPress(int key) override {
		if (key == GLFW_KEY_F3) {
			toggleHidden();
			for (auto& b : buttons_) {
				b->setHideState(isHidden());
			}
			if (!isHidden()) {
				text_graphics_.add(fps_tbox_);
				text_graphics_.add(target_dbg_info_);
				text_graphics_.add(target_name_);
				text_graphics_.add(level_display_);
			} else {
				text_graphics_.remove(fps_tbox_);
				text_graphics_.remove(target_dbg_info_);
				text_graphics_.remove(target_name_);
				text_graphics_.remove(level_display_);
			}
		}
	}

public:

	static void nextLevelCallback(void* must_be_this) {
		DebugMenu* this_ = static_cast<DebugMenu*>(must_be_this);
		Level::nextLevel();
		//this_->setDebugTarget(Level::getCurrentLevel()->getContents()[0]);
		this_->setDebugTarget(nullptr);
	}
	static void prevLevelCallback(void* must_be_this) {
		DebugMenu* this_ = static_cast<DebugMenu*>(must_be_this);
		Level::prevLevel();
		//this_->setDebugTarget(Level::getCurrentLevel()->getContents()[0]);
		this_->setDebugTarget(nullptr);
	}

	static void nextTargetCallback(void* must_be_this) {
		DebugMenu* this_ = static_cast<DebugMenu*>(must_be_this);
		if (this_->debug_target_ != nullptr) {
			this_->debug_target_->closeDebugUI(this_, this_->window_, this_->graphics_2d_, this_->text_graphics_);
		}
		if (Level::getCurrentLevel() == nullptr) { return; };
		if (this_->debug_target_ == nullptr) {
			this_->setDebugTarget(*Level::getCurrentLevel()->getContents().begin());
		} else {
			auto current_loc = std::find(Level::getCurrentLevel()->getContents().begin(), Level::getCurrentLevel()->getContents().end(), this_->debug_target_);
			if (current_loc != Level::getCurrentLevel()->getContents().end()-1) {
				this_->setDebugTarget(*(current_loc + 1));
			} else {
				this_->setDebugTarget(nullptr);
			}
		}
		if (this_->debug_target_ == &this_->debug_cam_) {
			nextTargetCallback(must_be_this);
		}
		if (this_->debug_target_ != nullptr) {
			this_->debug_target_->openDebugUI(this_, this_->window_, this_->graphics_2d_, this_->text_graphics_);
		}
	}

	static void prevTargetCallback(void* must_be_this) {
		DebugMenu* this_ = static_cast<DebugMenu*>(must_be_this);
		if (this_->debug_target_ != nullptr) {
			this_->debug_target_->closeDebugUI(this_, this_->window_, this_->graphics_2d_, this_->text_graphics_);
		}
		if (Level::getCurrentLevel() == nullptr) { return; };
		if (this_->debug_target_ == nullptr) {
			this_->setDebugTarget(*(Level::getCurrentLevel()->getContents().end()-1));
			
		}
		else {
			auto current_loc = std::find(Level::getCurrentLevel()->getContents().begin(), Level::getCurrentLevel()->getContents().end(), this_->debug_target_);
			if (current_loc != Level::getCurrentLevel()->getContents().begin()) {
				this_->setDebugTarget(*(current_loc - 1));
			}
			else {
				this_->setDebugTarget(nullptr);
			}
		}
		if (this_->debug_target_ == &this_->debug_cam_) {
			prevTargetCallback(must_be_this);
		}
		if (this_->debug_target_ != nullptr) {
			this_->debug_target_->openDebugUI(this_, this_->window_, this_->graphics_2d_, this_->text_graphics_);
		}
	}
	static void testSliderCallback(float new_val, void* must_be_nullptr) {
		std::cout << new_val << "\n";
	}

	void addButton(Button* button){
		buttons_.push_back(button);
	}

	DebugMenu(GLFWwindow* window, Default2d& graphics, TextGraphics& text_graphics, DebugCamera& debug_camera) : GameObject("debug_menu"),
		test_button_(.1, .2, "test_button"),
		test_slider_(.1, .3, 0, 1),
		next_target_(.2, .2, "next_target"),
		prev_target_(.2, .2, "prev_target"),
		next_level_(.2, .2, "next_level"),
		prev_level_(.2, .2, "prev_level"),
		fps_tbox_(),
		frame_counter_(0),
		text_graphics_(text_graphics),
		graphics_2d_(graphics),
		debug_cam_(debug_camera),//should eventually move debug camera construction and management into DebugMenu
		cam_clamp_(Eigen::Matrix4f::Identity()),
		window_(window){

		test_button_.activateMouseInput(window);
		test_button_.moveTo(.6, .9, 0);
		addButton(&test_button_);
		graphics.add(test_button_);

		fps_tbox_.text = "0";
		fps_tbox_.box_height = .5;
		fps_tbox_.box_width = .5;
		fps_tbox_.left = .5;
		fps_tbox_.top = .6;
		text_graphics.add(fps_tbox_);//for some reason removing this and beginning with the menu hidden causes an error

		test_slider_.load(window, graphics, text_graphics);
		test_slider_.moveTo(.5, -.5, 0);
		test_slider_.setSliderChangeCallback(&testSliderCallback,nullptr);

		target_dbg_info_.box_width = 2;
		target_dbg_info_.top = .95;
		target_dbg_info_.left = -1.;
		text_graphics.add(target_dbg_info_);

		prev_target_.moveTo(-1, -.4, 0);
		prev_target_.activateMouseInput(window);
		graphics.add(prev_target_);
		next_target_.moveTo(-.2, -.4, 0);
		next_target_.activateMouseInput(window);
		graphics.add(next_target_);
		addButton(&next_target_);
		addButton(&prev_target_);
		prev_target_.setCallback(prevTargetCallback,this);
		next_target_.setCallback(nextTargetCallback,this);

		target_name_.box_width = .6;
		target_name_.box_height = .2;
		target_name_.font_size = 1;
		target_name_.left = -.8;
		target_name_.top = -.4;


		prev_level_.moveTo(-1, 0., 0);
		prev_level_.activateMouseInput(window);
		graphics.add(prev_level_);
		next_level_.moveTo(-.2, 0., 0);
		next_level_.activateMouseInput(window);
		graphics.add(next_level_);
		addButton(&next_level_);
		addButton(&prev_level_);
		prev_level_.setCallback(prevLevelCallback, this);
		next_level_.setCallback(nextLevelCallback, this);


		level_display_.box_width = .6;
		level_display_.box_height = .2;
		level_display_.left = -.8;
		level_display_.top = 0;

	}

	void update(GLFWwindow* window) override {
		GameObject::update(window);

		frame_counter_++;
		time_since_last_fps_avg_ += getdt();
		if (time_since_last_fps_avg_ > 1.) {
			avg_fps_ = static_cast<float>(frame_counter_) / time_since_last_fps_avg_;
			time_since_last_fps_avg_ = 0.;
			frame_counter_ = 0;
			text_graphics_.unload(fps_tbox_);
			fps_tbox_.text = std::to_string(avg_fps_);
			text_graphics_.add(fps_tbox_);
		}

		test_slider_.update(window);
		if (debug_target_ != nullptr) {
			debug_cam_.connectTo(debug_target_,&cam_clamp_);
		} else {
			debug_cam_.disconnect();
		}

		if (!isHidden()) {
			std::string dbg_info;
			std::string name_text;
			if (debug_target_ == nullptr) {
				dbg_info = "";
				name_text = "no Target";
			}else {
				dbg_info = debug_target_->getDebugInfo();
				name_text = debug_target_->getName();
			}
			if (dbg_info != target_dbg_info_.text) {
				text_graphics_.unload(target_dbg_info_);
				target_dbg_info_.text = dbg_info;
				text_graphics_.add(target_dbg_info_);
			}
			if (name_text == InternalObject::no_name) {
				name_text = "unnamed";
			}
			if (name_text != target_name_.text) {
				text_graphics_.unload(target_name_);
				target_name_.text = name_text;
				text_graphics_.add(target_name_);
			}
			std::string level_name;
			if (Level::getCurrentLevel() == nullptr) {
				level_name = "base room";
			} else {
				level_name = Level::getCurrentLevel()->getName();
			}
			if (level_name != level_display_.text) {
				text_graphics_.unload(level_display_);
				level_display_.text = level_name;
				text_graphics_.add(level_display_);
			}

		}
	}

	void setDebugTarget(GameObject* target) {
		debug_target_ = target;
	}


};

#endif