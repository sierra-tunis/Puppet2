#pragma once

#ifndef PUPPET_DEBUGGER
#define PUPPET_DEBUGGER

#include <stdio.h>
#include <iostream>

#include "GameObject.h"
#include <GLFW/glfw3.h>


class Debugger : public InterfaceObject<GLFW_KEY_UP, GLFW_KEY_DOWN, GLFW_KEY_LEFT, GLFW_KEY_RIGHT> {
	OffsetConnector clamp_;
	RotationJoint r1_;
	RotationJoint r2_;
	ConnectorChain<OffsetConnector, RotationJoint, RotationJoint> linkage_;
public:
	Debugger(Eigen::Matrix4f position, std::string name, const InternalObject& parent) : 
		InterfaceObject(name),
		clamp_(position),
		r1_(Eigen::Vector3f(0,1,0)),
		r2_(Eigen::Vector3f(1, 0, 0)),
		linkage_(clamp_, r1_, r2_){
		this->connectTo(&parent,&linkage_);

		setModel(new Model("cube.obj"));
		setTexture(new Texture("obamna.jpg"));

	}

	/*
	const HboxGrobj& getHboxDbgGrobj() const {
		return HboxGrobj(this->getPosition(), &this->getHbox(), &this->getCollisionFlag());
	}*/

private:

	bool collision_flag_;

	inline void onStep() {
	};

	void onKeyDown(int key) {
		//std::cout << key << " key is pressed!\n";
		Eigen::Matrix3f r_x,r_y;

		switch (key) {
		case GLFW_KEY_LEFT:
			r_y << cos(.001), 0, sin(.001),
				0, 1, 0,
				-sin(.001), 0, cos(.001);

			rotate(r_y);
			break;
		case GLFW_KEY_RIGHT:
			r_y << cos(.001), 0, sin(.001),
				0, 1, 0,
				-sin(.001), 0, cos(.001);

			rotate(r_y.transpose());
			break;
		case GLFW_KEY_DOWN:
			r_x << 1, 0, 0,
				0, cos(.001), sin(.001),
				0, -sin(.001), cos(.001);

			rotate(r_x);
			break;
		case GLFW_KEY_UP:
			r_x << 1, 0, 0,
				0, cos(.001), sin(.001),
				0, -sin(.001), cos(.001);

			rotate(r_x.transpose());
			break;

		}
	}

	void onCollision(const InternalObject& other) override {
		this->collision_flag_= true;//grobj is copied into graphics, not passed by reference so this doesnt work
	}

	void onKeyUp(int key) {
		//std::cout << key << " key is released!\n";
	}

	const bool& getCollisionFlag() const {
		return collision_flag_;
	}

};


#endif