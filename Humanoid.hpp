#pragma once

#ifndef PUPPET_GAMEOBJECT_HUMANOID
#define PUPPET_GAMEOBJECT_HUMANOID

#include "GameObject.h"
#include "dynamic_model.hpp"

class Humanoid : public GameObject {
	//origin is at navel
	RotationJoint chest_rotation_;
	OffsetConnector left_shoulder_offset_;
	BallJoint left_shoulder_;
	OffsetConnector left_elbow_offset_;
	RotationJoint left_elbow_;
	OffsetConnector left_hand_offset_;
	BallJoint left_wrist_;
	OffsetConnector right_shoulder_offset_;
	BallJoint right_shoulder_;
	OffsetConnector right_elbow_offset_;
	RotationJoint right_elbow_;
	OffsetConnector right_hand_offset_;
	BallJoint right_wrist_;
	RotationJoint waist_rotation_;





};

#endif // !PUPPET_GAMEOBJECT_HUMANOID
