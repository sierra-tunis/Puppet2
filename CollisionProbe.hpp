#pragma once


#ifndef PUPPET_COLLISIONPROBE
#define PUPPET_COLLISIONPROBE

#include "collision.hpp"
#include "GameObject.h"

class CollisionProbe_Base {
	/*
	* ________________________
	* |_	hitbox 2	 ____|
	*   \		 _______/
	*    \______/
	*		  /\
	*		   \ dir
	*			\__
	*		   /   \_
	*		   |  O	 \ hitbox 1
	*			\__\_/
	*			    \
	*			     \	   /
	*				 \X\  /\
	*				  \\\/ dist
	*				   \ \	 \/
	*					\ \  /
	*					 \ \/
	*					  \/-----> pos
	*
	*
	*
	*/

	virtual Eigen::Vector3f getDist(Eigen::Vector3f pos, Eigen::Vector3f dir) {

	}

	virtual Eigen::Vector3f getNormal(Eigen::Vector3f pos, Eigen::Vector3f dir) {

	}
};


template<PrimaryHitbox PrimaryHitbox_T, SecondaryHitbox<PrimaryHitbox_T> SecondaryHitbox_T>
class CollisionProbe : public CollisionProbe_Base {

};

#endif