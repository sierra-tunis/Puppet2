#pragma once

#ifndef PUPPET_MATH_COLLISION
#define PUPPET_MATH_COLLISION

#include "surface.hpp"

template<class A, class B>
bool checkCollision(const A* PrimarySurf, const B* SecondarySurf, Eigen::Matrix4f PrimaryPosition, Eigen::Matrix4f SecondaryPosition, const Eigen::Matrix4f secondary_motion);

template<class A, class B>
bool checkCollision(const A* PrimarySurf, const B* SecondarySurf, Eigen::Matrix4f PrimaryPosition, Eigen::Matrix4f SecondaryPosition) {
	return checkCollision(PrimarySurf, SecondarySurf, PrimaryPosition, SecondaryPosition, Eigen::Matrix4f::Identity());
}


bool SurfaceNodeCollision(const Surface<3>* PrimarySurf, const MeshSurface* SecondarySurf, Eigen::Matrix4f SecondaryPosition);
bool SurfaceNodeCollision(const MeshSurface* PrimarySurf, const MeshSurface* SecondarySurf, Eigen::Matrix4f SecondaryPosition);

template<>
bool checkCollision<Surface<3>, MeshSurface>(const Surface<3>* PrimarySurf, const MeshSurface* SecondarySurf, const Eigen::Matrix4f PrimaryPosition, const Eigen::Matrix4f SecondaryPosition, const Eigen::Matrix4f secondary_motion);

template<>
bool checkCollision<MeshSurface, MeshSurface>(const MeshSurface* PrimarySurf, const MeshSurface* SecondarySurf, const Eigen::Matrix4f PrimaryPosition, const Eigen::Matrix4f SecondaryPosition, const Eigen::Matrix4f secondary_motion);

template<>
bool checkCollision<Ellipse, Ellipse>(const Ellipse* PrimarySurf, const Ellipse* SecondarySurf, const Eigen::Matrix4f PrimaryPosition, const Eigen::Matrix4f SecondaryPosition, const Eigen::Matrix4f secondary_motion);

//theres probably a very cool solution to mesh-mesh collision using probablistic methods
//also you could us ML to precompute a probabalistic function between two known meshes

template<class primary>
concept PrimaryHitbox = std::derived_from<primary, Surface<3>>;


template<class secondary, class primary>
concept SecondaryHitbox = requires(const primary & first, const secondary & second, Eigen::Matrix4f G1, Eigen::Matrix4f G2,Eigen::Matrix4f dG2) {
	checkCollision<primary, secondary>(&first, &second, G1, G2, dG2);
};

class CollisionPairBase {
public:
	virtual bool isCollision(Eigen::Matrix4f primary_position, Eigen::Matrix4f secondary_position,Eigen::Matrix4f secondary_motion) const = 0;
	virtual void fullCollisionInfo(Eigen::Matrix4f primary_position, Eigen::Matrix4f secondary_position,Eigen::Matrix4f secondary_motion) = 0;
};

template<PrimaryHitbox Prim_T, SecondaryHitbox<Prim_T> Sec_T>
class CollisionInfo {

};


template<>
class CollisionInfo<Surface<3>, MeshSurface> {

	struct EdgeCollisionInfo {
		bool is_colliding;
		float collision_location;
	};

	std::vector<EdgeCollisionInfo> edge_info_;//must be same length as meshbox edges

public:
	std::vector<EdgeCollisionInfo>& getEdgeInfo() {
		return edge_info_;
	}


};

template<>
class CollisionInfo<MeshSurface, MeshSurface> {

	struct EdgeCollisionInfo {
		bool is_colliding;
		float collision_location;
		std::vector<int> colliding_faces;
	};

	std::vector<EdgeCollisionInfo> edge_info_;//must be same length as meshbox edges

public:
	std::vector<EdgeCollisionInfo>& getEdgeInfo() {
		return edge_info_;
	}

	const std::vector<EdgeCollisionInfo>& getEdgeInfo_const() const {
		return edge_info_;
	}

	CollisionInfo(int n_edges=0):edge_info_(n_edges) {

	}

};

bool SurfaceNodeCollision(const Surface<3>* PrimarySurf, const MeshSurface* SecondarySurf, Eigen::Matrix4f SecondaryPosition, CollisionInfo<Surface<3>, MeshSurface>* collision_info);

template<class A, class B>
void getFullCollision(const A* PrimarySurf, const B* SecondarySurf, Eigen::Matrix4f PrimaryPosition, Eigen::Matrix4f SecondaryPosition, const Eigen::Matrix4f secondary_motion, CollisionInfo<A,B>* collision_info);


template<class A, class B>
void getFullCollision(const A* PrimarySurf, const B* SecondarySurf, Eigen::Matrix4f PrimaryPosition, Eigen::Matrix4f SecondaryPosition, CollisionInfo<A, B>* collision_info) {
	getFullCollision(PrimarySurf, PrimaryPosition, SecondarySurf, SecondaryPosition, Eigen::Matrix4f::Identity(), collision_info);
}


template<>
void getFullCollision<Surface<3>, MeshSurface>(const Surface<3>* PrimarySurf, const MeshSurface* SecondarySurf, const Eigen::Matrix4f PrimaryPosition, const Eigen::Matrix4f SecondaryPosition, const Eigen::Matrix4f secondary_motion, CollisionInfo<Surface<3>, MeshSurface>* collision_info);

template<>
void getFullCollision<MeshSurface, MeshSurface>(const MeshSurface* PrimarySurf, const MeshSurface* SecondarySurf, const Eigen::Matrix4f PrimaryPosition, const Eigen::Matrix4f SecondaryPosition, const Eigen::Matrix4f secondary_motion, CollisionInfo<MeshSurface, MeshSurface>* collision_info);


template<PrimaryHitbox PrimaryHitbox_T, SecondaryHitbox<PrimaryHitbox_T> SecondaryHitbox_T>
class CollisionPair : public CollisionPairBase {

	CollisionInfo<PrimaryHitbox_T, SecondaryHitbox_T> collision_info_;

public:
	const PrimaryHitbox_T& first;
	const SecondaryHitbox_T& second;

	bool isCollision(Eigen::Matrix4f primary_position, Eigen::Matrix4f secondary_position, Eigen::Matrix4f secondary_motion) const final override {
		return checkCollision<PrimaryHitbox_T, SecondaryHitbox_T>(&first, &second, primary_position,secondary_position,secondary_motion);
	}

	void fullCollisionInfo(Eigen::Matrix4f primary_position, Eigen::Matrix4f secondary_position, Eigen::Matrix4f secondary_motion) final override {
		return getFullCollision<PrimaryHitbox_T, SecondaryHitbox_T>(&first, &second, primary_position, secondary_position,secondary_motion, &collision_info_);

	}

	//void GetCollisionData(Eigen::Matrix4f primary_position, Eigen::Matrix4f secondary_position)

	const CollisionInfo<PrimaryHitbox_T, SecondaryHitbox_T>& getCollisionInfo() const {
		return collision_info_;
	}

	CollisionPair(const PrimaryHitbox_T& primary_hitbox, const SecondaryHitbox_T& secondary_hitbox) :
		first(primary_hitbox),
		second(secondary_hitbox){}	//std::pair<PrimaryHitbox_T, SecondaryHitbox_T> hitboxes;

};

class CollisionSetBase {

};

template<PrimaryHitbox PrimaryHitbox_T, SecondaryHitbox<PrimaryHitbox_T> SecondaryHitbox_T>
class CollisionSet : public CollisionSetBase {

};



#endif