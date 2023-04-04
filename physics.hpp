#pragma once

#ifndef PUPPET_PHYSICS
#define PUPPET_PHYSICS

#include<array>
#include<Eigen/dense>

class Physics {
	const int nth_order;
public:
	virtual std::vector<Eigen::Matrix4f> getNext(const std::vector<Eigen::Matrix4f>& derivatives, float dt) const {
		return derivatives[0];
	};

};

class Gravity : Physics{
	Eigen::Vector3f acceleration_;
public:
	std::vector<Eigen::Matrix4f> getNext(const std::vector<Eigen::Matrix4f>& derivatives, float dt) const override {
		Eigen::Matrix4f next = *derivatives[0];

	}
};

#endif