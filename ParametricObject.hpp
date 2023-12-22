#pragma once

#include <Eigen/Dense>

#include "math_constants.hpp"

#include "GameObject.h"
#include "dynamic_model.hpp"
#include "UI.h"
#include "animation.hpp"

#ifndef PUPPET_PARAMETRICOBJECT
#define PUPPET_PARAMETRICOBJECT



template<int n_dofs>
class ParametricObject : public GameObject {

private:

	Eigen::Vector<float, n_dofs> clipboard_;

	std::array<Slider*, n_dofs> debug_sliders_;

	std::vector<Button*> anim_buttons_;
	UIIterator<AnimationBase> animation_iterator_;
	bool edit_animation_mode_;
	Animation<n_dofs>* edit_animation_;

	DynamicModel* dyn_model_;

	Eigen::Vector<float, n_dofs> state_;

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
		ParametricObject<n_dofs>* this_ = static_cast<ParametricObject<n_dofs>*>(must_be_this);
		Eigen::Vector<float, n_dofs> state = this_->getState();
		state(index) = angle;
		if (this_->edit_animation_mode_ && this_->edit_animation_ != nullptr) {
			this_->edit_animation_->setFrame(state);
		}
		else {
			this_->setState(state);
		}
	}



	void setState(Eigen::Vector<float, n_dofs> new_state) {
		state_ = new_state;
	}

	virtual void updateParameters() {

	}

	template<int index = n_dofs - 1>
	void setSliderCallbacks() {
		if (debug_sliders_[index] != nullptr) {
			debug_sliders_[index]->setSliderChangeCallback(&setDoFState<index>, this);
		}
		if constexpr (index > 0) {
			setSliderCallbacks<index - 1>();
		}
	}

	
	static void toggleEditMode(void* must_be_this) {
		ParametricObject<n_dofs>* this_ = static_cast<ParametricObject<n_dofs>*>(must_be_this);
		this_->edit_animation_mode_ = !this_->edit_animation_mode_;
	}

	static void setAnimation_static(AnimationBase* unused, AnimationBase* new_animation, void* must_be_this) {
		ParametricObject<n_dofs>* this_ = static_cast<ParametricObject<n_dofs>*>(must_be_this);

		//if debug menu is open
		this_->edit_animation_ = static_cast<Animation<n_dofs>*>(new_animation);
	}

	static void saveAnimation(void* must_be_this) {
		ParametricObject<n_dofs>* this_ = static_cast<ParametricObject<n_dofs>*>(must_be_this);
		this_->edit_animation_->saveAnimation();
	}
	static void newFrame(void* must_be_this) {
		ParametricObject<n_dofs>* this_ = static_cast<ParametricObject<n_dofs>*>(must_be_this);
		if (this_->edit_animation_ != nullptr) {
			this_->edit_animation_->newFrame();
		}
	}
	static void nextFrame(void* must_be_this) {
		ParametricObject<n_dofs>* this_ = static_cast<ParametricObject<n_dofs>*>(must_be_this);
		if (this_->edit_animation_ != nullptr) {
			this_->edit_animation_->nextFrame();
		}
	}
	static void prevFrame(void* must_be_this) {
		ParametricObject<n_dofs>* this_ = static_cast<ParametricObject<n_dofs>*>(must_be_this);
		if (this_->edit_animation_ != nullptr) {
			this_->edit_animation_->prevFrame();
		}
	}
	static void setAnimationStart(void* must_be_this) {
		ParametricObject<n_dofs>* this_ = static_cast<ParametricObject<n_dofs>*>(must_be_this);
		if (this_->edit_animation_ != nullptr) {
			this_->edit_animation_->setAnimationStart();
		}
	}
	static void interpolateFrame(void* must_be_this) {
		ParametricObject<n_dofs>* this_ = static_cast<ParametricObject<n_dofs>*>(must_be_this);
		if (this_->edit_animation_ != nullptr) {
			this_->edit_animation_->interpolateCurrentFrame();
		}
	}

