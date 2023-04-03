#pragma once

#ifndef PUPPET_DEBUGCAMERA
#define PUPPET_DEBUGCAMERA

#include <Eigen/Dense>

#include "GameObject.h"
#include "GLFW/glfw3.h"
#include "level.h"
#include "collision.hpp"
#include "no_collide_constraint.hpp"

using Eigen::seq;

class DebugCamera : public InterfaceObject<GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D, GLFW_KEY_SPACE, GLFW_KEY_LEFT_SHIFT> {

	//const Level* level_;
	Level* current_level_;
	bool freefall;
	MeshSurface hitbox_;
	NoCollideConstraint<Zmap, MeshSurface> level_bounds_;
	std::vector<bool> collision_info;
	//BoundaryConstraint level_bounds_;


	void onKeyDown(int key) override {
		switch (key) {
		case GLFW_KEY_W:
			//boundedTranslate(this->getPosition()(seq(0, 2), 2) * -.01,current_level_->getZmap(), .5);
			translate(this->getPosition()(seq(0, 2), 2) * -.05);
			break;
		case GLFW_KEY_S:
			translate(this->getPosition()(seq(0, 2), 2) * .05);
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
	DebugCamera(Level* starting_level, std::string name):
	InterfaceObject(name),
	current_level_(starting_level),
	hitbox_("cube.obj"),
	level_bounds_(&current_level_->getZmap(), &current_level_->getPosition(),hitbox_),
	collision_info(hitbox_.getEdges().size())
	//level_bounds_(&current_level_->getZmap())
	{
		setModel(new Model("cube.obj"));
		setTexture(new Texture("obamna.jpg"));
		setAcceleration(Eigen::Vector3f(0, -0.81, 0));
		addMotionConstraint(&level_bounds_);
	}

	void update(GLFWwindow* window) override {
		SurfaceNodeCollision(&current_level_->getZmap(), &hitbox_, getPosition(), &collision_info);
		InterfaceObject::update(window);
		int current_room = current_level_->getZmap().getZdata(getPosition()(seq(0, 2), 3), 0.).first.room_id;
		std::cout << "current room# " << current_room << "\n";
		if (current_room != 1) {
			if (current_room == zdata::BaseRoom) {
				//std::cerr << "fatal out of bounds error!";
				//std::cout << "fatal out of bounds error!";
			} else {
				current_level_->activateNeighbor(current_room - 2);
				current_level_ = current_level_->getNeighbors()[current_room - 2];
			}
		}

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

	const Level* getLevel() const {
		return current_level_;
	}

	const MeshSurface& getHitbox() const {
		return hitbox_;
	}

	const std::vector<bool>& getCollisionInfo() const {
		return collision_info;
	}

};

//const Model DebugCamera::model_ = Model("cube.obj");
//const Texture DebugCamera::texture_ = Texture("obamna.jpg");
//const Hitbox DebugCamera::hbox_ = Hitbox({ 1.,2.,3. });
//Grobj_blinPhong DebugCamera::shared_grobj_ = Grobj_blinPhong(Eigen::Matrix4f::Identity());


#endif // !