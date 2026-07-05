#pragma once

#ifndef PUPPET_GAMEOBJECT_HUMANOID
#define PUPPET_GAMEOBJECT_HUMANOID


#include <Eigen/Dense>
#include <unordered_map>

#include "math_constants.hpp"

#include "GameObject.h"
#include "dynamic_model.hpp"
#include "UI.h"
#include "animation.hpp"


using LimbConnector = ConnectorChain<OffsetConnector, BallJoint, OffsetConnector, RotationJoint, OffsetConnector, BallJoint>;

class Humanoid : public GameObject {
protected:
	//origin is at navel
	CartesianJoint origin_position_;
	BallJoint origin_rotation_;

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

	OffsetConnector neck_offset_;
	BallJoint neck_;
	OffsetConnector head_offset_;
	RotationJoint head_tilt_;

	ConnectorChain<OffsetConnector, BallJoint, OffsetConnector, RotationJoint> head_chain_;

	LimbConnector arm_L_;
	LimbConnector arm_R_;
	LimbConnector leg_L_;
	LimbConnector leg_R_;

	/*ConnectorChain<CartesianJoint,
		OffsetConnector, RotationJoint, BallJoint,
		LimbConnector, LimbConnector,
		LimbConnector, LimbConnector,
		OffsetConnector, BallJoint, OffsetConnector, RotationJoint> humanoid_chain_;*/

	

public:
	static constexpr int n_dofs = CartesianJoint::getDoF() + BallJoint::getDoF() + 2 * RotationJoint::getDoF() + 2 * BallJoint::getDoF() + 4 * LimbConnector::getDoF();
	static constexpr int upper_dofs_ = 2 * LimbConnector::getDoF() + 2 * BallJoint::getDoF() + RotationJoint::getDoF();
	static constexpr int lower_dofs_ = 2 * LimbConnector::getDoF() + RotationJoint::getDoF();

private:

	Eigen::Vector<float, n_dofs> clipboard_;

	std::array<Slider*,n_dofs> debug_sliders_;
	Pane position_pane_;
	Pane torso_pane_;
	Pane head_pane_;
	Pane arm_L_pane_;
	Pane arm_R_pane_;
	Pane leg_L_pane_;
	Pane leg_R_pane_;
	TabbedPane slider_panes_;

	static std::unordered_set<Texture*> skin_tones;
	static std::unordered_set<Texture*> eye_colors;

public:
	enum class BodyType {
		A,
		A_no_legs,
		A_head_and_torso,
		A_trunk_hands_feet,
		B,
		B_no_legs,
		B_head_and_torso,
		B_trunk_hands_feet,
		custom
	};

	static Texture skin_tone_1;
	static Texture skin_tone_2;
	static Texture skin_tone_3;
	static Texture skin_tone_4;
	static Texture skin_tone_5;
	static Texture skin_tone_6;

	static Texture blue_eyes;
	static Texture green_eyes;
	static Texture brown_eyes;
	static Texture amber_eyes;
	static Texture hazel_eyes;

private:
	std::unordered_map<BodyType, DynamicModel*> body_models_;
	DynamicModel body_a_;
	DynamicModel body_a_no_legs_;
	DynamicModel body_a_head_and_torso_;
	DynamicModel body_a_trunk_hands_feet_;
	DynamicModel body_b_;
	DynamicModel body_b_no_legs_;
	DynamicModel body_b_head_and_torso_;
	DynamicModel body_b_trunk_hands_feet_;

	DynamicModel* dyn_model_;
	int update_count_;
	int update_freq_;

	std::vector<Button*> anim_buttons_;
	UIIterator<AnimationBase> animation_iterator_;
	bool edit_animation_mode_;
	Animation<n_dofs>* edit_animation_;

	MeshSurface hitbox_;
	MeshSurface exact_hitbox_;
	MeshSurface enlarged_hitbox_;


	void refreshDebugSliders() {
		if (debug_sliders_.size() != 0) {
			for (int i = 0; i < n_dofs; i++) {
				if (debug_sliders_[i] != nullptr) {
					debug_sliders_[i]->setCurrentValue(getState()(i));
				}
			}
		}
	}

	template<int index>
	static void setDoFState(float angle, void* must_be_this) {//this could be streamlined (setState is called for each dof)
		Humanoid* this_ = static_cast<Humanoid*>(must_be_this);
		Eigen::Vector<float, n_dofs> state = this_->getState();
		state(index) = angle;
		if (this_->edit_animation_mode_ && this_->edit_animation_ != nullptr) {
			this_->edit_animation_->setFrame(state);
		} else {
			this_->setState(state);
		}
	}

	

	void refresh() {
		setState(getState());
	}

