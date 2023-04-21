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

	void (*callback_func_)();

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
				callback_func_();
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

	void setCallback(void (*callback_func)()) {
		callback_func_ = callback_func;
	}

};

#endif