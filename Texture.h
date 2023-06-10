#pragma once
#ifndef PUPPET_TEXTURE
#define PUPPET_TEXTURE

#include <string>
#include <vector>


class Texture {
	std::string fname;
	bool loaded;
	std::vector<uint8_t> image_data;
	int w_tmp,h_tmp,n_ch_tmp;

	std::vector<uint8_t> read_img_data(std::string fname);

public:
	static constexpr char debug_path[] = "C:\\Users\\Justin\\source\\repos\\Puppet2\\Puppet2\\assets\\";
	static std::string default_path;

	const unsigned int width;
	const unsigned int height;
	const unsigned int n_channels;

	Texture(std::string fname):Texture(fname, default_path){
	}

	Texture(std::string fname, std::string path):
		image_data(read_img_data(path + fname)),
		width(w_tmp),
		height(h_tmp),
		n_channels(n_ch_tmp),
		fname(fname) {
	
	}

	std::vector<uint8_t> getData() const {
		return image_data;
	}

};

#endif