	template<int index=n_dofs-1>
	void setSliderCallbacks() {
		if (debug_sliders_[index] != nullptr) {
			debug_sliders_[index]->setSliderChangeCallback(&setDoFState<index>, this);
		}
		if constexpr(index > 0) {
			setSliderCallbacks<index - 1>();
		}
	}


	static void toggleEditMode(void* must_be_this) {
		Humanoid* this_ = static_cast<Humanoid*>(must_be_this);
		this_->edit_animation_mode_ = !this_->edit_animation_mode_;
	}

	static void setAnimation_static(AnimationBase* unused, AnimationBase* new_animation, void* must_be_this) {
		Humanoid* this_ = static_cast<Humanoid*>(must_be_this);

		//if debug menu is open
		this_->edit_animation_ = static_cast<Animation<n_dofs>*>(new_animation);
	}

	static void saveAnimation(void* must_be_this) {
		Humanoid* this_ = static_cast<Humanoid*>(must_be_this);
		this_->edit_animation_->saveAnimation();
	}
	static void newFrame(void* must_be_this) {
		Humanoid* this_ = static_cast<Humanoid*>(must_be_this);
		if (this_->edit_animation_ != nullptr) {
			this_->edit_animation_->newFrame();
		}
	}
	static void nextFrame(void* must_be_this) {
		Humanoid* this_ = static_cast<Humanoid*>(must_be_this);
		if (this_->edit_animation_ != nullptr) {
			this_->edit_animation_->nextFrame();
		}
	}
	static void prevFrame(void* must_be_this) {
		Humanoid* this_ = static_cast<Humanoid*>(must_be_this);
		if (this_->edit_animation_ != nullptr) {
			this_->edit_animation_->prevFrame();
		}
	}
	static void setAnimationStart(void* must_be_this) {
		Humanoid* this_ = static_cast<Humanoid*>(must_be_this);
		if (this_->edit_animation_ != nullptr) {
			this_->edit_animation_->setAnimationStart();
		}
	}
	static void interpolateFrame(void* must_be_this) {
		Humanoid* this_ = static_cast<Humanoid*>(must_be_this);
		if (this_->edit_animation_ != nullptr) {
			this_->edit_animation_->interpolateCurrentFrame();
		}
	}

protected:
	void onStep() {
		Eigen::Vector<float, n_dofs> new_state;
		if (edit_animation_mode_ && edit_animation_ != nullptr) { //set state using edit_frame cursor
			new_state = edit_animation_->getFrame()(seq(1, n_dofs));
			setState(new_state);
		}
		/*else if (!edit_animation_mode_ && getActiveAnimation() != nullptr) {//set state using elapsed time
			new_state = dynamic_cast<const Animation<n_dofs>*>(getActiveAnimation())->getState();
		}
		else {
			new_state = Eigen::Vector<float, n_dofs>::Constant(0);
		}
		setState(new_state);*/
		if (update_count_ == update_freq_) {
			dyn_model_->updateData();
			update_count_ = 0;
		} else {
			update_count_++;
		}

		refreshDebugSliders();
	}

	void onKeyPress(int key) override {
		if (key == GLFW_KEY_C) {
			clipboard_ = getState();
		} else if (key == GLFW_KEY_V) {
			if (this->edit_animation_mode_ && this->edit_animation_ != nullptr) {
				this->edit_animation_->setFrame(clipboard_);
			}
		}
	}

public:

