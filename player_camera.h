#pragma once

#ifndef PUPPET_PLAYERCAMERA
#define PUPPET_PLAYERCAMERA

#include <Eigen/Dense>

#include <cmath>

#include "camera.h"
#include "GameObject.h"
#include "signal.hpp"

using Eigen::Matrix4f;
using Eigen::Vector3f;



class PlayerCamera : public Camera {
private:
	float equilibrium_length_;
	OffsetConnector anchor_;
	RotationJoint pan_;
	RotationJoint tilt_;
	PrismaticJoint dist_;
	ConnectorChain<OffsetConnector,RotationJoint, RotationJoint, PrismaticJoint> tether_;
	MeshSurface cam_box_;

	float stick_drift_;
	float sensitivity_;

	float velocity_cushion_;

	bool look_mode_;
	const bool control_mode_; //i.e. CAD mode
	bool frozen_;
	bool zoom_enabled_;

	static constexpr float joystick_x_sensitivity = 1500;
	static constexpr float joystick_y_sensitivity = 1500;

	GLFWwindow* window_;

	//Signal damped_tether_length_;

	void onMouseMove(float x, float y, float dx, float dy) override {
		if (frozen_) {
			return;
		}
		if (look_mode_) {
			if (dx != 0) {
				pan_.setState(Eigen::Vector<float, 1>(pan_.getState()(0) - dx * .004*sensitivity_));
			}
			if (dy != 0) {
				float new_state = tilt_.getState()(0) - dy * .004 * sensitivity_;
				if (new_state <= M_PI * .667 / 2. && new_state >= -M_PI * .8 / 2.) {
					tilt_.setState(Eigen::Vector<float, 1>(new_state));
				}
			}
		}
		//std::cout << "(" << dx << ", " << dy << ")\n";
	}

	void onMouseDown(int key, float x, float y) override {
		if (key == GLFW_MOUSE_BUTTON_MIDDLE) {
			if (control_mode_) {
				look_mode_ = true;
			}
		}
		/*if (abs(x) <= 1 && abs(y) <= 1) {
			glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
			if (glfwRawMouseMotionSupported()) {
				glfwSetInputMode(window_, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
			}
		}*/
	}
	void onMouseUp(int key, float x, float y) override {
		if (key == GLFW_MOUSE_BUTTON_MIDDLE) {
			if (control_mode_) {
				look_mode_ = false;
			}
		}
	}

	void onMouseScroll(float dx, float dy) override {
		if(zoom_enabled_){
			equilibrium_length_ -= dy * .025;
			if (equilibrium_length_ < .125) {
				equilibrium_length_ = .125;
			}
			else if (equilibrium_length_ > 20.) {
				equilibrium_length_ = 20.;
			}
		}
	}

	void onKeyPress(int key) {
		Camera::onKeyPress(key);
		if (key == GLFW_KEY_ESCAPE) {
			glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
			if (glfwRawMouseMotionSupported()) {
				glfwSetInputMode(window_, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
			}
		}
	}

	

public:
	PlayerCamera(float near_clip, float far_clip, float fov, float pixels_width, float pixels_height, float equilibrium_length, bool CAD_mode=false, std::string name=InternalObject::no_name) :
		Camera(near_clip,far_clip,fov,pixels_width,pixels_height,"PlayerCamera"),
		equilibrium_length_(equilibrium_length),
		anchor_(OffsetConnector((Eigen::Matrix4f()<<1.,0.,0.,0.,  0.,1.,0.,.70,  0.,0.,1.,0.,  0., 0., 0., 1.).finished())),
		pan_(RotationJoint(Eigen::Vector3f(0, 1, 0))),
		tilt_(RotationJoint(Eigen::Vector3f(1, 0, 0))),
		dist_(PrismaticJoint(Eigen::Vector3f(0., .0, 2))),
		tether_(ConnectorChain<OffsetConnector, RotationJoint, RotationJoint, PrismaticJoint>(anchor_,pan_,tilt_,dist_)),
		cam_box_("frustum.obj", Model::debug_path),
		look_mode_(true),
		zoom_enabled_(true),
		control_mode_(CAD_mode),
		velocity_cushion_(0.0f),
		sensitivity_(1.0),
		stick_drift_(.1){

		setConnector(&tether_);
		tether_.setRootTransform(nullptr);


	}

	void update(GLFWwindow* window) override {
		Eigen::Vector3f joystick_command = Eigen::Vector3f(InternalObject::getRightStickPosition(window).first, 0, InternalObject::getRightStickPosition(window).second);
		if (joystick_command.norm() > stick_drift_ && !frozen_) {
			joystick_command = joystick_command - .1 * joystick_command.normalized();
			joystick_command = Eigen::Vector3f(joystick_command(0) * abs(joystick_command(0)), 0, joystick_command(2) * abs(joystick_command(2)));
			onMouseMove(0, 0, sensitivity_*joystick_x_sensitivity * joystick_command(0)*getdt(), sensitivity_ * joystick_y_sensitivity * joystick_command(2)*getdt());
		}
		Camera::update(window);
	}

	void onStep() override;

	void enableMouseControl(GLFWwindow* window) {
		window_ = window;
		activateKeyInput(window);
		activateMouseInput(window);
	}

	float getTilt() const {
		return tilt_.getState()(0);
	}
	void setTilt(float tilt)  {
		tilt_.setState(tilt);
		tether_.setState(tether_.getState());
	}
	float getPan() const {
		return pan_.getState()(0);
	}
	void setPan(float pan) {
		pan_.setState(pan);
		tether_.setState(tether_.getState());
	}
	void setStickDrift(float stick_drift) {
		stick_drift_ = stick_drift;
	}
	void setSensitivity(float sensitivity) {
		sensitivity_ = sensitivity;
	}
	void freeze() {
		frozen_ = true;
	}
	void unfreeze() {
		frozen_ = false;
	}

	void disableZoom() {
		zoom_enabled_ = false;
	}

	friend void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);

};

#endif