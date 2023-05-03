#pragma once

#ifndef PUPPET_DEBUGCAMERA
#define PUPPET_DEBUGCAMERA

#include <Eigen/Dense>

#include "GameObject.h"
#include "GLFW/glfw3.h"
#include "level.h"
#include "collision.hpp"
#include "no_collide_constraint.hpp"

#include "text_graphics.hpp"

using Eigen::seq;

class DebugCamera : public InterfaceObject<GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D, GLFW_KEY_SPACE, GLFW_KEY_LEFT_SHIFT> {

	//const Level* level_;
	const Level* current_level_;
	bool freefall;
	MeshSurface hitbox_;
	NoCollideConstraint<Surface<3>, MeshSurface>* level_bounds_;
	std::vector<bool> collision_info;
	constexpr static float max_step = .5f;
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
			rotateY(.01);
			break;
		case GLFW_KEY_D:
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

	bool fullyOutsideLevel() {
		for (auto& p : getHitbox().getVerts()) {
			if (current_level_->withinLevel(p + getPosition()(seq(0, 2), 3))) {
				return false;
			}
		}
		return true;
	}
protected:
	
	Eigen::Vector3f onInvalidTranslation(Eigen::Vector3f translation, BoundaryConstraint* broken_constraint) {
		Eigen::Vector3f step(0, .33, 0);
		//should rewrite this using bestTranslate since its in the engine part of the code
		Eigen::Vector3f best_translate = GameObject::onInvalidTranslation(translation, broken_constraint);
		if (best_translate != translation) {
			//this makes sense as a logical implementation of step 
			//best_translate = GameObject::onInvalidTranslation(translation + step, broken_constraint);
			best_translate = GameObject::onInvalidTranslation(translation.normalized() * max_step, broken_constraint);
			best_translate = best_translate.normalized() * translation.norm();
		}
		return best_translate;
	}

public:
	DebugCamera( std::string name):
	InterfaceObject(name),
	current_level_(nullptr),
	hitbox_("human_skeleton.obj"),
	level_bounds_(new NoCollideConstraint<Surface<3>, MeshSurface>(nullptr, nullptr,&hitbox_)),
	collision_info(hitbox_.getEdges().size())
	//level_bounds_(&current_level_->getZmap())
	{
		setModel(new Model("human.obj"));
		setTexture(new Texture("obamna.jpg"));
		addMotionConstraint(level_bounds_);
	}

	void onRoomActivation() override {
		current_level_ = Level::getCurrentLevel();
		level_bounds_->setBoundary(current_level_->getCollisionSurface());
		level_bounds_->setBoundaryPosition(&current_level_->getPosition());
		//*level_bounds_ = NoCollideConstraint<Surface<3>, MeshSurface>(current_level_->getCollisionSurface(), &current_level_->getPosition(), &hitbox_);
		//Level::getCurrentLevel()->add(*this);
	}
	void onRoomDeactivation() override {
		//Level::getCurrentLevel()->remove(*this);
	}

	void update(GLFWwindow* window) override {
		SurfaceNodeCollision(current_level_->getCollisionSurface(), &hitbox_, getPosition()-current_level_->getPosition(), &collision_info);
		InterfaceObject::update(window);
		if (fullyOutsideLevel()) {
			int neig_ind = current_level_->neighborAt(getPosition()(seq(0, 2), 3));
			if (neig_ind != -1) {
				// && !checkCollision<Surface<3>, MeshSurface>(current_level_->getNeighbors()[neig_ind]->getCollisionSurface(),&getHitbox(), current_level_->getNeighbors()[neig_ind]->getPosition(),getPosition())
				Level::getCurrentLevel()->goToNeighbor(neig_ind);
				//current_level_ = current_level_->getNeighbors()[neig_ind];
				//*level_bounds_ = NoCollideConstraint<Surface<3>, MeshSurface>(current_level_->getCollisionSurface(), &current_level_->getPosition(), &hitbox_);
				
			}
		}
		/*
		int current_room = current_level_->getZmap().getZdata(, 0.).first.room_id;
		//std::cout << "current room# " << current_room << "\n";
		if (current_room != 1) {
			if (current_room == zdata::BaseRoom) {
				//std::cerr << "fatal out of bounds error!";
				//std::cout << "fatal out of bounds error!";
			} else {
				current_level_->activateNeighbor(current_room - 2);
				current_level_ = current_level_->getNeighbors()[current_room - 2];
			}
		}*/

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

	const std::vector<bool>& getCollisionInfo() const {//this is a terrible way of doing this
		return collision_info;
	}

	std::string getDebugInfo() const override {
		const std::pair<zdata, zdata>& zdata_pair = static_cast<const Zmap*>(static_cast<const void*>(current_level_->getCollisionSurface()))->getZdata(getPosition()(seq(0, 2), 3)-current_level_->getPosition()(seq(0,2),3), 0.);
		const zdata& below = zdata_pair.first;
		const zdata& above = zdata_pair.second;
		std::string ret_str = "z below: " + std::to_string(below.z) + "\n" +
			"z above: " + std::to_string(above.z) + "\n" +
			"current room: " + std::to_string(below.room_id) + "\n"+
			"within current room?:" + (current_level_->withinLevel(getPosition()(seq(0,2),3))?" Yes\n":" No\n");
		int i = 0;
		for (const auto& neig : current_level_->getNeighbors()) {
			ret_str += "within neighbor " + std::to_string(i) + "?:" + (neig->withinLevel(getPosition()(seq(0, 2), 3)) ? " Yes\n" : " No\n");
			i++;
		}
		//ret_str += "\n\nFPS: " + std::to_string(1. / getdt());
		return ret_str;
	}

};

//const Model DebugCamera::model_ = Model("cube.obj");
//const Texture DebugCamera::texture_ = Texture("obamna.jpg");
//const Hitbox DebugCamera::hbox_ = Hitbox({ 1.,2.,3. });
//Grobj_blinPhong DebugCamera::shared_grobj_ = Grobj_blinPhong(Eigen::Matrix4f::Identity());


#endif // !