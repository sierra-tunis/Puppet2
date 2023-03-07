#pragma once
#include "InternalObject.h"
#include "Graphics.h"


#ifndef PUPPET_CAMERA
#define PUPPET_CAMERA

using Eigen::Matrix4f;

class Camera : public InternalObject {

private:
	bool screenshot_flag_;

public:

	Camera(Matrix4f position, int id) :InternalObject(position, id),screenshot_flag_(false) {}

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
