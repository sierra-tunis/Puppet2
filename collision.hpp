#pragma once

#ifndef PUPPET_MATH_COLLISION
#define PUPPET_MATH_COLLISION

#include "surface.hpp"

template<class A, class B>
bool checkCollision(const A* PrimarySurf, const B* SecondarySurf, Eigen::Matrix4f PrimaryPosition, Eigen::Matrix4f SecondaryPosition);

bool SurfaceNodeCollision(const Surface<3>* PrimarySurf, const MeshSurface* SecondarySurf, Eigen::Matrix4f SecondaryPosition);
bool SurfaceNodeCollision(const Surface<3>* PrimarySurf, const MeshSurface* SecondarySurf, Eigen::Matrix4f SecondaryPosition, std::vector<bool>* collision_info);

template<>
bool checkCollision<Surface<3>, MeshSurface>(const Surface<3>* PrimarySurf, const MeshSurface* SecondarySurf, const Eigen::Matrix4f PrimaryPosition, const Eigen::Matrix4f SecondaryPosition);

template<>
bool checkCollision<Ellipse, Ellipse>(const Ellipse* PrimarySurf, const Ellipse* SecondarySurf, const Eigen::Matrix4f PrimaryPosition, const Eigen::Matrix4f SecondaryPosition);

//theres probably a very cool solution to mesh-mesh collision using probablistic methods
//also you could us ML to precompute a probabalistic function between two known meshes
template<>
bool checkCollision<MeshSurface, MeshSurface>(const MeshSurface* PrimarySurf, const MeshSurface* SecondarySurf, const Eigen::Matrix4f PrimaryPosition, const Eigen::Matrix4f SecondaryPosition);


template<class primary>
concept PrimaryHitbox = std::derived_from<primary, Surface<3>>;


template<class secondary, class primary>
concept SecondaryHitbox = requires(const primary & first, const secondary & second, Eigen::Matrix4f G1, Eigen::Matrix4f G2) {
	checkCollision<primary, secondary>(&first, &second, G1, G2);
};

class CollisionPairBase {
public:
	virtual bool isCollision(Eigen::Matrix4f primary_position, Eigen::Matrix4f secondary_position) const = 0;

};

template<PrimaryHitbox PrimaryHitbox_T, SecondaryHitbox<PrimaryHitbox_T> SecondaryHitbox_T>
class CollisionPair : public CollisionPairBase {

public:
	const PrimaryHitbox_T& first;
	const SecondaryHitbox_T& second;

	bool isCollision(Eigen::Matrix4f primary_position, Eigen::Matrix4f secondary_position) const final override {
		return checkCollision<PrimaryHitbox_T, SecondaryHitbox_T>(&first, &second, primary_position,secondary_position);
	}

	CollisionPair(const PrimaryHitbox_T& primary_hitbox, const SecondaryHitbox_T& secondary_hitbox) :
		first(primary_hitbox),
		second(secondary_hitbox) {}	//std::pair<PrimaryHitbox_T, SecondaryHitbox_T> hitboxes;

};




#endif