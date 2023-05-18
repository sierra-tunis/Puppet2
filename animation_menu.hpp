#pragma once

#ifndef PUPPET_ANIMATIONMENU
#define PUPPET_ANIMATIONMENU

#include "GameObject.h"
#include "UI.h"
#include "animation.hpp"

class AnimationMenu : public GameObject{

	GameObject* target_;


	Button new_animation_;
	Button edit_animation_;
	Button play_animation_;
	Button next_animation_;
	Button prev_animation_;
	Button save_animation_;
	Button save_animation_as_;
	TextboxObject animation_name_;

	Button copy_frame_;
	Button paste_frame_;
	Button save_frame_;
	Button edit_frame_;
	Button next_frame_;
	Button prev_frame_;


};

#endif // !PUPPET_ANIMATIONMENU
