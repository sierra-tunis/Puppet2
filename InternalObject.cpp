#include "InternalObject.h"


InternalObject::callbackInput InternalObject::input_members_;
int InternalObject::last_id_ = 0;
std::unordered_map<std::string, InternalObject*> InternalObject::named_internal_objects_;
const KeyStateCallback_base InternalObject::no_key_state_callback;