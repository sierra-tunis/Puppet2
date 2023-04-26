#pragma once

#ifndef PUPPET_UI
#define PUPPET_UI

#include "Model.h"
#include "GameObject.h"

class Rect2d : public Model {
private:

	constexpr static std::vector<float> RectVerts(float height, float width) {
		return std::vector<float>{width/2,height/2,0,-width/2,height/2,0,width/2,-height/2,0,-width/2,-height/2,0 };
	}
	constexpr static std::vector<float> RectNorms() {
		return std::vector<float>{0.,0.,-1.,0,0,-1,0,0,-1,0,0,-1};
	}
	constexpr static std::vector<float> RectTex(float height, float width, float top, float left) {
		//(height ,width, 0,0) is top left
		return std::vector<float>{left, top, left+width, top, left, top+height, left+width, top+height};
	}
	constexpr static std::vector<unsigned int> RectFace() {
		return std::vector<unsigned int>{0,1,2,3};
	}
	constexpr static std::vector<unsigned int> RectFaceNorm() {
		return std::vector<unsigned int>{0, 1, 2, 3};
	}
	constexpr static std::vector<unsigned int> RectFaceTex() {
		return std::vector<unsigned int>{0, 1, 2, 3};
	}


public:

	Rect2d(float height, float width):
	Model(RectVerts(height,width),RectNorms(),RectTex(1.,1.,0.,0.),RectFace(),RectFaceNorm(),RectFaceTex()){}
};

class Button : public GameObject{

	const float height_, width_;
//	float x_, y_; //center coord
	Rect2d button_model_;

	void* callback_input_;
	void (*callback_func_)(void*);

	float getX() const {
		return getPosition()(0, 3);
	}

	float getY() const {
		return getPosition()(1, 3);
	}

	void onMouseDown(int key,float x, float y) override {
		if (!isHidden() &&
			x > getX() - width_ / 2 && x < getX() + width_ / 2
			&& y > getY() - height_ / 2 && y < getY() + height_ / 2) {
			if (callback_func_ != nullptr){
				callback_func_(callback_input_);
			}
			//std::cout << "within button!\n";
		}
	}

public:
	Button(float height, float width, std::string name) : 
		GameObject(name),
		height_(height),
		width_(width),
		button_model_(height_,width_){

		setModel(&button_model_);
		setTexture(new Texture("rocky.jpg"));
	}

	void setCallback(void (*callback_func)(void*), void* callback_input) {
		callback_func_ = callback_func;
		callback_input_ = callback_input;
	}
};
/*
template<class T>
concept GameObjectIterator = requires(const T & iterator, unsigned int i) {
	//{*iterator[i]} ->  std::convertible_to<GameObject*>;
	{iterator.size()} -> std::convertible_to<size_t>;
	{iterator.begin()} -> std::convertible_to<std::iterator< std::bidirectional_iterator_tag, GameObject*>>;
};

template<GameObjectIterator iterator_T>
class UIIterator : public GameObject{
	Button next_target_;
	Button prev_target_;

	const iterator_T* iterable_;
	std::iterator< std::bidirectional_iterator_tag, GameObject*> target_iterator_;
	Textbox target_name_;

	void (*callback_on_increment_)(GameObject*, GameObject*);
	void (*callback_on_decrement_)(GameObject*, GameObject*);


	static void nextTargetCallback(void* must_be_this) {
		UIIterator* this_ = static_cast<DebugMenu*>(must_be_this);
		if (this_->iterable == nullptr) { return; };
		if (this_->target_iterator_ == iterable_.end()) {
			target_iterator_ = iterable_.begin();
			callback_on_increment_(nullptr, *target_iterator_);
			return;
		}
		GameObject* prev_target = *target_iterator;
		target_iterator_++;
		callback_on_increment_(prev_target, *target_iterator_);
	}

	static void prevTargetCallback(void* must_be_this) {
		UIIterator* this_ = static_cast<DebugMenu*>(must_be_this);
		if (this_->iterable == nullptr) { return; };
		if (this_->target_iterator_ == iterable_.begin()) {
			target_iterator_ = iterable_.end();
			callback_on_increment_(*iterator_.begin(),nullptr);
			return;
		}
		GameObject* prev_target = *target_iterator;
		target_iterator_--;
		callback_on_increment_(prev_target, *target_iterator_);
	}


public:
	float button_size;
	float text_width;
	float text_height;
	float top;
	float left;

	class UIIterator(std::string name, Default2d graphics) :
		GameObject(name),
		next_target_(.2, .2, name + "_next_target"),
		prev_target_(.2, .2, name + "_prev_target") {


		prev_target_.moveTo(-1, -.4, 0);
		prev_target_.activateMouseInput(window);
		graphics.add(prev_target_);
		next_target_.moveTo(-.2, -.4, 0);
		next_target_.activateMouseInput(window);
		graphics.add(next_target_);
		prev_target_.setCallback(prevTargetCallback,this);
		next_target_.setCallback(nextTargetCallback,this);

		target_name_.box_width = .6;
		target_name_.box_height = .2;
		target_name_.font_size = 1;
		target_name_.left = -.8;
		target_name_.top = -.4;

	}

	GameObject* getTarget() {
		return *target_iterator_;
	}

};
*/
#endif