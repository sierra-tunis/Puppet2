#pragma once

#ifndef PUPPET_GAMEOBJECT
#define PUPPET_GAMEOBJECT

#include <Eigen/Dense>
#include <chrono>
#include <vector>
#include <array>
#include <unordered_map>
#include <glad/glad.h>


#include "Model.h"
#include "Texture.h"
#include "InternalObject.h"
#include "motion_constraint.h"


using Eigen::Matrix4f;
using Eigen::Matrix3f;
using Eigen::seq;

using std::chrono::microseconds;
using std::chrono::time_point;
using std::chrono::system_clock;
using std::chrono::duration_cast;

class GameObject: public InternalObject {

private:

	//this doesnt work?? this would cause all game objects to have the same texture?? only reason the map has a different texture is
	//because it has a texture member
	Model* model_; //model is all the model data in one place and can be subclassed for other shader types
	Texture* texture_; //texture has all the texture packed into it like color, normal etc, and can just be subclassed to add more
	bool hidden_;

	Eigen::Vector3f velocity_; //yes this could be a twist, but simplicity over technical accuracy
	Eigen::Vector3f acceleration_;
	std::vector<BoundaryConstraint*> motion_constraints_;
	bool freefall_;

	time_point<system_clock> t_ref_;
	std::chrono::duration<float> dt_;
	Eigen::Matrix4f last_position_;

	std::vector<const InternalObject*> collidors; //should be a safe pointer
	std::vector<bool> collision_flags_;

	/*
	variable which contains information about constrainst on motion relative to parent
	*/

protected:
	//virtual G makeGrobj(G shared_grobj) {}
	void setModel(Model* model) {
		model_ = model;
	}

	void setTexture(Texture* tex) {
		texture_ = tex;
	}

	virtual Eigen::Vector3f onInvalidTranslation(Eigen::Vector3f translation, BoundaryConstraint* broken_constraint) {
		//motion constraint::bestTranslate/limitTranslate will NEVER return an invalid translation, however if the
		//user wants to perform some chikanery here and decide to do something else they are allowed
		Eigen::Vector3f normal;
		Eigen::Vector3f binormal;
		if (translation.dot(getPosition()(seq(0, 2), 1)) == translation.norm()) {
			normal = getPosition()(seq(0, 2), 2);
			binormal = getPosition()(seq(0, 2), 0);
		}
		else {
			normal = translation.cross(Eigen::Vector3f(getPosition()(seq(0, 2), 1)));
			binormal = translation.cross(normal);
			normal.normalize();
			binormal.normalize();
		}
		return broken_constraint->bestTranslate(getPosition(), translation, normal, binormal);
	}

	virtual void onInvalidConstraintChange(BoundaryConstraint* bc) {

	}

public:
	
	GameObject(std::string name) :
		InternalObject(name),
		last_position_(getPosition()),
		//shared_grobj_(GameObject::model_, GameObject::texture_, Matrix4f::Identity()),//still needs rework
		t_ref_(system_clock::now()) {
		/*if (!shared_grobj_.isInitialized()) {
			shared_grobj_.initialize(GameObject::model_, GameObject::texture_);
		}
		grobj_.VAO = shared_grobj_.VAO;
		grobj_.tex_id = shared_grobj_.tex_id;
		grobj_.n_elems = shared_grobj_.n_elems;
		this->graphics_->add(this->getID(), this->grobj_);
		*/
	}

	void update(GLFWwindow* window) override{
		InternalObject::update(window);
		//update time
		time_point<system_clock> t = system_clock::now();
		dt_ = (t - t_ref_);
		t_ref_ = t;
		
		//check collisions
		auto collidor_it = collidors.begin();
		auto flag_it = collision_flags_.begin();
		for (int i = 0; i < collidors.size(); i++) {
			const InternalObject* c = *collidor_it;
			if (this->getHbox().checkCollision(c->getHbox(), getPosition(), c->getPosition())) [[unlikely]] {
				*flag_it = true;
				onCollision(*c);
			}
			else if (*flag_it) [[unlikely]] {//C++20
				*flag_it = false;
				onDecollision(*c);
			}
			else [[likely]] {}

			collidor_it++;
			flag_it++;
		}
		/*if (last_position_(seq(0, 2), 3) != getPosition()(seq(0, 2), 3)) {
			for (auto m_c : motion_constraints_) {
				
			}
		}*/
		//move according to velocity and acceleration
		/*if (freefall_) {
			velocity_ += acceleration_ * dt_.count();
			translate(velocity_ * dt_.count());
		}
		else {
			//not efficient
			velocity_ *= 0;
		}*/
	}


