#pragma once

#ifndef PUPPET_CAMERA
#define PUPPET_CAMERA

#define _USE_MATH_DEFINES
#include<cmath>

#include "GameObject.h"
#include "Graphics.hpp"


using Eigen::Matrix4f;

class Camera : public GameObject {

private:
	bool screenshot_flag_;
	float near_clip_;
	float far_clip_;
	float fov_;
	Eigen::Matrix4f perspective_;

public:

	Camera(std::string name) :
		GameObject(name), screenshot_flag_(false),
		near_clip_(0),
		far_clip_(0),
		fov_(0),
		perspective_(Eigen::Matrix4f::Identity()){
	}

	Camera(float near_clip, float far_clip, float fov,std::string name) : 
		GameObject(name),screenshot_flag_(false),
		near_clip_(near_clip),
		far_clip_(far_clip),
		fov_(fov){

		float S = 1. / (tan(fov_ / 2. * M_PI / 180.));
		perspective_ << S, 0, 0, 0,
			0, S, 0, 0,
			0, 0, -(far_clip_) / (far_clip_ - near_clip_), -2 * far_clip_ * near_clip_ / (far_clip_ - near_clip_),
			0, 0, -1., 0;
	}


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
