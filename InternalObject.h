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

const Eigen::Matrix4f __global_origin__ = Eigen::Matrix4f::Identity();

//a good cutoff for what is an internal vs game object is an internal object cannot be added to a level
extern class InternalObject;

class KeyStateCallback_base {	
public:
	inline virtual void pollInputs(GLFWwindow* window,InternalObject& this_) const {}
};

class ControllerStateCallback_base {
public:
	inline virtual void pollInputs(GLFWwindow* window, InternalObject& this_) const {}
};



class InternalObject { //class for non-template dependent values, internal use only functionality (i.e. debug stuff). No update function
private:
	//ALL transforms should be relative to parent, if you want to get the tform from one global position to the other
	//do get obj1.relativePosition(obj2) which should return G1_parent_inv*(obj2.global*obj1.global_inv)
	//so move(obj1.relativePosition(obj2)) will always move obj1 to obj2 but not need to "modify" global position
	
	static int last_id_;
	static std::unordered_map<std::string, InternalObject*> named_internal_objects_;
	const int id_;
	std::string name_;


	//std::vector<InternalObject*> children_; //children should be created and managed by parent. in this way each game object is a sub world object
	//this might be a terrible idea:
	const KeyStateCallback_base& key_state_callback_;
	const ControllerStateCallback_base& controller_state_callback_;
	static GLFWgamepadstate last_gamepad_state_;
	static constexpr std::array<int, 15> all_controller_buttons_{GLFW_GAMEPAD_BUTTON_A, GLFW_GAMEPAD_BUTTON_B, GLFW_GAMEPAD_BUTTON_X, GLFW_GAMEPAD_BUTTON_Y, GLFW_GAMEPAD_BUTTON_LEFT_BUMPER, GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER, GLFW_GAMEPAD_BUTTON_BACK, GLFW_GAMEPAD_BUTTON_START, GLFW_GAMEPAD_BUTTON_GUIDE, GLFW_GAMEPAD_BUTTON_LEFT_THUMB, GLFW_GAMEPAD_BUTTON_RIGHT_THUMB, GLFW_GAMEPAD_BUTTON_DPAD_UP, GLFW_GAMEPAD_BUTTON_DPAD_RIGHT, GLFW_GAMEPAD_BUTTON_DPAD_DOWN, GLFW_GAMEPAD_BUTTON_DPAD_LEFT};
	static int controller_id_;


	struct callbackInput {
		std::unordered_set<InternalObject*> key_;
		std::unordered_set<InternalObject*> mouse_;
		std::unordered_set<InternalObject*> controller_callback_members_;

		float mouse_xpos_last_, mouse_ypos_last_;
		//float mouse_xscroll_last, mouse_yscroll_last_;

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

	static void mouseScrollCallback(GLFWwindow* window, double dx, double dy) {
		callbackInput* input_members = static_cast<callbackInput*>(glfwGetWindowUserPointer(window));
		//xscroll = static_cast<float>(xscroll);
		//yscroll = static_cast<float>(yscroll);
		//float dx = xscroll - input_members->mouse_xscroll_last_;
		//float dy = yscroll - input_members->mouse_yscroll_last_;
		dx = static_cast<float>(dx);
		dy = static_cast<float>(dy);
		for (InternalObject* obj : input_members->mouse_) {
			obj->onMouseScroll(dx, dy);
		}
	}

