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

	Eigen::Matrix4f position_;
	Eigen::Vector3f velocity_; //yes this could be a twist, but simplicity over technical accuracy
	Eigen::Vector3f acceleration_;
	std::vector<BoundaryConstraint*> motion_constraints_;

	time_point<system_clock> t_ref_;
	std::chrono::duration<float> dt_;
	Eigen::Matrix4f last_position_;

	Surface<3>* hitbox;
	std::vector<const GameObject*> collidors; //should be a safe pointer
	std::vector<bool> collision_flags_;

	const GameObject* parent_;
	PositionConstraint* parent_connector_;


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


	inline virtual void onCollision(const GameObject& other) {};

	inline virtual void onDecollision(const GameObject& other) {};

public:

	virtual void load() {

	}

	virtual void unload() {

	}

	void updatePosition() {
		if (parent_ != nullptr && parent_connector_ != nullptr) {
			this->position_ = parent_->getPosition() * (parent_connector_->getConstraintTransform());
		}
	}
	GameObject(std::string name) :
		position_(Eigen::Matrix4f::Identity()),
		InternalObject(name),
		t_ref_(system_clock::now()),
		parent_(nullptr),
		parent_connector_(nullptr) {

	}

	GameObject():
		position_(Eigen::Matrix4f::Identity()),
		t_ref_(system_clock::now()),
		parent_(nullptr),
		parent_connector_(nullptr) {}

	void update(GLFWwindow* window) override{
		updatePosition();
		//update time
		time_point<system_clock> t = system_clock::now();
		dt_ = (t - t_ref_);
		t_ref_ = t;
		
		//check collisions
		auto collidor_it = collidors.begin();
		auto flag_it = collision_flags_.begin();
		for (int i = 0; i < collidors.size(); i++) {
			const GameObject* c = *collidor_it;
			/*if (this->getHbox().checkCollision(c->getHbox(), getPosition(), c->getPosition())) [[unlikely]] {
				*flag_it = true;
				onCollision(*c);
			}
			else if (*flag_it) [[unlikely]] {//C++20
				*flag_it = false;
				onDecollision(*c);
			}
			else [[likely]] {}
			*/
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

	const GameObject* getParent() const {
		return parent_;
	}

	const Matrix4f& getPosition() const {
		return this->position_;
	}

	const Matrix4f getRelativePosition(GameObject& other) const {
		//this is almost certainly wrong
		return inverseTform(parent_->getPosition()) * other.getPosition() * inverseTform(position_);
		//so globalPosition = parent.gloabl*getRelativePosition(other)*position_ = other.globalPosition
	}

	void setPosition(Eigen::Matrix4f new_position) {
		position_ = new_position;
	}

	void moveTo(Eigen::Vector3f vec) {
		position_(seq(0, 2), 3) = vec;
		//stale_global_position_ = true;
	};
	void moveTo(float x, float y, float z) {
		position_(0, 3) = x;
		position_(1, 3) = y;
		position_(2, 3) = z;
		//stale_global_position_ = true;
	};

	void rotate(Eigen::Matrix3f rot) {
		position_(seq(0, 2), seq(0, 2)) = rot * getPosition()(seq(0, 2), seq(0, 2));
		//stale_global_position_ = true;
	};

	void rotateAxisAngle(Eigen::Vector3f axis, float angle) {
		Matrix3f w_hat;
		w_hat << 0, -axis(2), axis(1),
			axis(2), 0, -axis(0),
			-axis(1), axis(0), 0;
		Matrix3f rot = Matrix3f::Identity() + w_hat * sin(angle) + w_hat * w_hat * (1 - cos(angle));
		rotate(rot);
	};

	void rotateX(float angle) { rotateAxisAngle(Eigen::Vector3f(1, 0, 0), angle); };
	void rotateY(float angle) { rotateAxisAngle(Eigen::Vector3f(0, 1, 0), angle); };
	void rotateZ(float angle) { rotateAxisAngle(Eigen::Vector3f(0, 0, 1), angle); };

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

	virtual void clampTo(const GameObject* parent) {// this has unintuitive behavior
		this->parent_ = parent;
		parent_connector_ = new OffsetConnector(parent_->getPosition(), position_);
	}

	void connectTo(const GameObject* parent, PositionConstraint* connector) {
		this->parent_ = parent;
		parent_connector_ = connector;
		connector->setRootTransform(&parent_->getPosition());
	}
	void connectTo(const GameObject* parent) {
		this->parent_ = parent;
		if (parent_connector_ != nullptr) {
			parent_connector_->setRootTransform(&parent_->getPosition());
		}
	}
	void setConnector(PositionConstraint* connector) {
		this->parent_connector_ = connector;
		connector->setRootTransform(&parent_->getPosition());

	}

	void disconnect() {
		this->parent_ = nullptr;
		this->parent_connector_ = nullptr;
	}

	const PositionConstraint* getConnector() const {
		return parent_connector_;
	}
	bool isHidden() const {
		return hidden_ || (getParent() != nullptr && getParent()->isHidden());
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

	virtual std::string getDebugInfo() const { return ""; };
	virtual void openDebugUI(float bounds_height, float bounds_width, float bounds_top, float bounds_left) {};
	virtual void closeDebugUI(float bounds_height, float bounds_width, float bounds_top, float bounds_left) {};
	
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