#pragma once

#ifndef PUPPET_ANIMATION
#define PUPPET_ANIMATION

#include <string>
#include <iostream>
#include "sequence.h"

class AnimationBase {

	float elapsed_time_;
	float playback_speed_;
	bool looping_;
	float length_;

protected:

	std::string fname_;
	bool loaded_;
	bool paused_;
	int edit_frame_;


public:
	static constexpr char debug_path[] = "C:\\Users\\Sierra\\source\\repos\\Puppet2\\Puppet2\\assets\\";
	static std::string default_path;

	virtual bool load() = 0;

	virtual bool unload() = 0;

	void  setPlaybackSpeed(float playback_speed) {
		playback_speed_ = playback_speed;
	}


	virtual void advance(float dt, bool* animation_over=nullptr) {
		if (!paused_) {
			elapsed_time_ += playback_speed_*dt;
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
		advance(0);
		paused_ = true;
	}

	float getElapsed() const {
		return elapsed_time_;
	}

	std::string getName() const {
		return fname_;
	}

	float getLength() const {
		return length_;
	}

	void setLength(float length) {
		length_ = length;
	}

	bool isLooping() const {
		return looping_;
	}

	void setLooping(bool looping) {
		looping_ = looping;
	}
	/*
	template <int n_dofs>
	const Eigen::Vector<float, n_dofs>& getState() {
		std::cerr << "illegal function call (not callable on AnimationBase)";
	}*/

	AnimationBase(std::string fname) :
		fname_(fname),elapsed_time_(0),
		playback_speed_(1),
		edit_frame_(0),
		looping_(true),
		length_(0){

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
		std::ifstream animFile(AnimationBase::default_path + fname_);
		if (animFile.is_open()) {
			std::string anim_info;
			std::getline(animFile, anim_info);
			if (anim_info[0] == 'v') {
				std::string file_version;
				std::string playback_speed;
				std::string looping;
				std::stringstream ss(anim_info);
				std::getline(ss, file_version, ' ');
				std::getline(ss, playback_speed, ' ');
				std::getline(ss, looping, ' ');
				setPlaybackSpeed(std::stof(playback_speed));
				setLooping(looping == "true");
			}
			animFile.close();
		}
		if (!animation_data_->readFromFile(fname_, AnimationBase::default_path)) {
			animation_data_->addCol(0, Eigen::Vector<float, n_dofs>::Constant(0));
		}
		setLength(animation_data_->getLastTime());
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

	void advance(float dt, bool* animation_over=nullptr) final override{
		if (animation_data_ == nullptr) {
			std::cerr << "animation not loaded!\n";
		}
		AnimationBase::advance(dt);
		current_state_ = animation_data_->getState(getElapsed());
		if (!isLooping() && getLength() > 0 && getElapsed() >= getLength()) {
			stop();
			if (animation_over != nullptr) {
				*animation_over = true;
			}
		}
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

	bool saveAnimation() const {
		return animation_data_->saveToFile(fname_, AnimationBase::default_path);
	}


	void newFrame() {
		animation_data_->addCol(animation_data_->getLastTime() + 1.,
			Eigen::Vector<float, n_dofs>::Constant(0));
		edit_frame_ = animation_data_->size() - 1;
	}
	void nextFrame() {
		edit_frame_++;
		if (edit_frame_ >= size()) {
			edit_frame_ = size() - 1;
		}
	}

	void prevFrame() {
		edit_frame_--;
		if (edit_frame_ < 0) {
			edit_frame_ = 0;
		}
	}


	void setAnimationStart() {
		StateSequence<n_dofs> new_animation_data;
		for (int i = 0; i < size(); i++) {
			int old_idx = (i + edit_frame_) % size();
			new_animation_data.addCol(animation_data_->getData(old_idx));
		}
		*(animation_data_) = new_animation_data;
		
	}

	void interpolateCurrentFrame() {
		if (edit_frame_ > 0 && edit_frame_ < size() - 1) {
			Eigen::Vector<float, n_dofs + 1> interp_data = (animation_data_->getData(edit_frame_ - 1) + animation_data_->getData(edit_frame_ + 1))/2;
			animation_data_->setCol(edit_frame_, interp_data);
		}
	}

};



#endif // !PUPPET_ANIMATION
