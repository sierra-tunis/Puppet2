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

	bool look_mode_;
	const bool control_mode_; //i.e. CAD mode
	static constexpr float joystick_x_sensitivity = 5;
	static constexpr float joystick_y_sensitivity = 5;

	GLFWwindow* window_;

	//Signal damped_tether_length_;

	void onMouseMove(float x, float y, float dx, float dy) override {
		if (look_mode_) {
			if (dx != 0) {
				pan_.setState(Eigen::Vector<float, 1>(pan_.getState()(0) - dx * .004));
			}
			if (dy != 0) {
				float new_state = tilt_.getState()(0) - dy * .004;
				if (new_state <= M_PI * 1.1 / 2. && new_state >= -M_PI * .667 / 2.) {
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
		equilibrium_length_ -= dy * .025;
		if (equilibrium_length_ < .125) {
			equilibrium_length_ = .125;
		}
		else if (equilibrium_length_ > 20.) {
			equilibrium_length_ = 20.;
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
		cam_box_("cam_box.obj", Model::debug_path),
		look_mode_(true),
		control_mode_(CAD_mode){

		setConnector(&tether_);
		tether_.setRootTransform(nullptr);


	}

	void update(GLFWwindow* window) override {
		Eigen::Vector3f joystick_command = Eigen::Vector3f(InternalObject::getRightStickPosition(window).first, 0, InternalObject::getRightStickPosition(window).second);
		if (joystick_command.norm() > .1) {
			onMouseMove(0, 0, joystick_x_sensitivity * joystick_command(0), joystick_y_sensitivity * joystick_command(2));
		}
		Camera::update(window);
	}

	void onStep() override {

		//Camera::update(window);
		float current_extension_ = tether_.getState()(2);
		dist_.setState(Eigen::Vector<float, 1>(0));
		Eigen::Vector<float, 3> equilibrium_state(pan_.getState()(0), tilt_.getState()(0), equilibrium_length_);
		if (getParent() != nullptr) {
			tether_.boundedMove<10>(equilibrium_state, getParent()->getMotionConstraints());
		}
		float new_extension_ = tether_.getState()(2);
		float damped_return_to_equilibrium = (new_extension_ + (19) * current_extension_) / (20);
		dist_.setState(Eigen::Vector<float,1>(damped_return_to_equilibrium));
		tether_.setState(tether_.getState());

		//scale camera mesh by certain ratio 
		
		//float delta_len = getParent()->getPosition()(seq(0, 2), seq(0, 2)) * damped_return_to_equilibrium;
		//float new_len = bounds_->findMaxTravel(getParent()->getPosition(), delta_pos, 0, 0, 10, 100).norm();
		//std::cout << (getParent()->getGlobalPosition()(seq(0, 2), seq(0, 2)).transpose() * (new_pos-parent_pos)).transpose() << "\n";
		//new_len = equilibrium_length_;
		//tether_.setState(Eigen::Vector3f(0,0,new_len));

		/*if (bounds_->getZdata(getGlobalPosition()(seq(0, 2), 3), 0).first.room_id == zdata::BaseRoom) {
			Vector3f new_pos = bounds_->findMaxTravel(parent_pos, getGlobalPosition()(seq(0, 2), 3) - parent_pos, 0, 0, 10, 5);

			this->moveTo(new_pos-parent_pos);
		} else {
			//moveTo((equilibrium_position_(seq(0, 2), 3) + getLocalPosition()(seq(0, 2), 3)) / 2);
		}*/
	}
	void enableMouseControl(GLFWwindow* window) {
		window_ = window;
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
		if (glfwRawMouseMotionSupported()) {
			glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
		}
		activateKeyInput(window);
		activateMouseInput(window);
	}

	float getTilt() const {
		return tilt_.getState()(0);
	}

	friend void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);

};

#endif