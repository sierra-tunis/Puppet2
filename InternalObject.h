#pragma once

//#include "zmap.h"

#ifndef PUPPET_INTERNALOBJECT
#define PUPPET_INTERNALOBJECT

#include <Eigen/Dense>

#include "Hitbox.h"
#include "motion_constraint.h"

#include <GLFW/glfw3.h>


using Eigen::Matrix4f;
using Eigen::Matrix3f;
using Eigen::seq;

//a good cutoff for what is an internal vs game object is an internal object cannot be added to a level

class InternalObject { //class for non-template dependent values, internal use only functionality (i.e. debug stuff). No update function
private:
	//ALL transforms should be relative to parent, if you want to get the tform from one global position to the other
	//do get obj1.relativePosition(obj2) which should return G1_parent_inv*(obj2.global*obj1.global_inv)
	//so move(obj1.relativePosition(obj2)) will always move obj1 to obj2 but not need to "modify" global position
	Eigen::Matrix4f position_;
	const Hitbox hbox_; //this should maybe be moved to gameobject since it is only relevant for dynamic behavior

	
	static int last_id_;
	static std::unordered_map<std::string,const InternalObject*> named_internal_objects_;
	const int id_;
	std::string name_;


	const InternalObject* parent_;
	//std::vector<InternalObject*> children_; //children should be created and managed by parent. in this way each game object is a sub world object
	PositionConstraint* parent_connector_;
	//this might be a terrible idea:

	struct callbackInput {
		std::vector<InternalObject*> key_;
		std::vector<InternalObject*> mouse_;
		std::vector<InternalObject*> controller_callback_members_;

		float mouse_xpos_last_, mouse_ypos_last_;

	};
	static callbackInput input_members_;

	static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mod) {
		//InternalObject* input_callback_ = static_cast<InternalObject*>(glfwGetWindowUserPointer(window));
		callbackInput* input_members = static_cast<callbackInput*>(glfwGetWindowUserPointer(window));
		switch (action) {
		case GLFW_PRESS:
			for (InternalObject* obj : input_members->key_) {
				obj->onKeyPress(key);
			}
			break;
		case GLFW_RELEASE:
			for (InternalObject* obj : input_members->key_) {
				obj->onKeyRelease(key);
			}
			break;
		case GLFW_REPEAT:
			break;
		}
	}

	static std::vector<InternalObject*> mouse_callback_members_;
	static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
		callbackInput* input_members = static_cast<callbackInput*>(glfwGetWindowUserPointer(window));
		xpos = static_cast<float>(xpos);
		ypos = static_cast<float>(ypos);
		float dx = xpos - input_members->mouse_xpos_last_;
		float dy = ypos - input_members->mouse_ypos_last_;
		for (InternalObject* obj : input_members->mouse_) {
			obj->onMouseMove(xpos,ypos,dx,dy);
		}
		input_members->mouse_xpos_last_ = xpos;
		input_members->mouse_ypos_last_ = ypos;
	}
	static void mousebuttonCallback(GLFWwindow* window, int button, int action, int mod) {
		//InternalObject* input_callback_ = static_cast<InternalObject*>(glfwGetWindowUserPointer(window));
		callbackInput* input_members = static_cast<callbackInput*>(glfwGetWindowUserPointer(window));
		std::pair<float, float> cursor_pos;
		switch (action) {
		case GLFW_PRESS:
			cursor_pos = getCursorPosition(window);
			for (InternalObject* obj : input_members->mouse_) {
				obj->onMouseDown(button,cursor_pos.first,cursor_pos.second);
			}
			break;
		case GLFW_RELEASE:
			cursor_pos = getCursorPosition(window);
			for (InternalObject* obj : input_members->mouse_) {
				obj->onMouseUp(button, cursor_pos.first, cursor_pos.second);
			}
			break;
		case GLFW_REPEAT:
			break;
		}
	}

public://needs to be changed later, only did this for debugging xzmapper
	void updatePosition() { 
		if (parent_ != nullptr && parent_connector_ != nullptr) {
			this->position_ = parent_->getPosition()*(parent_connector_->getConstraintTransform());
		}
		//if getGlobalPosition() were always correct then this position would never be stale by this line
	}

	//wrong class for this tbh
	static Eigen::Matrix4f inverseTform(const Eigen::Matrix4f& position) {
		Eigen::Matrix4f ret = Eigen::Matrix4f::Identity();
		Eigen::Matrix3f R_T = position(seq(0, 2), seq(0, 2)).transpose();
		ret(seq(0, 2), seq(0, 2)) = R_T;
		ret(seq(0, 2), 3) = -R_T * position(seq(0, 2), 3);
		return ret;
	}

protected:

	void unclamp() {
		this->parent_ = nullptr;
	}

	inline virtual void onKeyPress(int key) {}; //triggers once //not virtual so it can be inlined as {} if it's not overridden

	inline virtual void onKeyRelease(int key) {};//triggers once

	inline virtual void onKeyDown(int key) {}; //triggers repeatedly

	inline virtual void onKeyUp(int key) {}; //triggers repeatedly

	inline virtual void onMouseMove(float x, float y, float dx, float dy) {}; //triggers repeatedly

	inline virtual void onMouseDown(int key, float x, float y) {};
	
	inline virtual void onMouseUp(int key, float x, float y) {};

	inline virtual void onCreation() {};

	inline virtual void onStep() {};

	inline virtual void onCollision(const InternalObject& other) {};

	inline virtual void onDecollision(const InternalObject& other) {};

	const InternalObject* getParent() const {
		return parent_;
	}

