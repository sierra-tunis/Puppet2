#pragma once

#ifndef PUPPET_DEBUGCAMERA
#define PUPPET_DEBUGCAMERA

#include <Eigen/Dense>

#include "GameObject.h"
#include "GLFW/glfw3.h"
#include "level.h"
#include "collision.hpp"
#include "no_collide_constraint.hpp"
#include "dynamic_model.hpp"

#include "text_graphics.hpp"

using Eigen::seq;

class DebugCamera : public InterfaceObject<GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D, GLFW_KEY_SPACE, GLFW_KEY_LEFT_SHIFT> {

	const Level* current_level_;
	bool freefall;
	MeshSurface hitbox_;
	NoCollideConstraint<Surface<3>, MeshSurface>* level_bounds_;
	std::vector<bool> collision_info;
	constexpr static float max_step = .5f;

	Eigen::Matrix4f test_tform_;

	void onKeyDown(int key) override {
		switch (key) {
		case GLFW_KEY_W:
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

	void onCollision(const GameObject* other, const CollisionPairBase* collision) override {
		std::cout << "colliding with: " << other->getName() <<"\n";
	}

protected:
	
	Eigen::Vector3f onInvalidTranslation(Eigen::Vector3f translation, BoundaryConstraint* broken_constraint) override {
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
	hitbox_("small_cube.obj", Model::debug_path),
	level_bounds_(new NoCollideConstraint<Surface<3>, MeshSurface>(nullptr, nullptr,&hitbox_)),
	collision_info(hitbox_.getEdges().size()),
	test_tform_(Eigen::Matrix4f::Identity())
	//level_bounds_(&current_level_->getZmap())
	{
		/*std::vector<const Eigen::Matrix4f*> vert_tforms;
		for (int i = 0; i < kin_model_.glen(); i++) {
			vert_tforms.push_back(i % 2 == 0 ? &getPosition() : &test_tform_);
		}
		kin_model_.setVertTforms(vert_tforms);
		kin_model_.offsetVerts();*/
		setModel(new Model("small_cube.obj", Model::debug_path));
		//setModel(new Model("human.obj"));

		setTexture(new Texture("puppet_button.jpg", Texture::debug_path));
		addMotionConstraint(level_bounds_);
	}

	void onRoomActivation() override {
		current_level_ = Level::getCurrentLevel();
		NoCollideConstraint<Surface<3>, MeshSurface> new_bounds(current_level_->getCollisionSurface(), &current_level_->getPosition(), &hitbox_);
		if (new_bounds.breaksConstraint(getPosition(), getPosition())) {
			Level::goToPrevLevel();
			return;
		}
		//level_bounds_->setBoundary(current_level_->getCollisionSurface());
		//level_bounds_->setBoundaryPosition(&current_level_->getPosition());
		*level_bounds_ = new_bounds;
	}
	void onRoomDeactivation() override {
		//Level::getCurrentLevel()->remove(*this);
	}

	void onStep() override {
		SurfaceNodeCollision(current_level_->getCollisionSurface(), &hitbox_, getPosition()-current_level_->getPosition(), &collision_info);
		if (fullyOutsideLevel()) {
			int neig_ind = current_level_->neighborAt(getPosition()(seq(0, 2), 3));
			if (neig_ind != -1) {
				Level::getCurrentLevel()->goToNeighbor(neig_ind);
			}
		}
		//kin_model_.updateData();
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
		return ret_str;
	}

};

#endif