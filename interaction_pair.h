#pragma once

#ifndef PUPPET_INTERACTIONPAIR
#define PUPPET_INTERACTIONPAIR

#include "GameObject.h"

class InteractionPair_Base {

	

};

template<class PrimaryHitbox_T,class SecondaryHitbox_T>
class CollisionPair : public InteractionPair_Base {

	GameObject primary_object_;
	PrimaryHitbox_T* primary_hitbox_;


};


#endif // !PUPPET_INTERACTIONPAIR
