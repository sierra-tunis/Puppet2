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
extern class InternalObject;

class KeyStateCallback_base {	
public:
	inline virtual void pollInputs(GLFWwindow* window,InternalObject& this_){}
};


class InternalObject { //class for non-template dependent values, internal use only functionality (i.e. debug stuff). No update function
private:
	//ALL transforms should be relative to parent, if you want to get the tform from one global position to the other
	//do get obj1.relativePosition(obj2) which should return G1_parent_inv*(obj2.global*obj1.global_inv)
	//so move(obj1.relativePosition(obj2)) will always move obj1 to obj2 but not need to "modify" global position
	
	static int last_id_;
	static std::unordered_map<std::string,const InternalObject*> named_internal_objects_;
	const int id_;
	std::string name_;


	//std::vector<InternalObject*> children_; //children should be created and managed by parent. in this way each game object is a sub world object
	//this might be a terrible idea:
	KeyStateCallback_base& key_state_callback_;
	static KeyStateCallback_base no_key_state_callback_;

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
	

	//wrong class for this tbh
	static Eigen::Matrix4f inverseTform(const Eigen::Matrix4f& position) {
		Eigen::Matrix4f ret = Eigen::Matrix4f::Identity();
		Eigen::Matrix3f R_T = position(seq(0, 2), seq(0, 2)).transpose();
		ret(seq(0, 2), seq(0, 2)) = R_T;
		ret(seq(0, 2), 3) = -R_T * position(seq(0, 2), 3);
		return ret;
	}

protected:

	inline virtual void onKeyPress(int key) {}; //triggers once //not virtual so it can be inlined as {} if it's not overridden

	inline virtual void onKeyRelease(int key) {};//triggers once

	inline virtual void onMouseMove(float x, float y, float dx, float dy) {}; //triggers repeatedly

	inline virtual void onMouseDown(int key, float x, float y) {};
	
	inline virtual void onMouseUp(int key, float x, float y) {};

	inline virtual void onCreation() {};

	inline virtual void onStep() {};


public:

	// these two should probably not be protected
	inline virtual void onRoomActivation() {};

	inline virtual void onRoomDeactivation() {};

	inline virtual void onKeyDown(int key) {}; //triggers repeatedly

	inline virtual void onKeyUp(int key) {}; //triggers repeatedly, probably not useful


	const static std::string no_name;

	InternalObject(std::string name) :
		id_(last_id_++),//this is only to avoid not wanting to generate random strings
		name_(name),
		key_state_callback_(no_key_state_callback_){

		this->onCreation();//i dont think this works since you cant use virtual functions in a constructor
		InternalObject::named_internal_objects_[name] = this;

	}
	InternalObject() :
		name_(InternalObject::no_name),
		id_(last_id_++),
		key_state_callback_(no_key_state_callback_){

		this->onCreation();

	}
	InternalObject(std::string name, KeyStateCallback_base& key_state_callback) :
		id_(last_id_++),//this is only to avoid not wanting to generate random strings
		name_(name),
		key_state_callback_(key_state_callback) {

		this->onCreation();//i dont think this works since you cant use virtual functions in a constructor
		InternalObject::named_internal_objects_[name] = this;

	}


	inline virtual void update(GLFWwindow* window) {
		//this should be rewritten to be consteval eventually
		key_state_callback_.pollInputs(window,*this);
	}

	int getID() const {
		return this->id_;
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

	const std::string& getName() const { return name_; }

};

template<int... Keys>
class KeyStateCallback : public KeyStateCallback_base{
private:
	static constexpr std::array<int, sizeof...(Keys)> keys{ { Keys... } };
public:
	inline void pollInputs(GLFWwindow* window, InternalObject& this_) override {
		for (int k : keys) {
			if (glfwGetKey(window, k) == GLFW_PRESS) {
				this_.onKeyDown(k);
			}
			else if (glfwGetKey(window, k) == GLFW_RELEASE) {
				this_.onKeyUp(k);
			}
		}
	}
	
};

#endif
