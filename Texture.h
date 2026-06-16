#pragma once
#ifndef PUPPET_TEXTURE
#define PUPPET_TEXTURE

#include <stdio.h>
#include <iostream>

#include <string>
#include <vector>
#include"DebugPath.hpp"
#include <unordered_map>

class Texture {
	std::string fname;
	bool loaded;
	int w_tmp,h_tmp,n_ch_tmp;

	float specular_coefficient_;
	float shininess_;

	static std::unordered_map<std::string, size_t>& getReallocationMemoryMap() {
		static std::unordered_map<std::string, size_t> reallocation_memory;
		return reallocation_memory;
	}

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

//#ifndef EVILMONSTERS_PUBLISH

		if (getReallocationMemoryMap().contains(path + fname)) {
			getReallocationMemoryMap()[path + fname] += image_data.size() * sizeof(uint8_t);
		} else {
			getReallocationMemoryMap()[path + fname] = 0;
		}
//#endif

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

	static void printReallocationReport() {
		std::cout << "\n Puppet 2 Texture reallocation memory report:\n";
		size_t total = 0;
		for (auto& f : getReallocationMemoryMap()) {
			if (f.second != 0) {
				std::cout << f.first << ": " << f.second/ 1048576 << "Mb\n";
				total += f.second;
			}
		}
		std::cout << "Total reallocated texture memory: " << total/ 1048576 <<"Mb\n";
	}
};

#endif