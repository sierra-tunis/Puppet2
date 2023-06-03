#pragma once

#ifndef PUPPET_UI
#define PUPPET_UI

#include "Model.h"
#include "GameObject.h"
#include "Default2d.hpp"
#include "textbox_object.hpp"

Texture rect_tex("puppet_button.jpg");
Texture border_rect_tex("puppet_border.jpg");

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
	GraphicsRaw<Textbox>* text_graphics_;

	TextboxObject label_;

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
		button_model_(height_, width_),
		text_graphics_(nullptr){

		setModel(&button_model_);
		setTexture(&rect_tex);
		label_.box_width = width_*.9;
		label_.connectTo(this, new OffsetConnector(0, 0, 0));
		addDependent(&label_);
	}

	Button(float height, float width, std::string name) : 
		GameObject(name),
		height_(height),
		width_(width),
		button_model_(height_,width_),
		text_graphics_(nullptr){

		setModel(&button_model_);
		setTexture(&rect_tex);
		label_.box_width = width_*.9;
		label_.connectTo(this, new OffsetConnector(0, 0, 0));//idk why this needs to be height/2 instead of 0
		addDependent(&label_);
	}

	void setCallback(void (*callback_func)(void*), void* callback_input) {
		callback_func_ = callback_func;
		callback_input_ = callback_input;
	}

	float getHeight() const {
		return height_;
	}
	float getWidth() const {
		return width_;
	}

	void setLabel(std::string label) {
		if (text_graphics_ == nullptr) {
			label_.text = label;
		}
		else {
			text_graphics_->unload(label_);
			label_.text = label;
			text_graphics_->add(label_);
		}
		label_.font_size = label_.box_width / ((label_.text.size()+1) * char_info('A').unscaled_width);
		label_.box_height = char_info('A').unscaled_height * label_.font_size;
	}

	void load(GLFWwindow* window, GraphicsRaw<GameObject>& graphics_2d, GraphicsRaw<Textbox>& text_graphics) {
		graphics_2d.add(*this);
		text_graphics.add(this->label_);
		activateMouseInput(window);
		const TextboxObject& tmp = label_;
		text_graphics_ = &text_graphics;
		
	}

	void unload(GLFWwindow* window, GraphicsRaw<GameObject>& graphics_2d, GraphicsRaw<Textbox>& text_graphics) {
		graphics_2d.unload(*this);
		text_graphics.unload(this->label_);
		deactivateMouseInput(window);
		text_graphics_ = nullptr;
	}

};

class Slider : public GameObject {
	float lower_limit_, upper_limit_;
	float current_position_;
	float height_, width_;
	float increment_fraction_;

	Rect2d slider_model_;

	void (*slider_change_callback_)(float, void*); //float being the new slider value
	void* callback_input_;

	Button increment_;
	Button decrement_;
	Button slider_;
	bool mouse_drag_mode_;

	TextboxObject current_value_;
	TextboxObject lower_limit_value_;
	TextboxObject upper_limit_value_;

	OffsetConnector increment_offset_;
	OffsetConnector decrement_offset_;
	OffsetConnector current_val_offset_;
	OffsetConnector lower_lim_offset_;
	OffsetConnector upper_lim_offset_;
	PrismaticJoint slider_connector_;

	GraphicsRaw<GameObject>* graphics_2d_;
	GraphicsRaw<Textbox>* text_graphics_;

	static void incrementCallback(void* must_be_this) {
		Slider* this_ = static_cast<Slider*>(must_be_this);
		float new_val = this_->current_position_ + this_->increment_fraction_ * (this_->upper_limit_ - this_->lower_limit_);
		if (new_val > this_->upper_limit_) {
			new_val = this_->upper_limit_;
		}
		this_->setCurrentValue(new_val);
	}
	static void decrementCallback(void* must_be_this) {
		Slider* this_ = static_cast<Slider*>(must_be_this);
		float new_val = this_->current_position_ - this_->increment_fraction_ * (this_->upper_limit_ - this_->lower_limit_);
		if (new_val < this_->lower_limit_) {
			new_val = this_->lower_limit_;
		}
		this_->setCurrentValue(new_val);
	}
	static void beginMouseDrag(void* must_be_this) {
		Slider* this_ = static_cast<Slider*>(must_be_this);
		this_->mouse_drag_mode_ = true;
	}
	
