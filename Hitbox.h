#pragma once
#include <array>
#include <Eigen/Dense>
#include <initializer_list>

#ifndef PUPPET_HITBOX
#define PUPPET_HITBOX

class Hitbox {
	Eigen::Vector3f shape;
	
public:
	Hitbox(Eigen::Vector3f shape):shape(shape){}

	Hitbox() :shape({ 0,0,0 }){}

	const Eigen::Vector3f& getShape() const {
		return shape;
	}

	bool checkCollision(const Hitbox& other,Eigen::Matrix4f position1,Eigen::Matrix4f position2) const {
		return false;
	}


};

#endif