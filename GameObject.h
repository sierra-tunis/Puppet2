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
#include "collision.hpp"
#include "motion_constraint.h"
#include "graphics_raw.hpp"
#include "text.hpp"
#include "interface.hpp"
#include "animation.hpp"


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
	std::unordered_set<AnimationBase*> animations_;
	AnimationBase* active_animation_;
	//inherited from parent (might want to make seperate "visibility_parent_")
	bool hidden_;

	Eigen::Matrix4f position_;
	Eigen::Vector3f velocity_; //yes this could be a twist, but simplicity over technical accuracy
	Eigen::Vector3f acceleration_;
	std::vector<BoundaryConstraint*> motion_constraints_;
	Eigen::Matrix4f initial_position_;

	time_point<system_clock> t_ref_;
	std::chrono::duration<float> dt_;
	Eigen::Matrix4f last_position_;

	Surface<3>* hitbox;
	std::unordered_map<const GameObject*,CollisionPairBase*> collidors_; //should be a safe pointer
	std::unordered_map<const GameObject*, bool> collision_flags_;
	bool active_hitbox_;

	const GameObject* parent_;
	PositionConstraint* connector_;

	std::unordered_set<GameObject*> dependents_; //dependents get updated and modified by this object

	GameObject* position_display_;

	static float global_game_speed_;

protected:
	
	void addAnimation(AnimationBase* animation) {
		animations_.insert(animation);
		//animation->load();
	}
	void removeAnimation(AnimationBase* animation) {
		animations_.erase(animation);
	}


	void setActiveAnimation(AnimationBase* animation) {
		active_animation_ = animation;
	}

	virtual Eigen::Vector3f onInvalidTranslation(Eigen::Vector3f translation, BoundaryConstraint* broken_constraint) {
		//motion constraint::bestTranslate/limitTranslate will NEVER return an invalid translation, however if the
		//user wants to perform some chikanery here and decide to do something else they are allowed
		Eigen::Vector3f normal;
		Eigen::Vector3f binormal;
		if (abs(translation.dot(getPosition()(seq(0, 2), 1))) == translation.norm()) {
			//normal = getPosition()(seq(0, 2), 2);
			//binormal = getPosition()(seq(0, 2), 0);
			return broken_constraint->limitTranslate(getPosition(), translation);
		}
		else {
			normal = translation.cross(Eigen::Vector3f(getPosition()(seq(0, 2), 1)));
			binormal = translation.cross(normal);
			normal.normalize();
			binormal.normalize();
		}
		return broken_constraint->bestTranslate(getPosition(), translation, normal, binormal);
	}

	virtual Eigen::Matrix3f onInvalidRotation(Eigen::Matrix3f rotation, BoundaryConstraint* broken_constraint) {
		return Eigen::Matrix3f::Identity();
	}

	virtual void onInvalidConstraintChange(BoundaryConstraint* bc) {

	}


	inline virtual void onCollision(const GameObject* other, const CollisionPairBase* collision) {};

	inline virtual void onDecollision(const GameObject* other, const CollisionPairBase* collision) {};

	inline virtual void whileCollision(const GameObject* other, const CollisionPairBase* collision) {};

	//inline virtual void onAnimationEnd(const AnimationBase* animation) {}

	const std::unordered_set<GameObject*>& getDependents() const {
		return dependents_;
	}

	inline virtual void onAnimationEnd(AnimationBase* animation){}

	virtual void initialize(std::string init_string, std::string* remainder_string) {
		Eigen::Matrix4f init_position = Eigen::Matrix4f::Identity();
		int i = 0;
		std::string elem_value;
		std::stringstream ss(init_string);
		while (i < 16 && std::getline(ss, elem_value, '\t')) {
			init_position(i / 4, i % 4) = std::stof(elem_value);
			i++;
		}
		setInitialPosition(init_position);
		setPosition(initial_position_);
		std::getline(ss, *remainder_string, '\0');//rest of string

	}

