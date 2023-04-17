#pragma once
#ifndef PUPPET_TEXTURE
#define PUPPET_TEXTURE

#include <string>
#include <vector>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


#define TEXTURE_PATH "C:\\Users\\Justin\\source\\repos\\Puppet2\\Puppet2\\assets\\"

class Texture {
	std::string fname;
	bool loaded;
	std::vector<uint8_t> image_data;
	int w_tmp,h_tmp,n_ch_tmp;

	std::vector<uint8_t> read_img_data(std::string fname) {
		uint8_t* data = stbi_load(fname.c_str(), &(this->w_tmp), &(this->h_tmp), &(this->n_ch_tmp),0);
		return std::vector<uint8_t>(data, &(data[w_tmp * h_tmp * n_ch_tmp]));
	}

public:
	const unsigned int width;
	const unsigned int height;
	const unsigned int n_channels;
	Texture(std::string fname):
		image_data(read_img_data(TEXTURE_PATH + fname)),
		width(w_tmp),
		height(h_tmp),
		n_channels(n_ch_tmp),
		fname(fname) {
	
	}

	std::vector<uint8_t> getData() const {
		return image_data;
	}

};

#undef TEXTURE_PATH

#endif