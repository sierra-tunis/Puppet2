#pragma once
#ifndef PUPPET_TEXTURE
#define PUPPET_TEXTURE

#include <string>
#include <vector>


#define TEXTURE_PATH "C:\\Users\\Justin\\source\\repos\\Puppet2\\Puppet2\\assets\\"

class Texture {
	std::string fname;
	bool loaded;
	std::vector<uint8_t> image_data;
	int w_tmp,h_tmp,n_ch_tmp;

	std::vector<uint8_t> read_img_data(std::string fname);

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