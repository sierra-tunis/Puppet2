#pragma once

#ifndef PUPPET_ANIMATION
#define PUPPET_ANIMATION

#include <string>
#include "sequence.h"

class AnimationBase {

	float elapsed_time_;
	float playback_speed_;

protected:

	virtual bool load() = 0;

	virtual bool unload() = 0;

	std::string fname_;
	bool loaded_;
	bool paused_;
	int edit_frame_;


public:

	float setPlaybackSpeed(float playback_speed) {
		playback_speed_ = playback_speed;
	}


	virtual void advance(float dt) {
		if (!paused_) {
			elapsed_time_ += dt;
		}
	}

	void start() {
		elapsed_time_ = 0;
		advance(0);
		paused_ = false;
	}
	void play() {
		paused_ = false;
	}
	void pause() {
		paused_ = true;
	}
	void stop() {
		elapsed_time_ = 0;
		paused_ = true;
	}

	float getElapsed() const {
		return elapsed_time_;
	}

	/*
	template <int n_dofs>
	const Eigen::Vector<float, n_dofs>& getState() {
		std::cerr << "illegal function call (not callable on AnimationBase)";
	}*/

	AnimationBase(std::string fname) :fname_(fname),elapsed_time_(0), playback_speed_(1),edit_frame_(0) {

	}

};

template<int n_dofs>
class Animation : public AnimationBase {

	StateSequence<n_dofs>* animation_data_;
	Eigen::Vector<float, n_dofs> current_state_;

public:
	bool load() final override {
		loaded_ = true;
		animation_data_ = new StateSequence<n_dofs>();
		if (!animation_data_->readFromFile(fname_)) {
			animation_data_->addCol(0, Eigen::Vector<float, n_dofs>::Constant(0));
		}
		return true;
		
	}
	/*
	bool save() const {
		animation_data_->saveToFile(fname_);
	}

	bool saveAs(std::string fname) const {
		animation_data_->saveToFile(fname);
	}*/

	bool unload() final override {
		loaded_ = false;
		delete animation_data_;
		animation_data_ = nullptr;
		return true;
	}

	void advance(float dt) final override {
		AnimationBase::advance(dt);
		current_state_ = animation_data_->getState(getElapsed());
	}

	const Eigen::Vector<float, n_dofs>& getState() const {
		return current_state_;
	}

	Eigen::Vector<float, n_dofs + 1> getFrame() const {
		return animation_data_->getData(edit_frame_);
	}

	void setFrame(Eigen::Vector<float, n_dofs> state) {
		animation_data_->setCol(edit_frame_, animation_data_->getColTime(edit_frame_), state);
	}

	constexpr int size() {
		return animation_data_->size();
	}

	Animation(std::string fname):
		AnimationBase(fname),
		animation_data_(nullptr),
		current_state_(Eigen::Vector<float, n_dofs>::Constant(0)){

	}

	static void saveAnimation(void* animation) {
		Animation<n_dofs>* this_ = static_cast<Animation<n_dofs>*>(animation);
		this_->animation_data_->saveToFile(this_->fname_);
	}


	static void newFrame(void* animation) {
		Animation<n_dofs>* this_ = static_cast<Animation<n_dofs>*>(animation);
		this_->animation_data_->addCol(this_->animation_data_->getLastTime() + 1.,
			Eigen::Vector<float, n_dofs>::Constant(0));
		this_->edit_frame_ = this_->animation_data_->size() - 1;
	}

	static void nextFrame(void* animation) {
		Animation<n_dofs>* this_ = static_cast<Animation<n_dofs>*>(animation);
		this_->edit_frame_++;
		if (this_->edit_frame_ >= this_->size()) {
			this_->edit_frame_ = this_->size() - 1;
		}
	}

	static void prevFrame(void* animation) {
		Animation<n_dofs>* this_ = static_cast<Animation<n_dofs>*>(animation);
		this_->edit_frame_--;
		if (this_->edit_frame_ < 0) {
			this_->edit_frame_ = 0;
		}
	}

};



#endif // !PUPPET_ANIMATION
