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

	Button(float height, float width) :
		height_(height),
		width_(width),
		button_model_(height_, width_) {

		setModel(&button_model_);
		setTexture(new Texture("rocky.jpg"));
	}

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

class Slider : public GameObject {
	float lower_limit_, upper_limit_;
	float current_position_;
	float height_, width_;
	float increment_amount_;

	void (*slider_change_callback_)(float, void*); //float being the new slider value

	Button increment_;
	Button decrement_;
	Button slider_;

	Textbox current_value_;
	Textbox lower_limit_value_;
	Textbox upper_limit_value_;

	OffsetConnector increment_offset_;
	OffsetConnector decrement_offset_;
	PrismaticJoint slider_connector_;

public:
	Slider(float height, float width, float lower_limit, float upper_limit):
	GameObject(),
	height_(height),
	width_(width),
	increment_(height_,width_/20),
	decrement_(height_,width_/20),
	slider_(height_,width/100),
	slider_change_callback_(nullptr),
	lower_limit_(lower_limit),
	upper_limit_(upper_limit),
	current_position_(lower_limit/2 + upper_limit/2),
	increment_offset_(width_*(.5 + 1./20.),0,0),
	decrement_offset_(width_*(-.5-1./20.),0,0),
	slider_connector_(Eigen::Vector3f(1.,0.,0.)){
		
		increment_.connectTo(this, &increment_offset_);
		decrement_.connectTo(this, &decrement_offset_);

		slider_.connectTo(this, &slider_connector_);

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