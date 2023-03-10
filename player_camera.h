#pragma once

#ifndef PUPPET_PLAYERCAMERA
#define PUPPET_PLAYERCAMERA

#include <Eigen/Dense>
#include <cmath>

#include "camera.h"
#include "GameObject.h"

using Eigen::Matrix4f;
using Eigen::Vector3f;

class PlayerCamera : public Camera {
private:
	const float equilibrium_length_;
	const Zmap* bounds_;
	RotationJoint pan_;
	RotationJoint tilt_;
	PrismaticJoint dist_;
	ConnectorChain<RotationJoint, RotationJoint, PrismaticJoint> tether_;

	void onMouseMove(float x, float y, float dx, float dy){
		std::cout << "(" << dx << ", " << dy << ")\n";
	}

	void enableMouseControl(GLFWwindow* window) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
		if (glfwRawMouseMotionSupported()) {
			glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
		}
		activateMouseInput(window);
	}

public:
	PlayerCamera(float equilibrium_length,const Zmap* bounds, GLFWwindow* window, std::string name) :
		Camera("PlayerCamera"),
		equilibrium_length_(equilibrium_length),
		bounds_(bounds),
		pan_(RotationJoint(Eigen::Vector3f(0, 1, 0))),
		tilt_(RotationJoint(Eigen::Vector3f(1, 0, 0))),
		dist_(PrismaticJoint(Eigen::Vector3f(0, 2, 3))),
		tether_(ConnectorChain<RotationJoint, RotationJoint, PrismaticJoint>(pan_,tilt_,dist_)) {
		enableMouseControl(window);
		setConnector(&tether_);
	}

	void update(GLFWwindow* window) override {
		Camera::update(window);
		//kinematics are atrocoious here need to resolve eventually
		//Vector3f parent_pos = getParent()->getPosition()(seq(0, 2), 3);
		//Vector3f current_local_delta = getLocalPosition();
		float damped_return_to_equilibrium = (equilibrium_length_ + 9*tether_.getState()(2)) / 10;
		Eigen::Vector3f damped_equilibrium_state = tether_.getState();
		Eigen::Vector3f no_extension = tether_.getState();
		damped_equilibrium_state(2) = damped_return_to_equilibrium;
		no_extension(2) = 0;
		tether_.setState(no_extension);
		tether_.boundedMove<20>(damped_equilibrium_state,*bounds_);
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

	friend void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
};

#endif