	void addCollidor(const GameObject& other) {
		this->collidors.emplace_back(&other);
		this->collision_flags_.emplace_back(false);
	}

	float getdt() const {
		return dt_.count();
	}//note this is a copy, not a ref

	/*
	void boundedTranslate(const Eigen::Vector3f& vec, const Zmap& bounds, float max_step) {
		float height = getHbox().getShape()(1);
		float width = getHbox().getShape()(0);
		this->moveTo(bounds.getNewPosition(this->getPosition()(seq(0,2),3), vec, max_step,height,width,.5, freefall_));
	}*/

	virtual const Model* getModel() const { //these are virtual to allow for unique model/texture instances, i.e. level
		return model_;
	}

	virtual const Texture* getTexture() const {
		return texture_;
	}
	
	void setVelocity(Eigen::Vector3f velocity) {
		velocity_ = velocity;
	}

	Eigen::Vector3f getVelocity() const {
		return velocity_;
	}

	void setAcceleration(Eigen::Vector3f acceleration) {
		acceleration_ = acceleration;
	}
	
	Eigen::Vector3f getAcceleration() const {
		return acceleration_;
	}

	void addMotionConstraint(BoundaryConstraint* BC) {
		motion_constraints_.push_back(BC);
	}

	const std::vector<BoundaryConstraint*>& getMotionConstraints() const {
		return motion_constraints_;
	}

	bool isInFreefall() const {
		return freefall_;
	}

	bool isHidden() const {
		return hidden_;
	}

	void toggleHidden() {
		hidden_ = !hidden_;
	}

	void setHideState(bool is_hidden) {
		hidden_ = is_hidden;
	}

	void hide() {
		hidden_ = true;
	}
	void show() {
		hidden_ = false;
	}


	void translate(Eigen::Vector3f vec) {
		Eigen::Matrix4f new_pos = getPosition();
		for (auto m_c : motion_constraints_) {
			new_pos(seq(0,2),3) = getPosition()(seq(0,2),3) + vec;
			if (m_c->breaksConstraint(getPosition(), new_pos)) {
				vec = onInvalidTranslation(vec, m_c);
			}
		}
		moveTo(getPosition()(seq(0,2),3) + vec);
		//stale_global_position_ = true;
	};
	void translate(float dx, float dy, float dz) {
		translate(Eigen::Vector3f(dx, dy, dz));
		//stale_global_position_ = true;
	};

	

	//needs to be reworked
	/*void moveToGlobal(Eigen::Vector3f vec) {
		//parent*new_pos = vec
		//new_pos = parent^-1*vec
		//global = parent*local
		//parent = global*local^-1
		Eigen::Matrix4f tmp = inverseTform(position_ * inverseTform(position_));
		position_(seq(0, 2), 3) = tmp(seq(0, 2), seq(0, 2)) * vec + tmp(seq(0, 2), 3);
	}*/

};
//template <class G, int ... Keys>
//const std::array<int, sizeof(Keys)> GameObject<G, Keys...>::keys{ { Keys... } };

//template <class G, int ... Keys>
//const G GameObject<G, Keys...>::shared_grobj_(GameObject::model_, GameObject::texture_, NULL);


template <int ... Keys> //this has to change eventually i think
class InterfaceObject : public GameObject {

private:
	static constexpr std::array<int, sizeof...(Keys)> keys{ { Keys... } };

public:
	//poll inputs
	void update(GLFWwindow* window) override {
		//poll inputs first
		for (int k : keys) {
			if (glfwGetKey(window, k) == GLFW_PRESS) {
				this->onKeyDown(k);
			}
			else if (glfwGetKey(window, k) == GLFW_RELEASE) {
				this->onKeyUp(k);
			}
		}
		this->onStep();
		//this way inputs are parsed with main() pollInputs input parsing
		//then update GameObject
		GameObject::update(window);

	}
	InterfaceObject(std::string name) :
		GameObject(name){}
};



#endif