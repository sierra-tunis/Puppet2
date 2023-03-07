#pragma once
#include "Texture.h"

class Texture3d : public Texture {//4 for color 3 for normal
private:
	std::vector<std::array<float, 3>> normals;

};