	void onMouseUp(int key, float x, float y) override {
		mouse_drag_mode_ = false;
	}

	void onMouseMove(float x, float y, float dx, float dy) {
		if (mouse_drag_mode_) {
			float magic_scale_factor = 900.;//probably needs to be window resolution
			float new_val = current_position_ + dx/width_/magic_scale_factor * (upper_limit_ - lower_limit_);
			if (new_val < lower_limit_) {
				new_val = lower_limit_;
			}
			setCurrentValue(new_val);
		}
	}

public:
	Slider(float height, float width, float lower_limit, float upper_limit):
		GameObject(),
		height_(height),
		width_(width),
		slider_model_(height,width),
		increment_(height_,width_/10),
		decrement_(height_,width_/10),
		slider_(height_*1.5,width/20),
		slider_change_callback_(nullptr),
		lower_limit_(lower_limit),
		upper_limit_(upper_limit),
		increment_fraction_(.05f),
		current_position_(lower_limit/2 + upper_limit/2),
		increment_offset_(width_*.5 + increment_.getWidth(), 0, 0),
		decrement_offset_(width_*-.5-decrement_.getWidth(), 0, 0),
		current_val_offset_(0, 2*height_,0),
		lower_lim_offset_(0, 2*height_, 0),
		upper_lim_offset_(0, 2*height_, 0),
		slider_connector_(Eigen::Vector3f(1.,0.,0.)){
		
		setTexture(&rect_tex);
		setModel(&slider_model_);

		increment_.connectTo(this, &increment_offset_);
		decrement_.connectTo(this, &decrement_offset_);
		current_value_.connectTo(this, &current_val_offset_);
		upper_limit_value_.connectTo(&increment_, &upper_lim_offset_);
		lower_limit_value_.connectTo(&decrement_, &lower_lim_offset_);
		slider_.connectTo(this, &slider_connector_);

		increment_.setCallback(incrementCallback, this);
		decrement_.setCallback(decrementCallback, this);
		slider_.setCallback(beginMouseDrag, this);

	}

	void load(GLFWwindow* window, GraphicsRaw<GameObject>& graphics_2d, GraphicsRaw<Textbox>& text_graphics) {
		graphics_2d_ = &graphics_2d;
		text_graphics_ = &text_graphics;

		graphics_2d.add(*this);

		activateMouseInput(window);
		increment_.activateMouseInput(window);
		decrement_.activateMouseInput(window);
		slider_.activateMouseInput(window);

		increment_.setLabel("+");
		decrement_.setLabel("-");

		increment_.load(window, graphics_2d, text_graphics);
		decrement_.load(window, graphics_2d, text_graphics);
		slider_.load(window, graphics_2d, text_graphics);

		//graphics_2d.add(increment_);
		//graphics_2d.add(decrement_);
		//graphics_2d.add(slider_);
		
		current_value_.text = std::to_string(current_position_);
		lower_limit_value_.text = std::to_string(lower_limit_);
		upper_limit_value_.text = std::to_string(upper_limit_);

		current_value_.box_height = height_;
		lower_limit_value_.box_height = height_;
		upper_limit_value_.box_height = height_;
		current_value_.box_width = width_/4.;
		lower_limit_value_.box_width = width_ / 4.;
		upper_limit_value_.box_width = width_ / 4.;

		current_value_.font_size = .5;// current_value_.box_width / 5;
		lower_limit_value_.font_size = .5;// lower_limit_value_.box_width / 5;
		upper_limit_value_.font_size = .5;// upper_limit_value_.box_width / 5;
		
		current_value_.hide();
		lower_limit_value_.hide();
		upper_limit_value_.hide();

		text_graphics.add(current_value_);//for some reason removing this and beginning with the menu hidden causes an error
		text_graphics.add(lower_limit_value_);
		text_graphics.add(upper_limit_value_);
	}

