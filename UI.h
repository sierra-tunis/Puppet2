#pragma once

#ifndef PUPPET_UI
#define PUPPET_UI

#include "Model.h"
#include "GameObject.h"
#include "Default2d.hpp"
#include "text_graphics.hpp"
#include "textbox_object.hpp"
#include "solid_tex.hpp"
#include "math_constants.hpp"

class Rect2d : public Model {
private:

	constexpr static std::vector<float> RectVerts(float height, float width) {
		return std::vector<float>{-width/2,-height/2,0,width/2,-height/2,0,-width/2,height/2,0,width/2,height/2,0 };
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

	static Texture rect_tex;
	static Texture rect_tex_depressed;
	static Texture border_rect_tex;

	Rect2d(float height, float width):
	Model(RectVerts(height,width),RectNorms(),RectTex(1.,1.,0.,0.),RectFace(),RectFaceNorm(),RectFaceTex()){}
};


class UIElement : public GameObject {
	float height_, width_;
	Rect2d model_;

	TextboxObject label_;

	virtual void load(GLFWwindow* window, GraphicsRaw<GameObject>& graphics_2d, GraphicsRaw<Textbox>& text_graphics) = 0;
	virtual void unload(GLFWwindow* window, GraphicsRaw<GameObject>& graphics_2d, GraphicsRaw<Textbox>& text_graphics) = 0;

};


class Button : public GameObject{

	const float height_, width_;
//	float x_, y_; //center coord
	Rect2d button_model_;
	GraphicsRaw<Textbox>* text_graphics_;
	GraphicsRaw<GameObject>* graphics_2d_;


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
		text_graphics_(nullptr),
		graphics_2d_(nullptr){

		setModel(&button_model_);
		setTexture(&Rect2d::rect_tex);
		label_.box_width = width_*.9;
		addDependent(&label_);
		label_.connectToParent(new OffsetConnector(0, 0, 0));

	}

	Button(float height, float width, std::string name) : 
		GameObject(name),
		height_(height),
		width_(width),
		button_model_(height_,width_),
		text_graphics_(nullptr){

		setModel(&button_model_);
		setTexture(&Rect2d::rect_tex);
		label_.box_width = width_*.9;
		addDependent(&label_);
		label_.connectToParent(new OffsetConnector(0, 0, 0));//idk why this needs to be height/2 instead of 0

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
		graphics_2d_ = &graphics_2d;
	}

	void unload(GLFWwindow* window, GraphicsRaw<GameObject>& graphics_2d, GraphicsRaw<Textbox>& text_graphics) {
		graphics_2d.unload(*this);
		text_graphics.unload(this->label_);
		deactivateMouseInput(window);
		text_graphics_ = nullptr;
	}

	void depress() {
		setTexture(&Rect2d::rect_tex_depressed);
		if (graphics_2d_ != nullptr) {
			this->redraw(graphics_2d_);
		}
	}
	void undepress() {
		setTexture(&Rect2d::rect_tex);
		if (graphics_2d_ != nullptr) {
			this->redraw(graphics_2d_);
		}
	}
	
