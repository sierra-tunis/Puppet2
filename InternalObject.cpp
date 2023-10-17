#include "InternalObject.h"


InternalObject::callbackInput InternalObject::input_members_;
int InternalObject::last_id_ = 0;
std::unordered_map<std::string, InternalObject*> InternalObject::named_internal_objects_;
const KeyStateCallback_base InternalObject::no_key_state_callback;
const ControllerStateCallback_base  InternalObject::no_controller_state_callback;
GLFWgamepadstate InternalObject::last_gamepad_state_;
int InternalObject::controller_id_ = -1;