	void unload(GLFWwindow* window, GraphicsRaw<GameObject>& graphics_2d, GraphicsRaw<Textbox>& text_graphics) {
		graphics_2d.unload(*this);
		deactivateMouseInput(window);
		increment_.deactivateMouseInput(window);
		decrement_.deactivateMouseInput(window);
		slider_.deactivateMouseInput(window);
		increment_.unload(window, graphics_2d, text_graphics);
		decrement_.unload(window, graphics_2d, text_graphics);
		slider_.unload(window, graphics_2d, text_graphics);

		text_graphics.unload(current_value_);
		text_graphics.unload(lower_limit_value_);
		text_graphics.unload(upper_limit_value_);
	}
	
	void setCurrentValue(float new_val) {
		if (new_val <= upper_limit_ && new_val >= lower_limit_) {
			current_position_ = new_val;
			if (slider_change_callback_ != nullptr) {
				slider_change_callback_(new_val, callback_input_);
			}
		}
	}

	void update(GLFWwindow* window) override {

		slider_connector_.setState(width_ * (current_position_ - lower_limit_) / (upper_limit_ - lower_limit_) - width_ / 2);

		GameObject::update(window);
		increment_.update(window);
		decrement_.update(window);
		slider_.update(window);
		lower_limit_value_.update(window);
		upper_limit_value_.update(window);
		current_value_.update(window);
	}

	void setSliderChangeCallback(void (*slider_change_callback)(float, void*), void* callback_input) {
		slider_change_callback_ = slider_change_callback;
		callback_input_ = callback_input;
	}
};

template<std::derived_from<GameObject> T>
std::vector<GameObject*> openDebugUI(T obj, const GameObject* UI_container, Default2d& graphics_2d, TextGraphics& text_graphics) {};
template<std::derived_from<GameObject> T>
std::vector<GameObject*> closeDebugUI(T obj, const GameObject* UI_container, Default2d& graphics_2d, TextGraphics& text_graphics) {};
//virtual void openDebugUI(const GameObject* UI_container, Default2d& graphics_2d, TextGraphics& text_graphics) {};
//virtual void closeDebugUI(const GameObject* UI_container, Default2d& graphics_2d, TextGraphics& text_graphics) {};

template<class T, class Obj_T>
concept UI_Iterable = requires(T iter, Obj_T* obj) {
	{(*(iter.begin()))}->std::convertible_to<Obj_T*>;
	{obj->getName()}->std::convertible_to<std::string>;
};

/*
template<class T>
concept GameObjectIterator = requires(const T & iterator, unsigned int i) {
	//{*iterator[i]} ->  std::convertible_to<GameObject*>;
	{iterator.size()} -> std::convertible_to<size_t>;
	{iterator.begin()} -> std::convertible_to<std::iterator< std::bidirectional_iterator_tag, GameObject*>>;
};
*/

template<class obj_T>
class UIIterator : public GameObject{
	Button next_target_;
	Button prev_target_;

	const std::unordered_set<obj_T*>* iterable_;
	std::unordered_set<obj_T*>::const_iterator target_iterator_;
	TextboxObject target_name_;

	void (*callback_on_change_)(obj_T*, obj_T*, void*);//prev, next
	void* callback_input_;

	Rect2d iterator_model_;

	OffsetConnector prev_offset_;
	OffsetConnector next_offset_;
	OffsetConnector name_offset_;

	GraphicsRaw<GameObject>* graphics_2d_;
	GraphicsRaw<Textbox>* text_graphics_;

	float width_;
	float height_;