public:

	// these two should probably not be protected
	inline virtual void onRoomActivation() {};

	inline virtual void onRoomDeactivation() {};

	const static std::string no_name;

	InternalObject(std::string name) :
		position_(Eigen::Matrix4f::Identity()),
		id_(last_id_++),//this is only to avoid not wanting to generate random strings
		name_(name),
		parent_(nullptr),
		parent_connector_(nullptr) {

		this->onCreation();//i dont think this works since you cant use virtual functions in a constructor
		InternalObject::named_internal_objects_[name] = this;

	}
	InternalObject() :
		position_(Eigen::Matrix4f::Identity()),
		name_(InternalObject::no_name),
		id_(last_id_++),
		parent_(nullptr),
		parent_connector_(nullptr) {

		this->onCreation();

	}


	virtual void clampTo(const InternalObject* parent) {// this has unintuitive behavior
		this->parent_ = parent;
		parent_connector_ = new OffsetConnector(parent_->getPosition(), position_);
	}

	void connectTo(const InternalObject* parent,PositionConstraint* connector) {
		this->parent_ = parent;
		parent_connector_ = connector;
		connector->setRootTransform(&parent_->getPosition());
	}
	void connectTo(const InternalObject* parent) {
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

	virtual void update(GLFWwindow* window) {
		updatePosition();
	}

	const Matrix4f& getPosition() const {
		return this->position_;
	}

	const Matrix4f getRelativePosition(InternalObject& other) const {
		//this is almost certainly wrong
		return inverseTform(parent_->getPosition()) * other.getPosition() * inverseTform(position_);
		//so globalPosition = parent.gloabl*getRelativePosition(other)*position_ = other.globalPosition
	}

	int getID() const {
		return this->id_;
	}

	const Hitbox& getHbox() const {
		return hbox_;
	}


	void activateKeyInput(GLFWwindow* window) {
		//input_objs = InternalObject * this_ = static_cast<InternalObject*>(glfwGetWindowUserPointer(window));
		input_members_.key_.push_back(this);
		glfwSetWindowUserPointer(window, &input_members_);
		glfwSetKeyCallback(window, InternalObject::keyCallback);
	}

	void activateMouseInput(GLFWwindow* window) {
		input_members_.mouse_.push_back(this);
		//glfwSetWindowUserPointer(window, this);
		glfwSetCursorPosCallback(window, cursor_position_callback);
		double x, y;
		glfwGetCursorPos(window, &x, &y);
		glfwSetWindowUserPointer(window, &input_members_);
		input_members_.mouse_xpos_last_ = x;
		input_members_.mouse_ypos_last_ = y;

		glfwSetMouseButtonCallback(window, InternalObject::mousebuttonCallback);
	}

	static std::pair<float, float> getCursorPosition(GLFWwindow* window) {
		int window_width, window_height;
		glfwGetWindowSize(window, &window_width, &window_height);
		return std::pair<float, float>{input_members_.mouse_xpos_last_/window_width*2.-1., -input_members_.mouse_ypos_last_/window_height*2.+1.};
	}


	//virtual InternalObject(std::string filename) = 0;

	//virtual File save() const = 0;
	
	// virtual std::vector<bool> unitTest() = 0;

	void translate(Eigen::Vector3f vec) {
		position_(seq(0, 2), 3) += vec;
		//stale_global_position_ = true;
	};
	void translate(float dx, float dy, float dz) {
		position_(0, 3) += dx;
		position_(1, 3) += dy;
		position_(2, 3) += dz;
		//stale_global_position_ = true;
	};

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

	//needs to be reworked
	/*void moveToGlobal(Eigen::Vector3f vec) {
		//parent*new_pos = vec
		//new_pos = parent^-1*vec
		//global = parent*local
		//parent = global*local^-1
		Eigen::Matrix4f tmp = inverseTform(position_ * inverseTform(position_));
		position_(seq(0, 2), 3) = tmp(seq(0, 2), seq(0, 2)) * vec + tmp(seq(0, 2), 3);
	}*/

	void rotate(Eigen::Matrix3f rot) {
		position_(seq(0, 2), seq(0, 2)) = rot * position_(seq(0, 2), seq(0, 2));
		//stale_global_position_ = true;
	};

	void rotateAxisAngle(Eigen::Vector3f axis,float angle) {
		Matrix3f w_hat;
		w_hat << 0, -axis(2), axis(1),
					axis(2), 0, -axis(0),
					-axis(1), axis(0), 0;
		Matrix3f rot = Matrix3f::Identity() + w_hat * sin(angle) + w_hat * w_hat * (1 - cos(angle));
		rotate(rot);
	};

	void rotateX(float angle) {rotateAxisAngle(Eigen::Vector3f(1, 0, 0), angle);};
	void rotateY(float angle) {rotateAxisAngle(Eigen::Vector3f(0, 1, 0), angle);};
	void rotateZ(float angle) {rotateAxisAngle(Eigen::Vector3f(0, 0, 1), angle);};

	virtual std::string getDebugInfo() const { return ""; };
	
	const std::string& getName() const { return name_; }

};

#endif