	void virtualClick() {
		callback_func_(callback_input_);
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
		
		setTexture(&Rect2d::rect_tex);
		setModel(&slider_model_);

		addDependent(&increment_);
		addDependent(&decrement_);
		addDependent(&current_value_);
		addDependent(&slider_);
		increment_.addDependent(&upper_limit_value_);
		decrement_.addDependent(&lower_limit_value_);

		increment_.connectToParent(&increment_offset_);
		decrement_.connectToParent(&decrement_offset_);
		current_value_.connectToParent(&current_val_offset_);
		upper_limit_value_.connectToParent(&upper_lim_offset_);
		lower_limit_value_.connectToParent(&lower_lim_offset_);
		slider_.connectToParent(&slider_connector_);

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

	void onStep() override {

		slider_connector_.setState(width_ * (current_position_ - lower_limit_) / (upper_limit_ - lower_limit_) - width_ / 2);
	}

	void setSliderChangeCallback(void (*slider_change_callback)(float, void*), void* callback_input) {
		slider_change_callback_ = slider_change_callback;
		callback_input_ = callback_input;
	}
};
/*
template<std::derived_from<GameObject> T>
std::vector<GameObject*> openDebugUI(T obj, const GameObject* UI_container, Default2d& graphics_2d, TextGraphics& text_graphics) {};
template<std::derived_from<GameObject> T>
std::vector<GameObject*> closeDebugUI(T obj, const GameObject* UI_container, Default2d& graphics_2d, TextGraphics& text_graphics) {};
//virtual void openDebugUI(const GameObject* UI_container, Default2d& graphics_2d, TextGraphics& text_graphics) {};
//virtual void closeDebugUI(const GameObject* UI_container, Default2d& graphics_2d, TextGraphics& text_graphics) {};
*/
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
		if (this_->callback_on_change_ != nullptr) {
			this_->callback_on_change_(prev_target, this_->getTarget(), this_->callback_input_);
		}
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
		if (this_->callback_on_change_ != nullptr) {
			this_->callback_on_change_(prev_target, this_->getTarget(), this_->callback_input_);
		}
		if (this_->getTarget() == nullptr) {
			this_->target_name_.text = "None";
		} else {
			this_->target_name_.text = this_->getTarget()->getName();
		}
		this_->text_graphics_->refresh(this_->target_name_);

	}


public:
	