public:

	static void setGlobalGameSpeed(float game_speed) {
		global_game_speed_ = game_speed;
	}
	
	static std::unordered_set<GameObject*> global_game_objects;

	void updatePosition() {
		if (connector_ != nullptr) {
			connector_->refresh();
			this->position_ = connector_->getEndTransform();
		}
	}
	GameObject(std::string name=InternalObject::no_name, const KeyStateCallback_base& key_state_callback_caller=InternalObject::no_key_state_callback) :
		position_(Eigen::Matrix4f::Identity()),
		InternalObject(name, key_state_callback_caller),
		t_ref_(system_clock::now()),
		parent_(nullptr),
		connector_(nullptr),
		active_hitbox_(true){

	}
	/*GameObject(std::string name) :
		position_(Eigen::Matrix4f::Identity()),
		InternalObject(name),
		t_ref_(system_clock::now()),
		parent_(nullptr),
		parent_connector_(nullptr) {
	}*/


	void update(GLFWwindow* window) override {
		InternalObject::update(window); //should rename this function eventually
		updatePosition();
		//update time
		time_point<system_clock> t = system_clock::now();
		dt_ = (t - t_ref_);
		t_ref_ = t;
		
		//check collisions
		for (auto& collidor : collidors_) {
			if (active_hitbox_ && collidor.first->active_hitbox_ && collidor.second->isCollision(getPosition(),collidor.first->getPosition())) {
				collidor.second->fullCollisionInfo(getPosition(), collidor.first->getPosition());
				if (collision_flags_.at(collidor.first) == false) {
					//is colliding, hasnt called onCollision
					onCollision(collidor.first, collidor.second);
					collision_flags_.at(collidor.first) = true;
				} else {
					//is collidiing, has called onCollision
					whileCollision(collidor.first, collidor.second);
				}
			} else {
				if (collision_flags_.at(collidor.first) == true) {
					//isnt colliding, has called onCllision
					onDecollision(collidor.first, collidor.second);
					collision_flags_.at(collidor.first) = false;
				} else {
					//isnt colliding hasn't called onCollision
				}

			}
		}

		//update animations
		/*for (AnimationBase* animation : animations_) {
			if (animation != nullptr) {
				animation->advance(getdt());
			}
		}*/
		if (active_animation_ != nullptr) {
			bool animation_over = false;
			active_animation_->advance(getdt(),&animation_over);
			if (animation_over) {
				onAnimationEnd(active_animation_);
			}
		}
		//perform user code
		onStep();
		for (auto& child : dependents_) {//since dependenets DEPEND on parent state they should update after

			child->update(window);
		}

	}


	void addCollisionPair(const GameObject* other, CollisionPairBase* collision_pair) {
		this->collidors_.insert({ other,collision_pair });
		this->collision_flags_.insert({ other,false });
		//this->collision_flags_.emplace_back(false);
	}
	void removeCollisionPair(const GameObject* other) {
		this->collidors_.erase(other);
		this->collision_flags_.erase(other);
		//this->collision_flags_.emplace_back(false);
	}

	template<PrimaryHitbox PrimaryHitbox_T, SecondaryHitbox<PrimaryHitbox_T> SecondaryHitbox_T>
	void addCollidor(const PrimaryHitbox_T& primary_hbox, const SecondaryHitbox_T& secondary_hbox, GameObject* other) {
		addCollisionPair(new CollisionPair<PrimaryHitbox_T, SecondaryHitbox_T>(primary_hbox, secondary_hbox, this, other));
	}

	float getdt() const {
		//below 100 fps, game will just slow down
		return std::min(.01f, dt_.count()*global_game_speed_);
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
	void setParent(const GameObject* parent) {
		parent_ = parent;
	}

	const std::unordered_set<AnimationBase*>& getAnimations() const {
		return animations_;
	}

	const AnimationBase* getActiveAnimation() const {
		return active_animation_;
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
	}
	void moveTo(float x, float y, float z) {
		position_(0, 3) = x;
		position_(1, 3) = y;
		position_(2, 3) = z;
		//stale_global_position_ = true;
	}

	void rotateTo(Eigen::Matrix3f rot) {
		position_(seq(0, 2), seq(0, 2)) = rot;
	}

	void rotate(Eigen::Matrix3f rot) {
		Eigen::Matrix4f new_pos = getPosition();
		for (auto m_c : motion_constraints_) {
			new_pos(seq(0, 2), seq(0,2)) = rot * getPosition()(seq(0, 2), seq(0, 2));
			if (m_c->breaksConstraint(getPosition(), new_pos)) {
				rot = onInvalidRotation(rot, m_c);
			}
		}
		rotateTo(rot * getPosition()(seq(0, 2), seq(0, 2)));
		//position_(seq(0, 2), seq(0, 2)) = rot * getPosition()(seq(0, 2), seq(0, 2));
		//stale_global_position_ = true;
	}

	void rotateAxisAngle(Eigen::Vector3f axis, float angle) {
		Matrix3f w_hat;
		w_hat << 0, -axis(2), axis(1),
			axis(2), 0, -axis(0),
			-axis(1), axis(0), 0;
		Matrix3f rot = Matrix3f::Identity() + w_hat * sin(angle) + w_hat * w_hat * (1 - cos(angle));
		rotate(rot);
	}

	void rotateX(float angle) { rotateAxisAngle(Eigen::Vector3f(1, 0, 0), angle); };
	void rotateY(float angle) { rotateAxisAngle(Eigen::Vector3f(0, 1, 0), angle); };
	void rotateZ(float angle) { rotateAxisAngle(Eigen::Vector3f(0, 0, 1), angle); };

	//deprecated
	void setOrientation(Eigen::Matrix3f orientation) {
		rotateTo(orientation);
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

	void setInitialPosition(Eigen::Matrix4f initial_position) {
		initial_position_ = initial_position;
	}
	Eigen::Matrix4f getInitialPosition() const {
		return initial_position_;
	}

	void addMotionConstraint(BoundaryConstraint* BC) {
		motion_constraints_.push_back(BC);
	}

	const std::vector<BoundaryConstraint*>& getMotionConstraints() const {
		return motion_constraints_;
	}

	virtual void clampTo(const GameObject* parent) {// this has unintuitive behavior
		this->parent_ = parent;
		connector_ = new OffsetConnector(parent_->getPosition(), position_);
		if (parent != nullptr) {
			connector_->setRootTransform(&parent->getPosition());
		}
	}

	void connectTo(const GameObject* parent, PositionConstraint* connector) {
		this->parent_ = parent;
		connector_ = connector;
		if (parent != nullptr) {
			connector->setRootTransform(&parent_->getPosition());
		}
	}

	void connectTo(const GameObject* parent) {
		this->parent_ = parent;
		if (connector_ != nullptr && parent_ != nullptr) {
			connector_->setRootTransform(&parent_->getPosition());
		}
	}

	void setConnector(PositionConstraint* connector) {
		this->connector_ = connector;
	}
	void setConnectorBase(const Eigen::Matrix4f* base) {
		connector_->setRootTransform(base);
	}

	void disconnect() {
		this->parent_ = nullptr;
		this->connector_ = nullptr;
	}

	const PositionConstraint* getConnector() const {
		return connector_;
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

	//these return an error because we are relying on a function call in another "Module"
	//and also information cant be extracted from void const functions so I should 
	//future proof this with error checking
	/*GraphicsRaw<GameObject>::Error draw(GraphicsRaw<GameObject>* shader) const {
		return shader->add(*this);
	}*/
	void draw(GraphicsRaw<GameObject>* shader) const {
		shader->add(*this);
	}
	void drawFamily(GraphicsRaw<GameObject>* shader) const {
		draw(shader);
		for (auto& child : getDependents()) {
			child->drawFamily(shader);
		}
	}
	void erase(GraphicsRaw<GameObject>* shader) const {
		shader->unload(*this);
	}
	void eraseFamily(GraphicsRaw<GameObject>* shader) const {
		for (auto& child : getDependents()) {
			child->eraseFamily(shader);
		}
		erase(shader);//undraws in reverse order
	}

	void translate(Eigen::Vector3f vec) {
		Eigen::Matrix4f new_pos = getPosition();
		for (auto m_c : motion_constraints_) {
			new_pos(seq(0,2),3) = getPosition()(seq(0,2),3) + vec;
			if (m_c->breaksConstraint(getPosition(), new_pos)) {
				vec = onInvalidTranslation(vec, m_c);
			}
		}
		//ensure invalid translate vec is still valid
		new_pos(seq(0, 2), 3) = getPosition()(seq(0, 2), 3) + vec;
		for (auto m_c : motion_constraints_) {
			if (m_c->breaksConstraint(getPosition(), new_pos) || m_c->breaksConstraint(new_pos, getPosition())) {
				return;
			}
		}
		moveTo(new_pos(seq(0,2),3));
		//stale_global_position_ = true;
	};
	void translate(float dx, float dy, float dz) {
		translate(Eigen::Vector3f(dx, dy, dz));
		//stale_global_position_ = true;
	};

	virtual std::string getDebugInfo() const {
		const Eigen::Matrix4f& M = getPosition();
		constexpr char fs[] = "{:.3}";
		return std::format(fs, M(0, 0)) + "\t " + std::format(fs, M(0, 1)) + "\t " + std::format(fs, M(0, 2)) + "\t " + std::format(fs, M(0, 3)) + "\n" +
			std::format(fs, M(1, 0)) + "\t " + std::format(fs, M(1, 1)) + "\t " + std::format(fs, M(1, 2)) + "\t " + std::format(fs, M(1, 3)) + "\n" +
			std::format(fs, M(2, 0)) + "\t " + std::format(fs, M(2, 1)) + "\t " + std::format(fs, M(2, 2)) + "\t " + std::format(fs, M(2, 3)) + "\n" +
			std::format(fs, M(3, 0)) + "\t " + std::format(fs, M(3, 1)) + "\t " + std::format(fs, M(3, 2)) + "\t " + std::format(fs, M(3, 3)) + "\n"; 
	}
	//this is a terrible way of doing this since there is just total boilerplate code where you have to add all new objects to UI_container
	//also if the object is deleted, the UI elements associated with it would also be deleted???
	virtual void openDebugUI(const GameObject* UI_container, GLFWwindow* window, GraphicsRaw<GameObject>& graphics_2d, GraphicsRaw<Textbox>& text_graphics);
	virtual void closeDebugUI(const GameObject* UI_container, GLFWwindow* window, GraphicsRaw<GameObject>& graphics_2d, GraphicsRaw<Textbox>& text_graphics);
	

	void addDependent(GameObject* child) {
		dependents_.insert(child);
	}
	void removeDependent(GameObject* child) {
		dependents_.erase(child);
	}

	
	virtual void initialize(std::string init_string) {
		Eigen::Matrix4f init_position = Eigen::Matrix4f::Identity();
		int i = 0;
		std::string elem_value;
		std::stringstream ss(init_string);
		while (std::getline(ss, elem_value, '\t') && i < 16) {
			init_position(i / 4, i % 4) = std::stof(elem_value);
			i++;
		}
		setInitialPosition(init_position);
		setPosition(initial_position_);
	}

	virtual std::string save() const {
		const Eigen::Matrix4f& M = getInitialPosition();
		constexpr char fs[] = "{:.6}";
		return std::format(fs, M(0, 0)) + "\t" + std::format(fs, M(0, 1)) + "\t" + std::format(fs, M(0, 2)) + "\t" + std::format(fs, M(0, 3)) + "\t" +
			std::format(fs, M(1, 0)) + "\t" + std::format(fs, M(1, 1)) + "\t" + std::format(fs, M(1, 2)) + "\t" + std::format(fs, M(1, 3)) + "\t" +
			std::format(fs, M(2, 0)) + "\t" + std::format(fs, M(2, 1)) + "\t" + std::format(fs, M(2, 2)) + "\t" + std::format(fs, M(2, 3)) + "\t" +
			std::format(fs, M(3, 0)) + "\t" + std::format(fs, M(3, 1)) + "\t" + std::format(fs, M(3, 2)) + "\t" + std::format(fs, M(3, 3));
	}

	template<class T = GameObject>
	static T* getNamedObject(std::string name) {
		if (InternalObject::getNamedInternalObjects().contains(name)) {
			return dynamic_cast<T*>(InternalObject::getNamedInternalObjects().at(name));
		}
		else {
			return nullptr;
		}
	}

	void setModel(Model* model) {
		model_ = model;
	}

	void setTexture(Texture* tex) {
		texture_ = tex;
	}

	void activateHitbox() {
		active_hitbox_ = true;
	}
	void deactivateHitbox() {
		active_hitbox_ = false;
	}


};
//template <class G, int ... Keys>
//const std::array<int, sizeof(Keys)> GameObject<G, Keys...>::keys{ { Keys... } };

//template <class G, int ... Keys>
//const G GameObject<G, Keys...>::shared_grobj_(GameObject::model_, GameObject::texture_, NULL);

//DEPRECATED keeping for now for backwards compatability
template <int ... Keys> //this has to change eventually i think. 5/11/23 indeed it has
class InterfaceObject : public GameObject {
	KeyStateCallback<Keys...> key_state_callback_caller_;
private:
	static constexpr std::array<int, sizeof...(Keys)> keys{ { Keys... } };

public:
	//poll inputs
	void update(GLFWwindow* window) override {
		//poll inputs first
		//this way inputs are parsed with main() pollInputs input parsing
		//then update GameObject
		GameObject::update(window);

	}
	InterfaceObject(std::string name) :
		GameObject(name, key_state_callback_caller_){
	}
};



#endif