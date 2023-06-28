#pragma once

#ifndef PUPPET_GAMEOBJECT_HUMANOID
#define PUPPET_GAMEOBJECT_HUMANOID

#include "GameObject.h"
#include "dynamic_model.hpp"
#include "UI.h"
#include "animation.hpp"

using LimbConnector = ConnectorChain<OffsetConnector, BallJoint, OffsetConnector, RotationJoint, OffsetConnector, BallJoint>;

class Humanoid : public GameObject {
protected:
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

	OffsetConnector neck_offset_;
	BallJoint neck_;
	OffsetConnector head_offset_;
	RotationJoint head_tilt_;

	ConnectorChain<OffsetConnector, BallJoint, OffsetConnector, RotationJoint> head_chain_;

	LimbConnector arm_L_;
	LimbConnector arm_R_;
	LimbConnector leg_L_;
	LimbConnector leg_R_;

public:
	static constexpr int n_dofs = 2*RotationJoint::getDoF() + 2*BallJoint::getDoF() + 4 * LimbConnector::getDoF();
	static constexpr int upper_dofs_ = 2 * LimbConnector::getDoF() + 2 * BallJoint::getDoF() + RotationJoint::getDoF();
	static constexpr int lower_dofs_ = 2 * LimbConnector::getDoF() + RotationJoint::getDoF();

private:

	Eigen::Vector<float, n_dofs> clipboard_;

	std::array<Slider*,n_dofs> debug_sliders_;

	DynamicModel* dyn_model_;
	Animation<n_dofs>* current_animation_;

	std::vector<Button*> anim_buttons_;
	UIIterator<AnimationBase> animation_iterator_;
	bool edit_animation_mode_;
	Animation<n_dofs>* edit_animation_;

	MeshSurface hitbox_;

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

	

	void setState(Eigen::Vector<float, n_dofs> new_state) {
		//this can all be automatically generated via templates
		origin_.setState(Eigen::Vector<float, 0>());
		waist_rotation_.setState(new_state(seq(0, 0)));
		chest_rotation_.setState(new_state(seq(1, 3)));
		arm_L_.setState(new_state(seq(4, 10)));
		arm_R_.setState(new_state(seq(11,17)));
		leg_L_.setState(new_state(seq(18,24)));
		leg_R_.setState(new_state(seq(25,31)));
		head_chain_.setState(new_state(seq(32, 35)));
	}

