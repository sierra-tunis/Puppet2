#pragma once

#ifndef PUPPET_CAMERA
#define PUPPET_CAMERA

#include "InternalObject.h"
#include "Graphics.h"


using Eigen::Matrix4f;

class Camera : public InternalObject {

private:
	bool screenshot_flag_;

public:

	Camera(std::string name) : InternalObject(name),screenshot_flag_(false) {}

	/*void bindTo(const InternalObject& target) {
		clampTo(target);
	}*/

	void onKeyPress(int key) override {
		if (key == GLFW_KEY_F3) [[unlikely]]{
			this->screenshot_flag_ = true;
		}
	}

	bool getScreenshotFlag() const {
		return this->screenshot_flag_;
	}

	void clearScreenshotFlag() {
		this->screenshot_flag_ = false;
	}

	void update(GLFWwindow* window) override {
		InternalObject::update(window);
	}

};

#endif // !PUPPET_CAMERA