	static void nextTargetCallback(void* must_be_this) {
		UIIterator* this_ = static_cast<UIIterator*>(must_be_this);
		if (this_->iterable_ == nullptr) { return; };
		obj_T* prev_target = this_->getTarget();
		if (this_->target_iterator_ == this_->iterable_->end()) {
			this_->target_iterator_ = this_->iterable_->begin();
		} else {
			this_->target_iterator_++;
		}
		this_->callback_on_change_(prev_target, this_->getTarget(), this_->callback_input_);
		if (this_->getTarget() == nullptr) {
			this_->target_name_.text = "None";
		}
		else {
			this_->target_name_.text = this_->getTarget()->getName();
		}
		this_->text_graphics_->refresh(this_->target_name_);

	}

	static void prevTargetCallback(void* must_be_this) {
		UIIterator* this_ = static_cast<UIIterator*>(must_be_this);
		if (this_->iterable_ == nullptr) { return; };
		obj_T* prev_target = this_->getTarget();
		if (this_->target_iterator_ == this_->iterable_->begin()) {
			this_->target_iterator_ = this_->iterable_->end();
		} else {
			this_->target_iterator_--;
		}
		this_->callback_on_change_(prev_target, this_->getTarget(), this_->callback_input_);
		if (this_->getTarget() == nullptr) {
			this_->target_name_.text = "None";
		} else {
			this_->target_name_.text = this_->getTarget()->getName();
		}
		this_->text_graphics_->refresh(this_->target_name_);

	}


public:
	
	class UIIterator(float height, float width) :
		iterator_model_(height,width),
		height_(height),
		width_(width),
		next_target_(height,width/4),
		prev_target_(height,width/4),
		next_offset_(width/2+width/8,0,0),
		prev_offset_(-width/2-width/8,0,0),
		name_offset_(0,-height/2,0){

		setTexture(&border_rect_tex);
		setModel(&iterator_model_);

		prev_target_.setCallback(prevTargetCallback,this);
		next_target_.setCallback(nextTargetCallback,this);

		target_name_.box_width = width;
		target_name_.box_height = height;
		target_name_.font_size = 1;
		
		prev_target_.connectTo(this, &prev_offset_);
		next_target_.connectTo(this, &next_offset_);
		target_name_.connectTo(this, &name_offset_);

		prev_target_.setLabel("Prev");
		next_target_.setLabel("Next");

		addDependent(&prev_target_);
		addDependent(&next_target_);
		addDependent(&target_name_);
	}

	void load(GLFWwindow* window, GraphicsRaw<GameObject>& graphics_2d, GraphicsRaw<Textbox>& text_graphics) {
		graphics_2d_ = &graphics_2d;
		text_graphics_ = &text_graphics;

		graphics_2d.add(*this);
		graphics_2d.add(prev_target_);
		graphics_2d.add(next_target_);
		text_graphics.add(target_name_);

		prev_target_.activateMouseInput(window);
		next_target_.activateMouseInput(window);

		next_target_.load(window, graphics_2d, text_graphics);
		prev_target_.load(window, graphics_2d, text_graphics);

	}

	void unload(GLFWwindow* window, GraphicsRaw<GameObject>& graphics_2d, GraphicsRaw<Textbox>& text_graphics) {
		graphics_2d.unload(*this);
		graphics_2d.unload(prev_target_);
		graphics_2d.unload(next_target_);
		text_graphics.unload(target_name_);

		prev_target_.deactivateMouseInput(window);
		next_target_.deactivateMouseInput(window);

		next_target_.unload(window, graphics_2d, text_graphics);
		prev_target_.unload(window, graphics_2d, text_graphics);
	}

	obj_T* getTarget() {
		if (target_iterator_ == iterable_->end()) {
			return nullptr;
		} else {
			return *target_iterator_;
		}
	}

	void setChangeCallback(void (*callback_on_change)(obj_T*, obj_T*, void*), void* callback_input) {
		callback_on_change_ = callback_on_change;
		callback_input_ = callback_input;
	}

	void setIterable(const std::unordered_set<obj_T*>* iterable) {
		iterable_ = iterable;
		target_iterator_ = iterable_->begin();
	}


};

#endif