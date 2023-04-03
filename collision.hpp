#pragma once

#ifndef PUPPET_MATH_COLLISION
#define PUPPET_MATH_COLLISION

#include "surface.hpp"

template<class A, class B>
bool checkCollision(const A* PrimarySurf, const B* SecondarySurf, Eigen::Matrix4f PrimaryPosition, Eigen::Matrix4f SecondaryPosition) {
	//static_assert(false, "Not A Function");
	std::cerr << "cannot call generic collision function!" << "\n";
	return true;
};

bool SurfaceNodeCollision(const Surface<3>* PrimarySurf, const MeshSurface* SecondarySurf, Eigen::Matrix4f SecondaryPosition) {
	Eigen::Matrix3f R = SecondaryPosition(seq(0, 2), seq(0, 2));
	Eigen::Vector3f p = SecondaryPosition(seq(0, 2), 3);
	for (auto& e : SecondarySurf->getEdges()) {
		//if (PrimarySurf->crossesSurface(R*SecondarySurf->getVerts()[e.first]+p, R*SecondarySurf->getVerts()[e.second]+p)) {
		//should at least let the user choose is the mesh is irrotational?
		if (PrimarySurf->crossesSurface(SecondarySurf->getVerts()[e.first] + p, SecondarySurf->getVerts()[e.second] + p)) {
			return true;
		}
	}
	return false;
}

bool SurfaceNodeCollision(const Surface<3>* PrimarySurf, const MeshSurface* SecondarySurf, Eigen::Matrix4f SecondaryPosition, std::vector<bool>* collision_info) {
	Eigen::Matrix3f R = SecondaryPosition(seq(0, 2), seq(0, 2));
	Eigen::Vector3f p = SecondaryPosition(seq(0, 2), 3);
	bool result = false;
	int i = 0;
	for (auto& e : SecondarySurf->getEdges()) {
		//if (PrimarySurf->crossesSurface(R*SecondarySurf->getVerts()[e.first]+p, R*SecondarySurf->getVerts()[e.second]+p)) {
		//should at least let the user choose is the mesh is irrotational?
		(*collision_info)[i] = (PrimarySurf->crossesSurface(SecondarySurf->getVerts()[e.first] + p, SecondarySurf->getVerts()[e.second] + p));
		if (result == false && collision_info->back()==true) {
			result = true;
		}
		i++;
	}
	return result;
}

template<>
bool checkCollision<Surface<3>, MeshSurface>(const Surface<3>* PrimarySurf, const MeshSurface* SecondarySurf, const Eigen::Matrix4f PrimaryPosition, const Eigen::Matrix4f SecondaryPosition) {
	return SurfaceNodeCollision(PrimarySurf, SecondarySurf, PrimaryPosition.inverse() * SecondaryPosition);
}

template<>
bool checkCollision<Ellipse, Ellipse>(const Ellipse* PrimarySurf, const Ellipse* SecondarySurf, const Eigen::Matrix4f PrimaryPosition, const Eigen::Matrix4f SecondaryPosition) {
	return true;
}

template<class primary>
concept PrimaryHitbox = std::derived_from<primary, Surface<3>>;


template<class secondary, class primary>
concept SecondaryHitbox = requires(const primary & first, const secondary & second, Eigen::Matrix4f G1, Eigen::Matrix4f G2) {
	checkCollision<primary, secondary>(&first, &second, G1, G2);
};




#endif