protected:

	static constexpr int n_dofs = n_dofs;

	const Eigen::Vector<float, n_dofs>& getState() const {
		return state_;
	}

	void update(GLFWwindow* window) override {
		GameObject::update(window);
		Eigen::Vector<float, n_dofs> new_state;
		if (edit_animation_mode_ && edit_animation_ != nullptr) { //set state using edit_frame cursor
			new_state = edit_animation_->getFrame()(seq(1, n_dofs));
		}
		else if (!edit_animation_mode_ && getActiveAnimation() != nullptr) {//set state using elapsed time
			new_state = dynamic_cast<const Animation<n_dofs>*>(getActiveAnimation())->getState();
		}
		else {
			new_state = Eigen::Vector<float, n_dofs>::Zero();
		}
		setState(new_state);
		updateParameters();
		if (dyn_model_ != nullptr) {
			dyn_model_->updateData();
		}
		refreshDebugSliders();
	}

	void onKeyPress(int key) override {
		if (key == GLFW_KEY_C) {
			clipboard_ = getState();
		}
		else if (key == GLFW_KEY_V) {
			if (this->edit_animation_mode_ && this->edit_animation_ != nullptr) {
				this->edit_animation_->setFrame(clipboard_);
			}
		}
	}

public:

	ParametricObject(std::string name, const KeyStateCallback_base& key_state_callback_caller = InternalObject::no_key_state_callback, const ControllerStateCallback_base& controller_state_callback_caller = InternalObject::no_controller_state_callback) :
		GameObject(name, key_state_callback_caller, controller_state_callback_caller),
		animation_iterator_(.3, .6),
		state_(Eigen::Vector<float,n_dofs>::Zero()){

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

	void openDebugUI(GameObject* UI_container, GLFWwindow* window, GraphicsRaw<GameObject>& graphics_2d, GraphicsRaw<Textbox>& text_graphics) override {
		GameObject::openDebugUI(UI_container, window, graphics_2d, text_graphics);
		float slider_height = .6 / n_dofs;
		for (int i = 0; i < n_dofs; i++) {
			debug_sliders_[i] = new Slider(slider_height, .5, -M_PI, M_PI);
			UI_container->addDependent(debug_sliders_[i]);
			debug_sliders_[i]->load(window, graphics_2d, text_graphics);
			debug_sliders_[i]->moveTo(-.5, -static_cast<float>(i) / (n_dofs), 0);
		}
		setSliderCallbacks();

		Button* prev_frame = new Button(.1, .35);
		Button* next_frame = new Button(.1, .35);
		Button* save_animation = new Button(.1, .8);
		Button* insert_new_frame = new Button(.1, .8);
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
		animation_iterator_.setIterable(&getAnimations());

		//edit_animation_mode_ = true;
	}
	void closeDebugUI(GameObject* UI_container, GLFWwindow* window, GraphicsRaw<GameObject>& graphics_2d, GraphicsRaw<Textbox>& text_graphics) override {
		for (int i = 0; i < n_dofs; i++) {
			debug_sliders_[i]->unload(window, graphics_2d, text_graphics);
			UI_container->destroyChild(debug_sliders_[i]);
			debug_sliders_[i] = nullptr;
		}
		for (Button* button : anim_buttons_) {
			UI_container->removeDependent(button);
			button->unload(window, graphics_2d, text_graphics);
		}
		animation_iterator_.unload(window, graphics_2d, text_graphics);

		edit_animation_mode_ = false;
	}

	void ZeroPose() {
		setState(Eigen::Vector<float, n_dofs>::Zero());
	}

	void setDynamicModel(DynamicModel* dyn_model) {
		dyn_model_ = dyn_model;
	}
	/*
	void setDynamicModelGroupTransform(std::string group_name, const Eigen::Matrix4f* tform) {
		dyn_model_->getGroup(group_name)->setTform(tform);
	}
	*/

};

#endif // !PUPPET_GAMEOBJECT_HUMANOID