	Humanoid(std::string name, const KeyStateCallback_base& key_state_callback_caller=InternalObject::no_key_state_callback, const ControllerStateCallback_base& controller_state_callback_caller=InternalObject::no_controller_state_callback) :
		GameObject(name,key_state_callback_caller,controller_state_callback_caller),
		origin_(0, .15, 0),
		origin_rotation_(BallJoint::XZX),
		chest_rotation_(BallJoint::YXY),
		waist_rotation_(Eigen::Vector3f(0, 1, 0)),
		shoulder_offset_L_(Eigen::Vector3f(- .1607, .5952, 0), Eigen::Vector3f(0, .15, 0)),
		shoulder_L_(BallJoint::XZX),
		elbow_offset_L_(Eigen::Vector3f(- .48123, .5879, 0), Eigen::Vector3f(-.1607, .5952, 0)),
		elbow_L_(Eigen::Vector3f(0, 1, 0)),
		wrist_offset_L_(Eigen::Vector3f( - .77558, .60311, 0), Eigen::Vector3f(-.48123, .5879, 0)),
		wrist_L_(BallJoint::ZYX),
		shoulder_offset_R_(Eigen::Vector3f(.1607, .5952, 0), Eigen::Vector3f(0, .15, 0)),
		shoulder_R_(BallJoint::XZX),
		elbow_offset_R_(Eigen::Vector3f(.48123, .5879, 0), Eigen::Vector3f(.1607, .5952, 0)),
		elbow_R_(Eigen::Vector3f(0, 1, 0)),
		wrist_offset_R_(Eigen::Vector3f(.77558, .60311, 0), Eigen::Vector3f(.48123, .5879, 0)),
		wrist_R_(BallJoint::ZYX),
		hip_offset_L_(Eigen::Vector3f(- .0904, .1037, 0), Eigen::Vector3f(0, .15, 0)),
		hip_L_(BallJoint::YXY),
		knee_offset_L_(Eigen::Vector3f(-.0862, -.4319, 0), Eigen::Vector3f(-.0904, .1037, 0)),
		knee_L_(Eigen::Vector3f(1, 0, 0)),
		ankle_offset_L_(Eigen::Vector3f(-.0596, -.9047, 0), Eigen::Vector3f(-.0862, -.4319, 0)),
		ankle_L_(BallJoint::ZXY),
		hip_offset_R_(Eigen::Vector3f(.0904, .1037, 0), Eigen::Vector3f(0, .15, 0)),
		hip_R_(BallJoint::YXY),
		knee_offset_R_(Eigen::Vector3f(.0862, -.4319, 0), Eigen::Vector3f(.0904, .1037, 0)),
		knee_R_(Eigen::Vector3f(1, 0, 0)),
		ankle_offset_R_(Eigen::Vector3f(.0596, -.9047, 0), Eigen::Vector3f(.0862, -.4319, 0)),
		ankle_R_(BallJoint::ZXY),
		neck_offset_(Eigen::Vector3f(0,.6866,0), Eigen::Vector3f(0, .15, 0)),
		neck_(BallJoint::YXY),
		head_offset_(Eigen::Vector3f(0, .8766, 0),Eigen::Vector3f(0, .6866, 0)),
		head_tilt_(Eigen::Vector3f(1.,0,0)),

