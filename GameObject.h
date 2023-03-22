#pragma once
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
#include "zmap.h"

#ifndef PUPPET_GAMEOBJECT
#define PUPPET_GAMEOBJECT

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
	static const Model model_; //model is all the model data in one place and can be subclassed for other shader types
	static const Texture texture_; //texture has all the texture packed into it like color, normal etc, and can just be subclassed to add more

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
		if (last_position_(seq(0,2),3) != getPosition()(seq(0,2),3)) {
			for (auto m_c : motion_constraints_) {
				Eigen::Vector3f delta_pos = getPosition()(seq(0, 2), 3) - last_position_(seq(0, 2), 3);
				Eigen::Vector3f normal;
				Eigen::Vector3f binormal;
				if (delta_pos.dot(getPosition()(seq(0, 2), 1)) == delta_pos.norm()) {
					normal = getPosition()(seq(0, 2), 2);
					binormal = getPosition()(seq(0, 2), 0);
				}
				else {
					normal = delta_pos.cross(Eigen::Vector3f(getPosition()(seq(0, 2), 1)));
					binormal = delta_pos.cross(normal);
					normal.normalize();
					binormal.normalize();
				}
				moveTo(last_position_(seq(0, 2), 3) + m_c->bestTranslate(last_position_, delta_pos, normal, binormal));
				last_position_ = getPosition();
			}
		}
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

	void boundedTranslate(const Eigen::Vector3f& vec, const Zmap& bounds, float max_step) {
		float height = getHbox().getShape()(1);
		float width = getHbox().getShape()(0);
		this->moveTo(bounds.getNewPosition(this->getPosition()(seq(0,2),3), vec, max_step,height,width,.5, freefall_));
	}

	virtual const Model& getModel() const { //these are virtual to allow for unique model/texture instances, i.e. level
		return model_;
	}

	virtual const Texture& getTexture() const {
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

	bool isInFreefall() const {
		return freefall_;
	}

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