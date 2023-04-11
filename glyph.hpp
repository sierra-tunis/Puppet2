#pragma once

#ifndef PUPPET_TEXTUREGLYPH
#define PUPPET_TEXTUREGLYPH

#include <unordered_map>

#include "Texture.h"

class Glyph : public Texture {

	struct glyphbox {

	};

	const std::unordered_map<char, glyphbox> letter_map_;


};

#endif