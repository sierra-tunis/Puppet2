#pragma once

#ifndef PUPPET_GAMEOBJECT_HUMANOID
#define PUPPET_GAMEOBJECT_HUMANOID

#include "GameObject.h"
#include "dynamic_model.hpp"
#include "UI.h"

using LimbConnector = ConnectorChain<OffsetConnector, BallJoint, OffsetConnector, RotationJoint, OffsetConnector, BallJoint>;

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

	DynamicModel* dyn_model_;

	std::vector<int> debug_UI_ids;
	static void setLeftKneeState(float angle, void* must_be_this) {
		Humanoid* this_ = static_cast<Humanoid*>(must_be_this);
		this_->knee_L_.setState(angle);
		this_->leg_L_.setState(this_->leg_L_.getState());
	}

	void onStep() {
		dyn_model_->updateData();
	}

public:

	Humanoid(std::string name, KeyStateCallback_base& key_state_callback_caller):
		GameObject(name,key_state_callback_caller),
		origin_(0, .15, 0),
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
		hip_offset_L_(.0904, .1037, 0),
		hip_L_(BallJoint::YXY),
		knee_offset_L_(.0862, -.4319, 0),
		knee_L_(Eigen::Vector3f(1, 0, 0)),
		ankle_offset_L_(.0596, -.9047, 0),
		ankle_L_(BallJoint::ZXY),
		hip_offset_R_(-.0904, .1037, 0),
		hip_R_(BallJoint::YXY),
		knee_offset_R_(-.0862, -.4319, 0),
		knee_R_(Eigen::Vector3f(1, 0, 0)),
		ankle_offset_R_(-.0596, -.9047, 0),
		ankle_R_(BallJoint::ZXY),

		arm_L_(shoulder_offset_L_, shoulder_L_, elbow_offset_L_, elbow_L_, wrist_offset_L_, wrist_L_),
		arm_R_(shoulder_offset_R_, shoulder_R_, elbow_offset_R_, elbow_R_, wrist_offset_R_, wrist_R_),
		leg_L_(hip_offset_L_, hip_L_, knee_offset_L_, knee_L_, ankle_offset_L_, ankle_L_),
		leg_R_(hip_offset_R_, hip_R_, knee_offset_R_, knee_R_, ankle_offset_R_, ankle_R_),
		n_dofs(RotationJoint::getDoF() + BallJoint::getDoF() + 4 * LimbConnector::getDoF()) {

		arm_L_.setRootTransform(&chest_rotation_.getEndTransform());
		arm_R_.setRootTransform(&chest_rotation_.getEndTransform());
		leg_L_.setRootTransform(&waist_rotation_.getEndTransform());
		arm_L_.setRootTransform(&waist_rotation_.getEndTransform());
		chest_rotation_.setRootTransform(&getPosition());
		waist_rotation_.setRootTransform(&getPosition());

		arm_L_.refresh();
		arm_R_.refresh();
		leg_L_.refresh();
		leg_R_.refresh();

		DynamicModel* model = new DynamicModel("human.obj", "human.txt");
		std::vector<const Eigen::Matrix4f*> vert_tforms;
		for (int i = 0; i < model->glen(); i++) {
			vert_tforms.push_back(nullptr);
		}

		vert_tforms[model->getInd("fingers_L")] = &wrist_L_.getEndTransform();
		vert_tforms[model->getInd("thumb_L")] = &wrist_L_.getEndTransform();
		vert_tforms[model->getInd("palm_L")] = &wrist_L_.getEndTransform();
		vert_tforms[model->getInd("forearm_L")] = &elbow_L_.getEndTransform();
		vert_tforms[model->getInd("humerus_L")] = &shoulder_L_.getEndTransform();
		//offset tform not working?
		vert_tforms[model->getInd("shoulder_L")] = &shoulder_offset_L_.getEndTransform();

		vert_tforms[model->getInd("fingers_R")] = &wrist_R_.getEndTransform();
		vert_tforms[model->getInd("thumb_R")] = &wrist_R_.getEndTransform();
		vert_tforms[model->getInd("palm_R")] = &wrist_R_.getEndTransform();
		vert_tforms[model->getInd("forearm_R")] = &elbow_R_.getEndTransform();
		vert_tforms[model->getInd("humerus_R")] = &shoulder_R_.getEndTransform();
		vert_tforms[model->getInd("shoulder_R")] = &shoulder_offset_R_.getEndTransform();

		vert_tforms[model->getInd("ribcage")] = &chest_rotation_.getEndTransform();
		vert_tforms[model->getInd("waist")] = &waist_rotation_.getEndTransform();

		vert_tforms[model->getInd("hip_L")] = &hip_offset_L_.getEndTransform();
		vert_tforms[model->getInd("thigh_L")] = &hip_L_.getEndTransform();
		vert_tforms[model->getInd("calf_L")] = &knee_L_.getEndTransform();
		vert_tforms[model->getInd("foot_L")] = &ankle_L_.getEndTransform();

		vert_tforms[model->getInd("hip_R")] = &hip_offset_R_.getEndTransform();
		vert_tforms[model->getInd("thigh_R")] = &hip_R_.getEndTransform();
		vert_tforms[model->getInd("calf_R")] = &knee_R_.getEndTransform();
		vert_tforms[model->getInd("foot_R")] = &ankle_R_.getEndTransform();

		vert_tforms[model->getInd("neck")] = &getPosition();
		vert_tforms[model->getInd("head")] = &getPosition();

		model->setVertTforms(vert_tforms);
		model->offsetVerts();
		dyn_model_ = model;
		setModel(model);
		setTexture(new Texture("rocky.jpg"));
	}
	/*
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

		DynamicModel* model = new DynamicModel("human.obj", "human.txt");
		std::vector<const Eigen::Matrix4f*> vert_tforms;
		for (int i = 0; i < model->glen(); i++) {
			vert_tforms.push_back(nullptr);
		}

		vert_tforms[model->getInd("fingers_L")] = &wrist_L_.getEndTransform();
		vert_tforms[model->getInd("thumb_L")] = &wrist_L_.getEndTransform();
		vert_tforms[model->getInd("palm_L")] = &wrist_L_.getEndTransform();
		vert_tforms[model->getInd("forearm_L")] = &elbow_L_.getEndTransform();
		vert_tforms[model->getInd("humerus_L")] = &shoulder_L_.getEndTransform();
		vert_tforms[model->getInd("shoulder_L")] = &shoulder_offset_L_.getEndTransform();

		vert_tforms[model->getInd("fingers_R")] = &wrist_R_.getEndTransform();
		vert_tforms[model->getInd("thumb_R")] = &wrist_R_.getEndTransform();
		vert_tforms[model->getInd("palm_R")] = &wrist_R_.getEndTransform();
		vert_tforms[model->getInd("forearm_R")] = &elbow_R_.getEndTransform();
		vert_tforms[model->getInd("humerus_R")] = &shoulder_R_.getEndTransform();
		vert_tforms[model->getInd("shoulder_R")] = &shoulder_offset_R_.getEndTransform();

		vert_tforms[model->getInd("ribcage")] = &chest_rotation_.getEndTransform();
		vert_tforms[model->getInd("waist")] = &waist_rotation_.getEndTransform();

		vert_tforms[model->getInd("hip_L")] = &hip_offset_L_.getEndTransform();
		vert_tforms[model->getInd("thigh_L")] = &hip_L_.getEndTransform();
		vert_tforms[model->getInd("calf_L")] = &knee_L_.getEndTransform();
		vert_tforms[model->getInd("foot_L")] = &ankle_L_.getEndTransform();

		vert_tforms[model->getInd("hip_R")] = &hip_offset_R_.getEndTransform();
		vert_tforms[model->getInd("thigh_R")] = &hip_R_.getEndTransform();
		vert_tforms[model->getInd("calf_R")] = &knee_R_.getEndTransform();
		vert_tforms[model->getInd("foot_R")] = &ankle_R_.getEndTransform();

		vert_tforms[model->getInd("neck")] = &getPosition();
		vert_tforms[model->getInd("head")] = &getPosition();

		model->setVertTforms(vert_tforms);
		model->offsetVerts();
		dyn_model_ = model;
		setModel(model);
		setTexture(new Texture("rocky.jpg"));
	}*/

	void openDebugUI(const GameObject* UI_container, GLFWwindow* window, GraphicsRaw<GameObject>& graphics_2d, GraphicsRaw<Textbox>& text_graphics) override {
		Slider* s1 = new Slider(.1, .5, -M_PI, M_PI);
		addDependent(s1);
		s1->load(window, graphics_2d, text_graphics);
		s1->setSliderChangeCallback(&setLeftKneeState, this);
	}
	void closeDebugUI(const GameObject* UI_container, GLFWwindow* window, GraphicsRaw<GameObject>& graphics_2d, GraphicsRaw<Textbox>& text_graphics) override {
		
	}




};

#endif // !PUPPET_GAMEOBJECT_HUMANOID