		arm_L_(shoulder_offset_L_, shoulder_L_, elbow_offset_L_, elbow_L_, wrist_offset_L_, wrist_L_),
		arm_R_(shoulder_offset_R_, shoulder_R_, elbow_offset_R_, elbow_R_, wrist_offset_R_, wrist_R_),
		leg_L_(hip_offset_L_, hip_L_, knee_offset_L_, knee_L_, ankle_offset_L_, ankle_L_),
		leg_R_(hip_offset_R_, hip_R_, knee_offset_R_, knee_R_, ankle_offset_R_, ankle_R_),
		head_chain_(neck_offset_,neck_,head_offset_,head_tilt_),
		//humanoid_chain_(origin_position_,origin_,waist_rotation_, chest_rotation_, arm_L_,arm_R_,leg_L_,leg_R_,neck_offset_,neck_,head_offset_,head_tilt_),
		animation_iterator_(.3,.6),
		slider_panes_(1.,1.,1.),
		hitbox_("human_static_hitbox.obj", AnimationBase::debug_path),
		exact_hitbox_("human_B.obj",AnimationBase::debug_path),
		enlarged_hitbox_("human_combat_hitbox.obj", AnimationBase::debug_path),
		body_a_("human.obj", "human.txt"),
		body_a_no_legs_("human_a_no_legs.obj", "human_a_no_legs.txt"),
		body_a_head_and_torso_("human_a_head_and_torso.obj", "human_a_head_and_torso.txt"),
		body_a_trunk_hands_feet_("human_a_trunk_hands_feet.obj","human_a_trunk_hands_feet.txt"),
		body_b_("human_B.obj", "human_B.txt"),
		body_b_no_legs_("human_b_no_legs.obj", "human_b_no_legs.txt"),
		body_b_head_and_torso_("human_b_head_and_torso.obj", "human_b_head_and_torso.txt"),
		body_b_trunk_hands_feet_("human_b_trunk_hands_feet.obj", "human_b_trunk_hands_feet.txt"),
		body_models_{ { BodyType::A,&body_a_},{ BodyType::A_no_legs,&body_a_no_legs_},{ BodyType::A_head_and_torso,&body_a_head_and_torso_},{ BodyType::A_trunk_hands_feet,&body_a_trunk_hands_feet_} ,{BodyType::B,&body_b_}, { BodyType::B_no_legs,&body_b_no_legs_}, { BodyType::B_head_and_torso,&body_b_head_and_torso_},{BodyType::B_trunk_hands_feet,&body_b_trunk_hands_feet_} } {

		arm_L_.setRootTransform(&chest_rotation_.getEndTransform());
		arm_R_.setRootTransform(&chest_rotation_.getEndTransform());
		leg_L_.setRootTransform(&waist_rotation_.getEndTransform());
		leg_R_.setRootTransform(&waist_rotation_.getEndTransform());
		head_chain_.setRootTransform(&chest_rotation_.getEndTransform());
		chest_rotation_.setRootTransform(&origin_.getEndTransform());
		waist_rotation_.setRootTransform(&origin_.getEndTransform());
		origin_.setRootTransform(&origin_rotation_.getEndTransform());
		origin_position_.setRootTransform(&getPosition());
		origin_rotation_.setRootTransform(&origin_position_.getEndTransform());
		//origin_position_.setRootTransform(nullptr);

		refresh();



		//DynamicModel* model = new DynamicModel("human_B.obj", "human_B.txt");
		for (std::pair<const BodyType, DynamicModel*>& m : body_models_) {
			DynamicModel* model = m.second;
			model->getGroup("fingers_L")->setTform(&wrist_L_.getEndTransform());
			model->getGroup("fingers_L")->setTform(&wrist_L_.getEndTransform());
			model->getGroup("thumb_L")->setTform(&wrist_L_.getEndTransform());
			model->getGroup("palm_L")->setTform(&wrist_L_.getEndTransform());
			model->getGroup("forearm_L")->setTform(&elbow_L_.getEndTransform());
			model->getGroup("humerus_L")->setTform(&shoulder_L_.getEndTransform());
			model->getGroup("shoulder_L")->setTform(&shoulder_L_.getEndTransform());

			model->getGroup("fingers_R")->setTform(&wrist_R_.getEndTransform());
			model->getGroup("thumb_R")->setTform(&wrist_R_.getEndTransform());
			model->getGroup("palm_R")->setTform(&wrist_R_.getEndTransform());
			model->getGroup("forearm_R")->setTform(&elbow_R_.getEndTransform());
			model->getGroup("humerus_R")->setTform(&shoulder_R_.getEndTransform());
			model->getGroup("shoulder_R")->setTform(&shoulder_R_.getEndTransform());

			model->getGroup("ribcage")->setTform(&chest_rotation_.getEndTransform());
			model->getGroup("waist")->setTform(&waist_rotation_.getEndTransform());

			model->getGroup("hip_L")->setTform(&hip_offset_L_.getEndTransform());
			model->getGroup("thigh_L")->setTform(&hip_L_.getEndTransform());
			model->getGroup("calf_L")->setTform(&knee_L_.getEndTransform());
			model->getGroup("foot_L")->setTform(&ankle_L_.getEndTransform());

			model->getGroup("hip_R")->setTform(&hip_offset_R_.getEndTransform());
			model->getGroup("thigh_R")->setTform(&hip_R_.getEndTransform());
			model->getGroup("calf_R")->setTform(&knee_R_.getEndTransform());
			model->getGroup("foot_R")->setTform(&ankle_R_.getEndTransform());

			model->getGroup("neck")->setTform(&neck_.getEndTransform());
			model->getGroup("head")->setTform(&head_tilt_.getEndTransform());
			model->getGroup("eye_L")->setTform(&head_tilt_.getEndTransform());
			model->getGroup("eye_R")->setTform(&head_tilt_.getEndTransform());



			model->offsetVerts();
			model->setRootTransform(&getPosition());
		}
		setModel(&body_b_);
		dyn_model_ = &body_b_;
		setTexture(&skin_tone_2);
		setOverlayTexture(&brown_eyes);

		edit_animation_mode_ = false;
	}


