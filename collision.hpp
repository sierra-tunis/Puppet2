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

template<>
bool checkCollision<MeshSurface, MeshSurface>(const MeshSurface* PrimarySurf, const MeshSurface* SecondarySurf, const Eigen::Matrix4f PrimaryPosition, const Eigen::Matrix4f SecondaryPosition);


template<class primary>
concept PrimaryHitbox = std::derived_from<primary, Surface<3>>;


template<class secondary, class primary>
concept SecondaryHitbox = requires(const primary & first, const secondary & second, Eigen::Matrix4f G1, Eigen::Matrix4f G2) {
	checkCollision<primary, secondary>(&first, &second, G1, G2);
};




#endif