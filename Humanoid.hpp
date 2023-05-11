#pragma once

#ifndef PUPPET_GAMEOBJECT_HUMANOID
#define PUPPET_GAMEOBJECT_HUMANOID

#include "GameObject.h"
#include "dynamic_model.hpp"

typedef ConnectorChain <OffsetConnector, BallJoint, OffsetConnector, RotationJoint, OffsetConnector, BallJoint> LimbConnector;

//template <class T>
//using LRpair = std::pair<T, T>;

class Humanoid : public GameObject {
	//origin is at navel
	OffsetConnector origin_;
	RotationJoint waist_rotation_;
	BallJoint chest_rotation_;

	OffsetConnector shoulder_offset_L_;
	BallJoint shoulder_L_;
	OffsetConnector elbow_offset_L_;
	RotationJoint elbow_L_;
	OffsetConnector wrist_offset_L_;
	BallJoint wrist_L_;

	OffsetConnector shoulder_offset_R_;
	BallJoint shoulder_R_;
	OffsetConnector elbow_offset_R_;
	RotationJoint elbow_R_;
	OffsetConnector wrist_offset_R_;
	BallJoint wrist_R_;

	OffsetConnector hip_offset_L_;
	BallJoint hip_L_;
	OffsetConnector knee_offset_L_;
	RotationJoint knee_L_;
	OffsetConnector ankle_offset_L_;
	BallJoint ankle_L_;

	OffsetConnector hip_offset_R_;
	BallJoint hip_R_;
	OffsetConnector knee_offset_R_;
	RotationJoint knee_R_;
	OffsetConnector ankle_offset_R_;
	BallJoint ankle_R_;

	LimbConnector arm_L_;
	LimbConnector arm_R_;
	LimbConnector leg_L_;
	LimbConnector leg_R_;

	int n_dofs;

public:

	Humanoid() :
		//in theory, the origins could be read from a skeleton file
		origin_(0,.15,0),
		chest_rotation_(BallJoint::YXY),
		waist_rotation_(Eigen::Vector3f(0, 1, 0)),
		shoulder_offset_L_(.1607, .5952, 0),
		shoulder_L_(BallJoint::XYX),
		elbow_offset_L_(.48123, .5879, 0),
		elbow_L_(Eigen::Vector3f(0, 1, 0)),
		wrist_offset_L_(.77558, .60311, 0),
		wrist_L_(BallJoint::ZYX),
		shoulder_offset_R_(-.1607, .5952, 0),
		shoulder_R_(BallJoint::XYX),
		elbow_offset_R_(-.48123, .5879, 0),
		elbow_R_(Eigen::Vector3f(0, 1, 0)),
		wrist_offset_R_(-.77558, .60311, 0),
		wrist_R_(BallJoint::ZYX),
		hip_offset_L_(.0904,.1037,0),
		hip_L_(BallJoint::YXY),
		knee_offset_L_(.0862,-.4319,0),
		knee_L_(Eigen::Vector3f(1,0,0)),
		ankle_offset_L_(.0596,-.9047,0),
		ankle_L_(BallJoint::ZXY),
		hip_offset_R_(-.0904, .1037, 0),
		hip_R_(BallJoint::YXY),
		knee_offset_R_(-.0862, -.4319, 0),
		knee_R_(Eigen::Vector3f(1, 0, 0)),
		ankle_offset_R_(-.0596, -.9047, 0),
		ankle_R_(BallJoint::ZXY),

		arm_L_(shoulder_offset_L_,shoulder_L_,elbow_offset_L_,elbow_L_,wrist_offset_L_,wrist_L_),
		arm_R_(shoulder_offset_R_, shoulder_R_, elbow_offset_R_, elbow_R_, wrist_offset_R_, wrist_R_),
		leg_L_(hip_offset_L_,hip_L_,knee_offset_L_,knee_L_,ankle_offset_L_,ankle_L_),
		leg_R_(hip_offset_R_, hip_R_, knee_offset_R_, knee_R_, ankle_offset_R_, ankle_R_),
		n_dofs(RotationJoint::getDoF() + BallJoint::getDoF() + 4*LimbConnector::getDoF()){

		DynamicModel model("human.obj");
		for (int i = 0; i < model.vlen(); i++) {

		}
		model.setVertTforms();

	}




};

#endif // !PUPPET_GAMEOBJECT_HUMANOID