	Humanoid(std::string name, float scale, DynamicModel* model, const KeyStateCallback_base& key_state_callback_caller = InternalObject::no_key_state_callback, const ControllerStateCallback_base& controller_state_callback_caller = InternalObject::no_controller_state_callback) :
		GameObject(name, key_state_callback_caller, controller_state_callback_caller),
		origin_(0, .15, 0),
		origin_rotation_(BallJoint::XZX),
		chest_rotation_(BallJoint::YXY),
		waist_rotation_(Eigen::Vector3f(0, 1, 0)),
		shoulder_offset_L_(Eigen::Vector3f(-.1607 * scale, .5952 * scale, 0), Eigen::Vector3f(0, .15 * scale, 0)),
		shoulder_L_(BallJoint::XZX),
		elbow_offset_L_(Eigen::Vector3f(-.48123 * scale, .5879 * scale, 0), Eigen::Vector3f(-.1607 * scale, .5952 * scale, 0)),
		elbow_L_(Eigen::Vector3f(0, 1, 0)),
		wrist_offset_L_(Eigen::Vector3f(-.77558 * scale, .60311 * scale, 0), Eigen::Vector3f(-.48123 * scale, .5879 * scale, 0)),
		wrist_L_(BallJoint::ZYX),
		shoulder_offset_R_(Eigen::Vector3f(.1607 * scale, .5952 * scale, 0), Eigen::Vector3f(0, .15 * scale, 0)),
		shoulder_R_(BallJoint::XZX),
		elbow_offset_R_(Eigen::Vector3f(.48123 * scale, .5879 * scale, 0), Eigen::Vector3f(.1607 * scale, .5952 * scale, 0)),
		elbow_R_(Eigen::Vector3f(0, 1, 0)),
		wrist_offset_R_(Eigen::Vector3f(.77558 * scale, .60311 * scale, 0), Eigen::Vector3f(.48123 * scale, .5879 * scale, 0)),
		wrist_R_(BallJoint::ZYX),
		hip_offset_L_(Eigen::Vector3f(-.0904 * scale, .1037 * scale, 0), Eigen::Vector3f(0, .15 * scale, 0)),
		hip_L_(BallJoint::YXY),
		knee_offset_L_(Eigen::Vector3f(-.0862 * scale, -.4319 * scale, 0), Eigen::Vector3f(-.0904 * scale, .1037 * scale, 0)),
		knee_L_(Eigen::Vector3f(1, 0, 0)),
		ankle_offset_L_(Eigen::Vector3f(-.0596 * scale, -.9047 * scale, 0), Eigen::Vector3f(-.0862 * scale, -.4319 * scale, 0)),
		ankle_L_(BallJoint::ZXY),
		hip_offset_R_(Eigen::Vector3f(.0904 * scale, .1037 * scale, 0), Eigen::Vector3f(0, .15 * scale, 0)),
		hip_R_(BallJoint::YXY),
		knee_offset_R_(Eigen::Vector3f(.0862 * scale, -.4319 * scale, 0), Eigen::Vector3f(.0904 * scale, .1037 * scale, 0)),
		knee_R_(Eigen::Vector3f(1, 0, 0)),
		ankle_offset_R_(Eigen::Vector3f(.0596 * scale, -.9047 * scale, 0), Eigen::Vector3f(.0862 * scale, -.4319 * scale, 0)),
		ankle_R_(BallJoint::ZXY),
		neck_offset_(Eigen::Vector3f(0, .6866 * scale, 0), Eigen::Vector3f(0, .15 * scale, 0)),
		neck_(BallJoint::YXY),
		head_offset_(Eigen::Vector3f(0, .8766 * scale, 0), Eigen::Vector3f(0, .6866 * scale, 0)),
		head_tilt_(Eigen::Vector3f(1., 0, 0)),

