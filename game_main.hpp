
#ifndef PUPPET_GAMEMAIN
#define PUPPET_GAMEMAIN

#include "level.h"
#include "graphics_base.hpp"
#include "GameObject.h"

class Game {
	std::vector<Level*>& levels_;
	std::vector<GraphicsBase*> graphics_;
	GameObject& player;

};

Game& GameMain();

#endif