#pragma once

#ifndef PUPPET_SKELETON
#define PUPPET_SKELETON

#include "motion_constraint.h"

class Skeleton_base {

};

template<Connector connector_T, Connector... connectors_T>
class Skeleton {

	std::tuple<constraint&, constraints&...> connectors_;


};

#endif