		arm_L_(shoulder_offset_L_, shoulder_L_, elbow_offset_L_, elbow_L_, wrist_offset_L_, wrist_L_),
		arm_R_(shoulder_offset_R_, shoulder_R_, elbow_offset_R_, elbow_R_, wrist_offset_R_, wrist_R_),
		leg_L_(hip_offset_L_, hip_L_, knee_offset_L_, knee_L_, ankle_offset_L_, ankle_L_),
		leg_R_(hip_offset_R_, hip_R_, knee_offset_R_, knee_R_, ankle_offset_R_, ankle_R_),
		head_chain_(neck_offset_, neck_, head_offset_, head_tilt_),
		//humanoid_chain_(origin_position_,origin_,waist_rotation_, chest_rotation_, arm_L_,arm_R_,leg_L_,leg_R_,neck_offset_,neck_,head_offset_,head_tilt_),
		animation_iterator_(.3, .6),
		slider_panes_(1., 1., 1.),
		hitbox_("human_static_hitbox.obj", AnimationBase::debug_path),
		exact_hitbox_("human_B.obj", AnimationBase::debug_path),
		enlarged_hitbox_("human_combat_hitbox.obj", AnimationBase::debug_path),
		dyn_model_(model),
		body_models_{{BodyType::custom,model}}	{

		arm_L_.setRootTransform(&chest_rotation_.getEndTransform());
		arm_R_.setRootTransform(&chest_rotation_.getEndTransform());
		leg_L_.setRootTransform(&waist_rotation_.getEndTransform());
		leg_R_.setRootTransform(&waist_rotation_.getEndTransform());
		head_chain_.setRootTransform(&chest_rotation_.getEndTransform());
		chest_rotation_.setRootTransform(&origin_.getEndTransform());
		waist_rotation_.setRootTransform(&origin_.getEndTransform());
		origin_.setRootTransform(&origin_rotation_.getEndTransform());
		origin_position_.setRootTransform(&getPosition());
		origin_rotation_.setRootTransform(&origin_position_.getEndTransform());
		//origin_position_.setRootTransform(nullptr);

		refresh();



		//DynamicModel* model = new DynamicModel("human_B.obj", "human_B.txt");
		dyn_model_->getGroup("fingers_L")->setTform(&wrist_L_.getEndTransform());
		dyn_model_->getGroup("fingers_L")->setTform(&wrist_L_.getEndTransform());
		dyn_model_->getGroup("thumb_L")->setTform(&wrist_L_.getEndTransform());
		dyn_model_->getGroup("palm_L")->setTform(&wrist_L_.getEndTransform());
		dyn_model_->getGroup("forearm_L")->setTform(&elbow_L_.getEndTransform());
		dyn_model_->getGroup("humerus_L")->setTform(&shoulder_L_.getEndTransform());
		dyn_model_->getGroup("shoulder_L")->setTform(&shoulder_L_.getEndTransform());

		dyn_model_->getGroup("fingers_R")->setTform(&wrist_R_.getEndTransform());
		dyn_model_->getGroup("thumb_R")->setTform(&wrist_R_.getEndTransform());
		dyn_model_->getGroup("palm_R")->setTform(&wrist_R_.getEndTransform());
		dyn_model_->getGroup("forearm_R")->setTform(&elbow_R_.getEndTransform());
		dyn_model_->getGroup("humerus_R")->setTform(&shoulder_R_.getEndTransform());
		dyn_model_->getGroup("shoulder_R")->setTform(&shoulder_R_.getEndTransform());

		dyn_model_->getGroup("ribcage")->setTform(&chest_rotation_.getEndTransform());
		dyn_model_->getGroup("waist")->setTform(&waist_rotation_.getEndTransform());

		dyn_model_->getGroup("hip_L")->setTform(&hip_offset_L_.getEndTransform());
		dyn_model_->getGroup("thigh_L")->setTform(&hip_L_.getEndTransform());
		dyn_model_->getGroup("calf_L")->setTform(&knee_L_.getEndTransform());
		dyn_model_->getGroup("foot_L")->setTform(&ankle_L_.getEndTransform());

		dyn_model_->getGroup("hip_R")->setTform(&hip_offset_R_.getEndTransform());
		dyn_model_->getGroup("thigh_R")->setTform(&hip_R_.getEndTransform());
		dyn_model_->getGroup("calf_R")->setTform(&knee_R_.getEndTransform());
		dyn_model_->getGroup("foot_R")->setTform(&ankle_R_.getEndTransform());

		dyn_model_->getGroup("neck")->setTform(&neck_.getEndTransform());
		dyn_model_->getGroup("head")->setTform(&head_tilt_.getEndTransform());
		dyn_model_->getGroup("eye_L")->setTform(&head_tilt_.getEndTransform());
		dyn_model_->getGroup("eye_R")->setTform(&head_tilt_.getEndTransform());

		dyn_model_->offsetVerts();
		dyn_model_->setRootTransform(&getPosition());
		setModel(dyn_model_);
		setTexture(&skin_tone_2);

		edit_animation_mode_ = false;

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
	void openDebugUI(GameObject* UI_container, GLFWwindow* window, GraphicsRaw<GameObject>& graphics_2d, GraphicsRaw<Textbox>& text_graphics ) override {
		openDebugUI(getAnimations(), UI_container, window, graphics_2d, text_graphics);
	}

	void openDebugUI(const std::unordered_set<AnimationBase*>& animations,GameObject* UI_container, GLFWwindow* window, GraphicsRaw<GameObject>& graphics_2d, GraphicsRaw<Textbox>& text_graphics)  {
		GameObject::openDebugUI(UI_container, window, graphics_2d, text_graphics);

		slider_panes_.addPane(&torso_pane_, "Torso", .12);
		slider_panes_.addPane(&arm_L_pane_, "Arm L", .12);
		slider_panes_.addPane(&arm_R_pane_, "Arm R", .12);
		slider_panes_.addPane(&leg_L_pane_, "Leg L", .12);
		slider_panes_.addPane(&leg_R_pane_, "Leg R", .12);
		slider_panes_.addPane(&head_pane_, "Head", .12);
		slider_panes_.addPane(&position_pane_, "Pos", .12);
		slider_panes_.moveTo(-.5, -.5, 0);
		UI_container->addDependent(&slider_panes_);
		slider_panes_.load(window, graphics_2d, text_graphics);
		slider_panes_.clampTo(UI_container);

		std::array<int,7> dof_map = { 4,7,7,7,7,4,6 };
		std::array<Pane*,7> panes_map = { &torso_pane_,&arm_L_pane_,&arm_R_pane_,&leg_L_pane_,&leg_R_pane_,&head_pane_,&position_pane_ };


		int dof = 0;
		int dofs_in_tab;
		Pane* current_pane;
		for (int tab = 0; tab < dof_map.size();tab++) {
			current_pane = panes_map[tab];
			dofs_in_tab = dof_map[tab];
			for (int i = 0; i < dofs_in_tab; i++) {
				float limit;
				if (panes_map[tab] == &position_pane_) {
					if (i < 3) limit = 3.0;//translation
					else limit = 3 * M_PI; // add extra rotation
				} else {
					limit = M_PI;
				}
				debug_sliders_[dof] = new Slider(.6/dofs_in_tab, .5, -1*limit, limit);
				current_pane->addDependent(debug_sliders_[dof]);
				debug_sliders_[dof]->load(window, graphics_2d, text_graphics);
				debug_sliders_[dof]->moveTo(-.5, -.8*static_cast<float>(i) / (dofs_in_tab) - .2, 0);
				dof++;
			}
		}
		
		setSliderCallbacks();

		Button* prev_frame = new Button(.1,.35);
		Button* next_frame = new Button(.1,.35);
		Button* save_animation = new Button(.1,.8);
		Button* insert_new_frame = new Button(.1,.8);
		Button* toggle_edit_mode = new Button(.1, .8);
		Button* set_animation_start = new Button(.1, .8);
		Button* interpolate_frame = new Button(.1, .8);


		prev_frame->setLabel("prev frame");
		next_frame->setLabel("next frame");
		save_animation->setLabel("save animation");
		insert_new_frame->setLabel("  new frame  ");
		toggle_edit_mode->setLabel("toggle edit mode");
		set_animation_start->setLabel("set start frame");
		interpolate_frame->setLabel("interpolate frame");

		animation_iterator_.moveTo(.5, .9, 0);
		prev_frame->moveTo(.275, -.05, 0);
		next_frame->moveTo(.725, -.05, 0);
		insert_new_frame->moveTo(.5, -.2, 0);
		save_animation->moveTo(.5, -.35, 0);
		toggle_edit_mode->moveTo(.5, -.5, 0);
		set_animation_start->moveTo(.5, -.65, 0);
		interpolate_frame->moveTo(.5, -.8, 0);

		//issue here: if animation iterator changes current_animation without reloading debug menu it doesnt update current_animation_
		//should move static callbacks back into humanoid, but leave non-static helpers in animation
		prev_frame->setCallback(&prevFrame, this);
		next_frame->setCallback(&nextFrame, this);
		save_animation->setCallback(&saveAnimation, this);
		insert_new_frame->setCallback(&newFrame, this);
		set_animation_start->setCallback(&setAnimationStart, this);
		toggle_edit_mode->setCallback(&toggleEditMode, this);
		interpolate_frame->setCallback(&interpolateFrame, this);

		std::vector<Button*> anim_buttons{ prev_frame,next_frame,save_animation,insert_new_frame,toggle_edit_mode,set_animation_start,interpolate_frame };
		anim_buttons_ = anim_buttons;
		for (Button* button : anim_buttons_) {
			UI_container->addDependent(button);
			button->load(window, graphics_2d, text_graphics);
			button->activateMouseInput(window);//should be in button->load() call
		}
		UI_container->addDependent(&animation_iterator_);
		animation_iterator_.load(window, graphics_2d, text_graphics);
		animation_iterator_.setChangeCallback(setAnimation_static, this);
		animation_iterator_.setIterable(&animations);

		activateKeyInput(window);
		//edit_animation_mode_ = true;
	}
	void closeDebugUI(GameObject* UI_container, GLFWwindow* window, GraphicsRaw<GameObject>& graphics_2d, GraphicsRaw<Textbox>& text_graphics) override {
		std::array<int, 7> dof_map = { 4,7,7,7,7,4,6 };
		std::array<Pane*, 7> panes_map = { &torso_pane_,&arm_L_pane_,&arm_R_pane_,&leg_L_pane_,&leg_R_pane_,&head_pane_,&position_pane_ };

		int dof = 0;
		for (int tab = 0; tab < dof_map.size(); tab++) {
			int dofs_in_tab = dof_map[tab];
			for (int i = 0; i < dofs_in_tab; i++) {
				debug_sliders_[dof]->unload(window, graphics_2d, text_graphics);
				panes_map[tab]->destroyChild(debug_sliders_[dof]);
				debug_sliders_[dof] = nullptr;
				dof++;
			}
		}
		UI_container->removeDependent(&slider_panes_);
		slider_panes_.unload(window, graphics_2d, text_graphics);

		for (Button* button : anim_buttons_) {
			UI_container->removeDependent(button);
			button->unload(window, graphics_2d, text_graphics);
		}
		animation_iterator_.unload(window, graphics_2d, text_graphics);

		edit_animation_mode_ = false;
	}

	const MeshSurface& getEnlargedHitbox() const {
		return enlarged_hitbox_;
	}

	const MeshSurface& getHitbox() const {
		return hitbox_;
	}

	const MeshSurface& getExactHitbox() const {
		return exact_hitbox_;
	}

	const DynamicModel* getDynamicModel() const {
		return dyn_model_;
	}

	void setBodyType(BodyType body_type) {
		dyn_model_ = body_models_.at(body_type);
		setModel(dyn_model_);
	}

	static const std::unordered_set<Texture*>& getSkinTones() {
		return skin_tones;
	}
	static std::string getStringFromSkinTone(const Texture* tone) {
		if (tone == &skin_tone_1) {
			return "Skin Tone 1";
		} else if (tone == &skin_tone_2) {
			return "Skin Tone 2";
		} else if (tone == &skin_tone_3) {
			return "Skin Tone 3";
		} else if (tone == &skin_tone_4) {
			return "Skin Tone 4";
		} else if (tone == &skin_tone_5) {
			return "Skin Tone 5";
		} else if (tone == &skin_tone_6) {
			return "Skin Tone 6";
		} else {
			return "custom";
		}
	}
	static Texture* getSkinToneFromString(std::string tone) {
		if (tone == "Skin Tone 1") {
			return &skin_tone_1;
		} else if (tone == "Skin Tone 2") {
			return &skin_tone_2;
		} else if (tone == "Skin Tone 3") {
			return &skin_tone_3;
		} else if (tone == "Skin Tone 4") {
			return &skin_tone_4;
		} else if (tone == "Skin Tone 5") {
			return &skin_tone_5;
		} else if (tone == "Skin Tone 6") {
			return &skin_tone_6;
		} else {
			return nullptr;
		}
	}

	static const std::unordered_set<Texture*>& getEyeColors() {
		return eye_colors;
	}
	static std::string getStringFromEyeColor(const Texture* color) {
		if (color == &brown_eyes) {
			return "Brown";
		}
		else if (color == &amber_eyes) {
			return "Amber";
		}
		else if (color == &hazel_eyes) {
			return "Hazel";
		}
		else if (color == &green_eyes) {
			return "Green";
		}
		else if (color == &blue_eyes) {
			return "Blue";
		} else {
			return "custom";
		}
	}
	static Texture* getEyeColorFromString(std::string color) {
		if (color == "Blue") {
			return &blue_eyes;
		}
		else if (color == "Green") {
			return &green_eyes;
		}
		else if (color == "Hazel") {
			return &hazel_eyes;
		}
		else if (color == "Amber") {
			return &amber_eyes;
		}
		else if (color == "Brown") {
			return &brown_eyes;
		}
		else {
			return nullptr;
		}
	}

	void updateDynamicModel() {
		dyn_model_->updateData();
	}

	void setUpdateFreq(int freq) {
		update_freq_ = freq;
	}
	void setUpdateCount(int count) {
			update_count_ = count%update_freq_;
	}

	void TPose() {
		setState(Eigen::Vector<float, n_dofs>::Zero());
	}

	bool inEditMode() {
		return edit_animation_mode_;
	}


	const Eigen::Matrix4f& getOriginTform() const {
		return origin_rotation_.getEndTransform();
	}
	const Eigen::Matrix4f& getLeftHandTform() const {
		return wrist_L_.getEndTransform();
	}
	const Eigen::Matrix4f& getRightHandTform() const {
		return wrist_R_.getEndTransform();
	}
	const Eigen::Matrix4f& getLeftFootTform() const {
		return ankle_L_.getEndTransform();

	}
	const Eigen::Matrix4f& getRightFootTform() const {
		return ankle_R_.getEndTransform();

	}
	const Eigen::Matrix4f& getHeadTform() const {
		return head_tilt_.getEndTransform();
	}


	void setState(Eigen::Vector<float, n_dofs> new_state) {
		//this can all be automatically generated via templates
		origin_.setState(Eigen::Vector<float, 0>());
		waist_rotation_.setState(new_state(seq(0, 0)));
		chest_rotation_.setState(new_state(seq(1, 3)));
		arm_L_.setState(new_state(seq(4, 10)));
		arm_R_.setState(new_state(seq(11, 17)));
		leg_L_.setState(new_state(seq(18, 24)));
		leg_R_.setState(new_state(seq(25, 31)));
		head_chain_.setState(new_state(seq(32, 35)));
		origin_position_.setState(new_state(seq(36, 38)));
		origin_rotation_.setState(new_state(seq(39, 41)));
	}

	Eigen::Vector<float, n_dofs> getState() const {
		Eigen::Vector<float, n_dofs> ret;
		ret(seq(0, 0)) = waist_rotation_.getState();
		ret(seq(1, 3)) = chest_rotation_.getState();
		ret(seq(4, 10)) = arm_L_.getState();
		ret(seq(11, 17)) = arm_R_.getState();
		ret(seq(18, 24)) = leg_L_.getState();
		ret(seq(25, 31)) = leg_R_.getState();
		ret(seq(32, 35)) = head_chain_.getState();
		ret(seq(36, 38)) = origin_position_.getState();
		ret(seq(39, 41)) = origin_rotation_.getState();
		return ret;
	}


};

#endif // !PUPPET_GAMEOBJECT_HUMANOID
