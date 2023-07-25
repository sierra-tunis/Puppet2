#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
std::string Texture::default_path = Texture::debug_path;

std::vector<uint8_t> Texture::read_img_data(std::string fname) {
	stbi_set_flip_vertically_on_load(true);
	uint8_t* data;
	if (fname[fname.size() - 3] == 'p') {//png
		data = stbi_load(fname.c_str(), &(this->w_tmp), &(this->h_tmp), &(this->n_ch_tmp), STBI_rgb_alpha);
	} else {
		data = stbi_load(fname.c_str(), &(this->w_tmp), &(this->h_tmp), &(this->n_ch_tmp), STBI_rgb);
	}
	return std::vector<uint8_t>(data, &(data[w_tmp * h_tmp * n_ch_tmp]));
}