	UIIterator(float height, float width) :
		iterable_(nullptr),
		iterator_model_(height,width),
		height_(height),
		width_(width),
		next_target_(height,width/4),
		prev_target_(height,width/4),
		next_offset_(width/2+width/8,0,0),
		prev_offset_(-width/2-width/8,0,0),
		name_offset_(0,-height/2,0){

		setTexture(&Rect2d::border_rect_tex);
		setModel(&iterator_model_);

		prev_target_.setCallback(prevTargetCallback,this);
		next_target_.setCallback(nextTargetCallback,this);

		target_name_.box_width = width;
		target_name_.box_height = height;
		target_name_.font_size = 1;
		

		addDependent(&prev_target_);
		addDependent(&next_target_);
		addDependent(&target_name_);

		prev_target_.connectToParent(&prev_offset_);
		next_target_.connectToParent(&next_offset_);
		target_name_.connectToParent(&name_offset_);

		prev_target_.setLabel("Prev");
		next_target_.setLabel("Next");

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

	obj_T* getTarget() const {
		if (iterable_ == nullptr || target_iterator_ == iterable_->end()) {
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
		obj_T* prev_target = getTarget();
		iterable_ = iterable;
		target_iterator_ = iterable_->begin();
		if (callback_on_change_ != nullptr) {
			callback_on_change_(prev_target, getTarget(), callback_input_);
		}
	}

	void goToNext() {
		nextTargetCallback(this);
	}
	void goToPrev() {
		prevTargetCallback(this);
	}

};

class Pane : public GameObject {

	float height_, width_;

};

class TabbedPane : public GameObject {
	
	float height_, width_;

	std::vector<Pane*> panes_;
	std::vector<Button*> tabs_;
	float total_tab_width_;
	float tab_height_;
	static constexpr int max_tabs = 10;

	int active_pane_index_;

	//whether this function is private or public says a lot about the architecture of puppet code pachake/puppet software
	void activatePane(int i) {
		panes_[active_pane_index_]->hide();
		tabs_[active_pane_index_]->undepress();
		panes_[i]->show();
		tabs_[i]->depress();
		active_pane_index_ = i;

	}

	template<int i>
	static void activatePaneCallback(void* must_be_this) {
		TabbedPane* this_ = static_cast<TabbedPane*>(must_be_this);
		this_->activatePane(i);
	}

	template<int index = max_tabs>
	void setButtonCallbacks() {
		if (index < tabs_.size()) {
			tabs_[index]->setCallback(&activatePaneCallback<index>, this);
		}
		if constexpr (index > 0) {
			setButtonCallbacks<index - 1>();
		}
	}

public:
	TabbedPane(float height, float width, float tab_height) :
		height_(height),
		width_(width),
		total_tab_width_(0.0f),
		tab_height_(.1),
		active_pane_index_(0){

	}

	void addPane(Pane* new_pane, std::string pane_label, float tab_width) {
		panes_.push_back(new_pane);
		addDependent(new_pane);

		tabs_.emplace_back(new Button(tab_height_, tab_width));	
		setButtonCallbacks();
		addDependent(tabs_.back());
		tabs_.back()->moveTo(-width_/2 + total_tab_width_+tab_width/2, height_ / 2 + tab_height_ / 2, 0);
		tabs_.back()->clampToParent();
		tabs_.back()->setLabel(pane_label);
		total_tab_width_ += tab_width;
		activatePane(tabs_.size()-1);
		new_pane->clampToParent();
	}

	Pane* getPane(int i) {
		return panes_[i];
	}

	void load(GLFWwindow* window, GraphicsRaw<GameObject>& graphics_2d, GraphicsRaw<Textbox>& text_graphics) {
		for (auto& tab : tabs_) {
			graphics_2d.add(*tab);
			tab->load(window, graphics_2d, text_graphics);
		}
		activatePane(0);
	}

	void unload(GLFWwindow* window, GraphicsRaw<GameObject>& graphics_2d, GraphicsRaw<Textbox>& text_graphics) {
		for (auto& tab : tabs_) {
			graphics_2d.unload(*tab);
			tab->unload(window, graphics_2d, text_graphics);
		}
	}

	void next() {
		activatePane((active_pane_index_ + 1) % panes_.size());
	}

	void prev() {
		activatePane((panes_.size() + active_pane_index_ - 1) % panes_.size());
	}

	int getActivePaneIndex() const {
		return active_pane_index_;
	}

};

class PositiveRect2d : public Model {
private:

	constexpr static std::vector<float> RectVerts(float height, float width) {
		return std::vector<float>{0.,0., 0, width, 0, 0, 0, height, 0, width, height, 0 };
	}
	constexpr static std::vector<float> RectNorms() {
		return std::vector<float>{0., 0., -1., 0, 0, -1, 0, 0, -1, 0, 0, -1};
	}
	constexpr static std::vector<float> RectTex(float height, float width, float top, float left) {
		//(height ,width, 0,0) is top left
		return std::vector<float>{left, top, left + width, top, left, top + height, left + width, top + height};
	}
	constexpr static std::vector<unsigned int> RectFace() {
		return std::vector<unsigned int>{0, 1, 2, 3};
	}
	constexpr static std::vector<unsigned int> RectFaceNorm() {
		return std::vector<unsigned int>{0, 1, 2, 3};
	}
	constexpr static std::vector<unsigned int> RectFaceTex() {
		return std::vector<unsigned int>{0, 1, 2, 3};
	}


public:

	static Texture rect_tex;
	static Texture border_rect_tex;

	PositiveRect2d(float height, float width) :
		Model(RectVerts(height, width), RectNorms(), RectTex(1., 1., 0., 0.), RectFace(), RectFaceNorm(), RectFaceTex()) {}
};

class ProgressBar : public GameObject {

	GraphicsRaw<GameObject>* graphics_2d_;
	
	PositiveRect2d bar_;
	Eigen::Vector4f color_;
	const float* read_float_;
	float builtin_float_;
	float bar_max_;
	float bar_min_;

	void onStep() override {
		//bar_.rescale((*read_float_ - bar_min_) / (bar_max_ - bar_min_),1.0, 1.0);
		//graphics_2d_->refresh(*this);
	}

public:
	void readFrom(const float& signal) {
		read_float_ = &signal;
	}


	ProgressBar(float height, float width, Eigen::Vector4f color, GraphicsRaw<GameObject>* graphics_2d) :
		read_float_(&builtin_float_),
		bar_max_(1.0),
		bar_min_(0.0),
		builtin_float_(1.0),
		bar_(height, width) {

		setModel(&bar_);
		setTexture(new SolidTexture(color(0), color(1), color(2), color(3)));

		draw(graphics_2d);
		graphics_2d_ = graphics_2d;

	}
};

template<GameObject_T obj_T>
class UIWheel : public GameObject {

	Eigen::Vector2f cursor_;
	int n_sections_;

	const std::vector<obj_T*>* iterable_;
	obj_T* target_;
	TextboxObject target_name_;

	Rect2d iterator_model_;
	static Texture wheel_tex_;
	static Texture section_textures_[8]; //i.e. texture for 0 chunks, 1 chunk, 2 chunks etc
	static Texture highlighted_section_textures_[8];

	std::vector<GameObject*> sections_;
	std::vector<OffsetConnector*> section_offsets_;
	OffsetConnector name_offset_;

	GraphicsRaw<GameObject>* graphics_2d_;
	GraphicsRaw<Textbox>* text_graphics_;

	float radius_;

	GLFWwindow* window_;

	void onStep() override {
		if (!isHidden() && window_ != nullptr) {
			Eigen::Vector2f stick_input = Eigen::Vector2f(InternalObject::getRightStickPosition(window_).first, InternalObject::getRightStickPosition(window_).second);
			if (stick_input.norm() > .3) {
				cursor_ = stick_input.normalized();
			}
			if (getTarget() == nullptr) {
				target_name_.text = "None";
			} else {
				target_name_.text = getTarget()->getName();
			}
			//setOrientation(Eigen::Matrix3f::Identity());
			//rotateZ(getCursorAngle());
			text_graphics_->refresh(target_name_);
			//change approprate section texture and redraw 
		}
	}

public:

	UIWheel(float radius) :
		iterable_(nullptr),
		iterator_model_(radius, radius),
		name_offset_(0, -radius / 2, 0) {

		setTexture(&Rect2d::border_rect_tex);
		setModel(&iterator_model_);

		target_name_.box_width = radius;
		target_name_.box_height = radius;
		target_name_.font_size = 1;

		addDependent(&target_name_);

		target_name_.connectToParent(&name_offset_);

	}

	void load(GLFWwindow* window, GraphicsRaw<GameObject>& graphics_2d, GraphicsRaw<Textbox>& text_graphics) {
		graphics_2d_ = &graphics_2d;
		text_graphics_ = &text_graphics;
		window_ = window;

		graphics_2d.add(*this);
		text_graphics.add(target_name_);

		activateControllerInput(window);
		activateMouseInput(window);

	}

	void unload(GLFWwindow* window, GraphicsRaw<GameObject>& graphics_2d, GraphicsRaw<Textbox>& text_graphics) {
		graphics_2d.unload(*this);
		text_graphics.unload(target_name_);
		
		deactivateControllerInput(window);
		deactivateMouseInput(window);
	}

	float getCursorAngle() const {
		return fmod(atan2(-cursor_.x(), cursor_.y()) + M_PI, 2 * M_PI);
	}

	int getSection() const {
		//indexes go counter clockwise
		float cursor_angle = getCursorAngle();
		float section_width_radians = 2.0 * M_PI / n_sections_;
		return ((int)round(cursor_angle/section_width_radians))%n_sections_;
	}

	obj_T* getTarget() const {
		if (iterable_ == nullptr) {
			return nullptr;
		}
		else {
			int section = getSection();
			if (iterable_->size() <= section) {
				return nullptr;
			}
			else {
				return (*iterable_)[section];
			}
		}
	}

	void setIterable(const std::vector<obj_T*>* iterable) {
		obj_T* prev_target = getTarget();
		iterable_ = iterable;
		n_sections_ = iterable_->size();
		//do draw calls based on iterable_.size();
	}

};

#endif