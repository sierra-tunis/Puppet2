#pragma once

#ifndef PUPPET_CAMERA
#define PUPPET_CAMERA

#include<cmath>

#include "math_constants.hpp"
#include "GameObject.h"


using Eigen::Matrix4f;

class Camera : public GameObject {

private:
	bool screenshot_flag_;
	float near_clip_;
	float far_clip_;
	float fov_;
	Eigen::Matrix4f perspective_;

public:

	Camera() : Camera(InternalObject::no_name) {}
	Camera(std::string name) :
		GameObject(name), screenshot_flag_(false),
		near_clip_(0),
		far_clip_(0),
		fov_(0),
		perspective_(Eigen::Matrix4f::Identity()){
	}
	Camera(float near_clip, float far_clip, float fov, float pixels_width, float pixels_height, std::string name = InternalObject::no_name) :
		GameObject(name), screenshot_flag_(false),
		near_clip_(near_clip),
		far_clip_(far_clip),
		fov_(fov) {

		float S = 1. / (tan(fov_ / 2. * M_PI / 180.));
		perspective_ << S, 0, 0, 0,
			0, S* pixels_width / pixels_height, 0, 0,
			0, 0, -(far_clip_) / (far_clip_ - near_clip_), -2 * far_clip_ * near_clip_ / (far_clip_ - near_clip_),
			0, 0, -1., 0;
	}

	Camera(float near_clip, float far_clip, float fov, std::string name= InternalObject::no_name) :
		Camera(near_clip,far_clip,fov,1,1,name){}

	void onKeyPress(int key) override {
		if (key == GLFW_KEY_F1) {
			this->screenshot_flag_ = true;
		}
	}

	bool getScreenshotFlag() const {
		return this->screenshot_flag_;
	}

	void clearScreenshotFlag() {
		this->screenshot_flag_ = false;
	}

	const Eigen::Matrix4f& getPerspective() const {
		return perspective_;
	}

	const Eigen::Matrix4f getCameraMatrix() const {
		Eigen::Matrix4f camera_matrix = Eigen::Matrix4f::Identity();
		camera_matrix(seq(0, 2), seq(0, 2)) = getPosition()(seq(0, 2), seq(0, 2)).transpose();
		camera_matrix(seq(0, 2), 3) = -camera_matrix(seq(0, 2), seq(0, 2)) * getPosition()(seq(0, 2), 3);
		return camera_matrix;
	}

};

#endif // !PUPPET_CAMERA