	Eigen::Vector<float,n_dofs> getState() const {
		Eigen::Vector<float, n_dofs> ret;
		ret(seq(0,0)) = waist_rotation_.getState();
		ret(seq(1, 3)) = chest_rotation_.getState();
		ret(seq(4, 10)) = arm_L_.getState();
		ret(seq(11, 17)) = arm_R_.getState();
		ret(seq(18, 24)) = leg_L_.getState();
		ret(seq(25, 31)) = leg_R_.getState();
		ret(seq(32, 35)) = head_chain_.getState();
		return ret;
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
protected:
	void onStep() {
		dyn_model_->updateData();
		Eigen::Vector<float, n_dofs> new_state;
		if (edit_animation_mode_ && edit_animation_ != nullptr) { //set state using edit_frame cursor
			new_state = edit_animation_->getFrame()(seq(1, n_dofs));
		}
		else if (!edit_animation_mode_ && current_animation_ != nullptr) {//set state using elapsed time
			new_state = current_animation_->getState();
		}
		else {
			new_state = Eigen::Vector<float, n_dofs>::Constant(0);
		}
		setState(new_state);

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

	Humanoid(std::string name, const KeyStateCallback_base& key_state_callback_caller):
		GameObject(name,key_state_callback_caller),
		origin_(0, .15, 0),
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
		neck_offset_(Vector3f(0,.6866,0), Eigen::Vector3f(0, .15, 0)),
		neck_(BallJoint::YXY),
		head_offset_(Vector3f(0, .8766, 0),Vector3f(0, .6866, 0)),
		head_tilt_(Eigen::Vector3f(1.,0,0)),

		arm_L_(shoulder_offset_L_, shoulder_L_, elbow_offset_L_, elbow_L_, wrist_offset_L_, wrist_L_),
		arm_R_(shoulder_offset_R_, shoulder_R_, elbow_offset_R_, elbow_R_, wrist_offset_R_, wrist_R_),
		leg_L_(hip_offset_L_, hip_L_, knee_offset_L_, knee_L_, ankle_offset_L_, ankle_L_),
		leg_R_(hip_offset_R_, hip_R_, knee_offset_R_, knee_R_, ankle_offset_R_, ankle_R_),
		head_chain_(neck_offset_,neck_,head_offset_,head_tilt_),
		animation_iterator_(.3,.6),
		hitbox_("human_static_hitbox.obj", AnimationBase::debug_path) {

		arm_L_.setRootTransform(&chest_rotation_.getEndTransform());
		arm_R_.setRootTransform(&chest_rotation_.getEndTransform());
		leg_L_.setRootTransform(&waist_rotation_.getEndTransform());
		leg_R_.setRootTransform(&waist_rotation_.getEndTransform());
		head_chain_.setRootTransform(&chest_rotation_.getEndTransform());
		chest_rotation_.setRootTransform(&origin_.getEndTransform());
		waist_rotation_.setRootTransform(&origin_.getEndTransform());
		origin_.setRootTransform(&getPosition());

		refresh();

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
		vert_tforms[model->getInd("shoulder_L")] = &shoulder_L_.getEndTransform();

		vert_tforms[model->getInd("fingers_R")] = &wrist_R_.getEndTransform();
		vert_tforms[model->getInd("thumb_R")] = &wrist_R_.getEndTransform();
		vert_tforms[model->getInd("palm_R")] = &wrist_R_.getEndTransform();
		vert_tforms[model->getInd("forearm_R")] = &elbow_R_.getEndTransform();
		vert_tforms[model->getInd("humerus_R")] = &shoulder_R_.getEndTransform();
		vert_tforms[model->getInd("shoulder_R")] = &shoulder_R_.getEndTransform();

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

		vert_tforms[model->getInd("neck")] = &neck_.getEndTransform();
		vert_tforms[model->getInd("head")] = &head_tilt_.getEndTransform();
		vert_tforms[model->getInd("eye_L")] = &head_tilt_.getEndTransform();
		vert_tforms[model->getInd("eye_R")] = &head_tilt_.getEndTransform();



		model->setVertTforms(vert_tforms);
		model->offsetVerts();
		dyn_model_ = model;
		setModel(model);
		setTexture(new Texture("rocky.jpg", Texture::debug_path));

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

	void openDebugUI(const GameObject* UI_container, GLFWwindow* window, GraphicsRaw<GameObject>& graphics_2d, GraphicsRaw<Textbox>& text_graphics) override {
		float slider_height = .6 / n_dofs;
		int n_sections = 6;
		for (int i = 0; i < n_dofs; i++) {
			debug_sliders_[i] = new Slider(slider_height, .5, -M_PI, M_PI);
			addDependent(debug_sliders_[i]);
			debug_sliders_[i]->load(window, graphics_2d, text_graphics);
			debug_sliders_[i]->setParent(UI_container);
		}
		for (int i = 0; i < 4; i++) {
			debug_sliders_[i]->moveTo(-.5, -static_cast<float>(i) / (n_dofs+n_sections), 0);
		}
		for (int i = 4; i < 11; i++) {
			debug_sliders_[i]->moveTo(-.5, -static_cast<float>(i+1) / (n_dofs + n_sections), 0);
		}
		for (int i = 11; i < 18; i++) {
			debug_sliders_[i]->moveTo(-.5, -static_cast<float>(i+2) / (n_dofs + n_sections), 0);
		}
		for (int i = 18; i < 25; i++) {
			debug_sliders_[i]->moveTo(-.5, -static_cast<float>(i+3) / (n_dofs + n_sections), 0);
		}
		for (int i = 25; i < 32; i++) {
			debug_sliders_[i]->moveTo(-.5, -static_cast<float>(i+4) / (n_dofs + n_sections), 0);
		}
		for (int i = 32; i < n_dofs; i++) {
			debug_sliders_[i]->moveTo(-.5, -static_cast<float>(i+5) / (n_dofs + n_sections), 0);
		}
		
		setSliderCallbacks();

		Button* prev_frame = new Button(.1,.35);
		Button* next_frame = new Button(.1,.35);
		Button* save_animation = new Button(.1,.8);
		Button* insert_new_frame = new Button(.1,.8);
		Button* toggle_edit_mode = new Button(.1, .8);
		Button* set_animation_start = new Button(.1, .8);


		prev_frame->setLabel("prev frame");
		next_frame->setLabel("next frame");
		save_animation->setLabel("save animation");
		insert_new_frame->setLabel("  new frame  ");
		toggle_edit_mode->setLabel("toggle edit mode");
		set_animation_start->setLabel("set start frame");

		animation_iterator_.moveTo(.5, .9, 0);
		prev_frame->moveTo(.275, -.1, 0);
		next_frame->moveTo(.725, -.1, 0);
		insert_new_frame->moveTo(.5, -.3, 0);
		save_animation->moveTo(.5, -.5, 0);
		toggle_edit_mode->moveTo(.5, -.7, 0);
		set_animation_start->moveTo(.5, -.9, 0);

		//issue here: if animation iterator changes current_animation without reloading debug menu it doesnt update current_animation_
		//should move static callbacks back into humanoid, but leave non-static helpers in animation
		prev_frame->setCallback(&prevFrame, this);
		next_frame->setCallback(&nextFrame, this);
		save_animation->setCallback(&saveAnimation, this);
		insert_new_frame->setCallback(&newFrame, this);
		set_animation_start->setCallback(&setAnimationStart, this);
		toggle_edit_mode->setCallback(&toggleEditMode, this);

		std::vector<Button*> anim_buttons{ prev_frame,next_frame,save_animation,insert_new_frame,toggle_edit_mode,set_animation_start };
		anim_buttons_ = anim_buttons;
		for (Button* button : anim_buttons_) {
			addDependent(button);
			button->setParent(UI_container);
			button->load(window, graphics_2d, text_graphics);
			button->activateMouseInput(window);//should be in button->load() call
		}
		addDependent(&animation_iterator_);
		animation_iterator_.setParent(UI_container);
		animation_iterator_.load(window, graphics_2d, text_graphics);
		animation_iterator_.setChangeCallback(setAnimation_static, this);
		animation_iterator_.setIterable(&getAnimations());

		edit_animation_mode_ = true;
	}
	void closeDebugUI(const GameObject* UI_container, GLFWwindow* window, GraphicsRaw<GameObject>& graphics_2d, GraphicsRaw<Textbox>& text_graphics) override {
		for (int i = 0; i < n_dofs; i++) {
			removeDependent(debug_sliders_[i]);
			debug_sliders_[i]->unload(window, graphics_2d, text_graphics);
			delete debug_sliders_[i];
			debug_sliders_[i] = nullptr;
		}
		for (Button* button : anim_buttons_) {
			removeDependent(button);
			button->setParent(nullptr);
			button->unload(window, graphics_2d, text_graphics);
		}
		animation_iterator_.unload(window, graphics_2d, text_graphics);

		edit_animation_mode_ = false;
	}

	void setSkeletonAnimation(Animation<n_dofs>* animation) {
		current_animation_ = animation;
	}

	const MeshSurface& getHitbox() const {
		return hitbox_;
	}


};

#endif // !PUPPET_GAMEOBJECT_HUMANOID
