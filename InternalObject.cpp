#include "InternalObject.h"


InternalObject::callbackInput InternalObject::input_members_;
int InternalObject::last_id_ = 0;
const std::string InternalObject::no_name = "";
std::unordered_map<std::string, const InternalObject*> InternalObject::named_internal_objects_;
KeyStateCallback_base InternalObject::no_key_state_callback_;