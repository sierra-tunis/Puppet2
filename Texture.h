#pragma once
#ifndef PUPPET_TEXTURE
#define PUPPET_TEXTURE

#include <string>
#include <vector>
#include"DebugPath.hpp"

class Texture {
	std::string fname;
	bool loaded;
	int w_tmp,h_tmp,n_ch_tmp;

	float specular_coefficient_;
	float shininess_;

protected:
	std::vector<uint8_t> read_img_data(std::string fname);

	std::vector<uint8_t> image_data;


public:
	static constexpr char debug_path[] = DEBUG_PATH;
	static std::string default_path;

	const unsigned int width;
	const unsigned int height;
	const unsigned int n_channels;

	//Texture() :Texture(0, 0, 0, std::vector<uint8_t>()) {}

	Texture(std::string fname):Texture(fname, default_path){
	}

	Texture(std::string fname, std::string path):
		image_data(read_img_data(path + fname)),
		width(w_tmp),
		height(h_tmp),
		n_channels(n_ch_tmp),
		fname(fname),
		specular_coefficient_(0.0f),
		shininess_(1.0f) {
	
	}

	Texture(int height, int width, int n_channels, std::vector<uint8_t> img_data) :
		image_data(img_data),
		width(width),
		height(height),
		n_channels(n_channels),
		fname(""),
		specular_coefficient_(0.0f),
		shininess_(1.0f){

	}


	std::vector<uint8_t> getData() const {
		return image_data;
	}

	void setSpecularCoefficient(float k_s) {
		specular_coefficient_ = k_s;
	}
	float getSpecularCoefficient() const {
		return specular_coefficient_;
	}
	void setShininess(float shininess) {
		shininess_ = shininess;
	}
	float getShininess() const {
		return shininess_;
	}
};

#endif