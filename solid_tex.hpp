#ifndef PUPPET_SOLID_TEXTURE
#define PUPPET_SOLID_TEXTURE

#include "Texture.h"

class SolidTexture : public Texture{
public:
	SolidTexture(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha) :
		Texture(2,2,3, { red, green, blue, alpha,
						 red, green, blue, alpha,
						 red, green, blue, alpha,
						 red, green, blue, alpha }){

	}

};


#endif