	static void mousebuttonCallback(GLFWwindow* window, int button, int action, int mod) {
		//InternalObject* input_callback_ = static_cast<InternalObject*>(glfwGetWindowUserPointer(window));
		callbackInput* input_members = static_cast<callbackInput*>(glfwGetWindowUserPointer(window));
		std::pair<float, float> cursor_pos;
		//std::unordered_set<InternalObject*> mouse_members = input_members->mouse_;//copy to avoid input members changing during execution
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


	static void connectControllerCallback(int jid, int event) {
		if (event == GLFW_CONNECTED){
			if (glfwJoystickIsGamepad(jid)) {
				controller_id_ = jid;
			}
		}
		else if (event == GLFW_DISCONNECTED){
			controller_id_ = -1;
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

	inline virtual void onMouseScroll(float dx, float dy) {

	};

	inline virtual void onControllerButtonPress(int key) {}; //triggers once //not virtual so it can be inlined as {} if it's not overridden

	inline virtual void onControllerButtonRelease(int key) {}; //triggers once //not virtual so it can be inlined as {} if it's not overridden

	inline virtual void onCreation() {};

	inline virtual void onStep() {};

	static const std::unordered_map<std::string, InternalObject*>& getNamedInternalObjects() {
		return named_internal_objects_;
	};

public:

	// these two should probably not be protected
	inline virtual void onRoomActivation() {};

	inline virtual void onRoomDeactivation() {};

	inline virtual void onKeyDown(int key) {}; //triggers repeatedly

	inline virtual void onKeyUp(int key) {}; //triggers repeatedly, probably not useful

	inline virtual void onControllerButtonDown(int button) {}; //triggers repeatedly

	inline virtual void onControllerButtonUp(int button) {}; //triggers repeatedly, probably not useful



	constexpr static char no_name[] = "";
	const static KeyStateCallback_base no_key_state_callback;
	const static ControllerStateCallback_base no_controller_state_callback;

	InternalObject(std::string name=no_name, const KeyStateCallback_base& key_state_callback = no_key_state_callback, const ControllerStateCallback_base& controller_state_callback = no_controller_state_callback) :
		id_(last_id_++),//this is only to avoid not wanting to generate random strings
		name_(name),
		key_state_callback_(key_state_callback),
		controller_state_callback_(controller_state_callback){

		if (name_ != no_name) {
			if (InternalObject::named_internal_objects_.contains(name_)) {
				name_ = no_name; //copies have no name (i.e. all copies are temporary)
			} else {
				InternalObject::named_internal_objects_[name_] = this;
			}
		}

	}

	InternalObject(InternalObject& other) : InternalObject(other.getName()) {

	}
	
	~InternalObject() {
		last_id_ = getID();
		if (name_ != no_name) {
			InternalObject::named_internal_objects_.erase(name_);
		}
	}

	inline virtual void update(GLFWwindow* window) {
		//this should be rewritten to be consteval eventually
		key_state_callback_.pollInputs(window,*this);
		controller_state_callback_.pollInputs(window, *this);
	}

	int getID() const {
		return this->id_;
	}


	void activateKeyInput(GLFWwindow* window) {
		//input_objs = InternalObject * this_ = static_cast<InternalObject*>(glfwGetWindowUserPointer(window));
		input_members_.key_.insert(this);
		glfwSetWindowUserPointer(window, &input_members_);
		glfwSetKeyCallback(window, InternalObject::keyCallback);
	}
	void deactivateKeyInput(GLFWwindow* window) {
		input_members_.key_.erase(this);
	}

	void activateMouseInput(GLFWwindow* window) {
		input_members_.mouse_.insert(this);
		//glfwSetWindowUserPointer(window, this);
		glfwSetCursorPosCallback(window, cursor_position_callback);
		double x, y;
		glfwGetCursorPos(window, &x, &y);
		glfwSetWindowUserPointer(window, &input_members_);
		input_members_.mouse_xpos_last_ = x;
		input_members_.mouse_ypos_last_ = y;

		glfwSetMouseButtonCallback(window, InternalObject::mousebuttonCallback);
		glfwSetScrollCallback(window, InternalObject::mouseScrollCallback);
	}
	void deactivateMouseInput(GLFWwindow* window) {
		input_members_.mouse_.erase(this);
	}
	void activateControllerInput(GLFWwindow* window) {
		input_members_.controller_callback_members_.insert(this);
		glfwSetWindowUserPointer(window, &input_members_);
		glfwSetJoystickCallback(connectControllerCallback);

	}
	void deactivateControllerInput(GLFWwindow* window) {
		input_members_.controller_callback_members_.erase(this);
		glfwSetWindowUserPointer(window, &input_members_);

	}

	static std::pair<float, float> getCursorPosition(GLFWwindow* window) {
		int window_width, window_height;
		glfwGetWindowSize(window, &window_width, &window_height);
		return std::pair<float, float>{input_members_.mouse_xpos_last_/window_width*2.-1., -input_members_.mouse_ypos_last_/window_height*2.+1.};
	}

	static std::pair<float, float> getLeftStickPosition(GLFWwindow* window) {
		return { last_gamepad_state_.axes[GLFW_GAMEPAD_AXIS_LEFT_X],last_gamepad_state_.axes[GLFW_GAMEPAD_AXIS_LEFT_Y] };
	}

	static std::pair<float, float> getRightStickPosition(GLFWwindow* window) {
		return { last_gamepad_state_.axes[GLFW_GAMEPAD_AXIS_RIGHT_X],last_gamepad_state_.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y] };

	}
	static float getLeftTriggerPosition(GLFWwindow* window) {
		return last_gamepad_state_.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER];

	}
	static float getRightTriggerPosition(GLFWwindow* window) {
		return last_gamepad_state_.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER];
	}

	static GLFWgamepadstate& getGamepadState() {
		return last_gamepad_state_;
	}

	static void pollControllerInputs() {
		if (InternalObject::controller_id_ != -1) {
			GLFWgamepadstate gamepad_state;
			glfwGetGamepadState(InternalObject::controller_id_, &gamepad_state);
			for (int button : all_controller_buttons_) { //calls in button major order
				if (gamepad_state.buttons[button] && !last_gamepad_state_.buttons[button]) {
					//rising press
					for (InternalObject* obj : input_members_.controller_callback_members_) {
						obj->onControllerButtonPress(button);
					}

				}
				else if (!gamepad_state.buttons[button] && last_gamepad_state_.buttons[button]) {
					for (InternalObject* obj : input_members_.controller_callback_members_) {
						obj->onControllerButtonRelease(button);
					}
				}
			}
			last_gamepad_state_ = gamepad_state;
		}

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
	inline void pollInputs(GLFWwindow* window, InternalObject& this_) const override {
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


template<int... ControllerButtons>
class ControllerStateCallback : public ControllerStateCallback_base {
private:
	static constexpr std::array<int, sizeof...(ControllerButtons)> buttons{ { ControllerButtons... } };

public:
	inline void pollInputs(GLFWwindow* window, InternalObject& this_) const override {//object major order
		for (int b : buttons) { 
			if (InternalObject::getGamepadState().buttons[b]) {
				this_.onControllerButtonDown(b);
			}
			else {
				this_.onControllerButtonUp(b);
			}
		}
	}
};


#endif
