#pragma once

#ifndef PUPPET_DEBUGPLAYER
#define PUPPET_DEBUGPLAYER

#include "Humanoid.hpp"

//class debug_player : public Humanoid {
class DebugPlayer : public Humanoid {
	KeyStateCallback<GLFW_KEY_UP, GLFW_KEY_DOWN, GLFW_KEY_LEFT, GLFW_KEY_RIGHT, GLFW_KEY_SPACE, GLFW_KEY_LEFT_SHIFT> key_state_callback_caller_;

	void onKeyDown(int key) override {
		switch (key) {
		case GLFW_KEY_UP:
			//boundedTranslate(this->getPosition()(seq(0, 2), 2) * -.01,current_level_->getZmap(), .5);
			translate(this->getPosition()(seq(0, 2), 2) * -.05);
			break;
		case GLFW_KEY_DOWN:
			translate(this->getPosition()(seq(0, 2), 2) * .05);
			break;
		case GLFW_KEY_LEFT:
			rotateY(.01);
			break;
		case GLFW_KEY_RIGHT:
			rotateY(-.01);
			break;
		case GLFW_KEY_SPACE:
			translate(this->getPosition()(seq(0, 2), 1) * .05);
			break;
		case GLFW_KEY_LEFT_SHIFT:
			translate(this->getPosition()(seq(0, 2), 1) * -.05);
			break;
		}
	}

public:
	DebugPlayer() :
		Humanoid("debug_player", key_state_callback_caller_) {

	}

	/*
	void openDebugUI(GameObject* UI_container, GraphicsRaw<GameObject>& graphics_2d, GraphicsRaw<Textbox>& text_graphics) override {

	}

	void closeDebugUI(GameObject* UI_container, GraphicsRaw<GameObject>& graphics_2d, GraphicsRaw<Textbox>& text_graphics) override {

	}*/

};

#endif