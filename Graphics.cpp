#include "Graphics.hpp"

#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

template <class T>
void write_png(std::string fname, int screenshot_width, int screenshot_height, std::vector<T>* img) {
	stbi_write_png(fname.c_str(), screenshot_width, screenshot_height, 4, img->data(), screenshot_width * 4);//4 channels
}

template
void write_png<uint8_t>(std::string fname, int screenshot_width, int screenshot_height, std::vector<uint8_t>* img);