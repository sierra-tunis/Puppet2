#pragma once

#ifndef PUPPET_DEBUGPLAYER
#define PUPPET_DEBUGPLAYER

#include "Humanoid.hpp"

//class debug_player : public Humanoid {
class DebugPlayer : public Humanoid {
	KeyStateCallback<GLFW_KEY_UP, GLFW_KEY_DOWN, GLFW_KEY_LEFT, GLFW_KEY_RIGHT, GLFW_KEY_SPACE, GLFW_KEY_LEFT_SHIFT> key_state_callback_caller_;
	Animation<Humanoid::n_dofs> the_griddy_;
	Animation<Humanoid::n_dofs> standing_;

	int n_move_keys_down_;
	float walk_speed_;

	void onKeyPress(int key) override {
		if (key == GLFW_KEY_UP || key == GLFW_KEY_DOWN) {
			if (n_move_keys_down_ == 0) {
				setSkeletonAnimation(&the_griddy_);
				the_griddy_.start();
			}
			n_move_keys_down_++;
		}
	}
	void onKeyRelease(int key) override {
		if (key == GLFW_KEY_UP || key == GLFW_KEY_DOWN) {
			n_move_keys_down_--;
			if (n_move_keys_down_ == 0) {
				the_griddy_.stop();
				setSkeletonAnimation(&standing_);
			}
		}

	}

	void onKeyDown(int key) override {
		switch (key) {
		case GLFW_KEY_UP:
			//boundedTranslate(this->getPosition()(seq(0, 2), 2) * -.01,current_level_->getZmap(), .5);
			translate(this->getPosition()(seq(0, 2), 2) * -walk_speed_*getdt());
			break;
		case GLFW_KEY_DOWN:
			translate(this->getPosition()(seq(0, 2), 2) * walk_speed_*getdt());
			break;
		case GLFW_KEY_LEFT:
			rotateY(.01);
			break;
		case GLFW_KEY_RIGHT:
			rotateY(-.01);
			break;
		case GLFW_KEY_SPACE:
			//translate(this->getPosition()(seq(0, 2), 1) * .05);
			break;
		case GLFW_KEY_LEFT_SHIFT:
			//translate(this->getPosition()(seq(0, 2), 1) * -.05);
			break;
		}
	}

public:
	DebugPlayer() :
		Humanoid("debug_player", key_state_callback_caller_),
		the_griddy_("the_griddy.csv"),
		standing_("standing.csv"),
		walk_speed_(2.5){

		addAnimation(&the_griddy_);
		the_griddy_.load();
		the_griddy_.setPlaybackSpeed(4);

		addAnimation(&standing_);
		standing_.load();
		setSkeletonAnimation(&standing_);
	}


	/*
	void openDebugUI(GameObject* UI_container, GraphicsRaw<GameObject>& graphics_2d, GraphicsRaw<Textbox>& text_graphics) override {

	}

	void closeDebugUI(GameObject* UI_container, GraphicsRaw<GameObject>& graphics_2d, GraphicsRaw<Textbox>& text_graphics) override {

	}*/

};

#endif