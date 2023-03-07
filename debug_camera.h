#pragma once

#include <Eigen/Dense>

#include "GameObject.h"
#include "GLFW/glfw3.h"
#include "level.h"

#ifndef PUPPET_DEBUGCAMERA
#define PUPPET_DEBUGCAMERA

using Eigen::seq;

class DebugCamera : public InterfaceObject<GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D, GLFW_KEY_SPACE, GLFW_KEY_LEFT_SHIFT> {

	//const Level* level_;
	const Zmap* floor_;
	bool freefall;


	void onKeyDown(int key) override {
		switch (key) {
		case GLFW_KEY_W:
			boundedTranslate(this->getPosition()(seq(0, 2), 2) * -.01,*floor_,.5);
			break;
		case GLFW_KEY_S:
			boundedTranslate(this->getPosition()(seq(0, 2), 2) * +.01, *floor_, .5);
			break;
		case GLFW_KEY_A:
			rotateY(.005);
			break;
		case GLFW_KEY_D:
			rotateY(-.005);
			break;
		case GLFW_KEY_SPACE:
			translate(this->getPosition()(seq(0, 2), 1) * .01);
			break;
		case GLFW_KEY_LEFT_SHIFT:
			translate(this->getPosition()(seq(0, 2), 1) * -.01);
			break;
		}
	}
public:
	DebugCamera(const Zmap& zmap, int id):
	InterfaceObject(Eigen::Matrix4f::Identity(),id),
	floor_(&zmap){
		setAcceleration(Eigen::Vector3f(0, -0.81, 0));
	}

	void update(GLFWwindow* window) override {
		InterfaceObject::update(window);
		/*if (isInFreefall()) {
			getVelocity() += getAcceleration() * getdt();
			boundedTranslate(getVelocity() * getdt(),*floor_,.1);
		}
		else {
			//not efficient
			setVelocity(Eigen::Vector3f(0,0,0));
		}*/
		/*
		std::pair<zdata, zdata> tmp = floor_->getZdata(getLocalPosition()(seq(0, 2), 3), .5);
		zdata floor = tmp.first;
		zdata ceiling = tmp.second;
		if (floor.room_id != 0) {
			Eigen::Vector3f floor_pos;
			floor_pos << getLocalPosition()(0, 3), floor.z, getLocalPosition()(2, 3);
			//floor_pos(1) = floor_->getZdata(floor_pos).z + .1;
			moveTo(floor_pos);
		}*/
	}

};

//const Model DebugCamera::model_ = Model("cube.obj");
//const Texture DebugCamera::texture_ = Texture("obamna.jpg");
//const Hitbox DebugCamera::hbox_ = Hitbox({ 1.,2.,3. });
//Grobj_blinPhong DebugCamera::shared_grobj_ = Grobj_blinPhong(Eigen::Matrix4f::Identity());


#endif // !