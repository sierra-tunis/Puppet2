#pragma once

#ifndef PUPPET_GRAPHICSTEXT
#define PUPPET_GRAPHICSTEXT

#include <string>

#include "text.hpp"
#include "Graphics.hpp"


//e.g. of how to implement tediously large number of cached argumend

struct char_info {
	float unscaled_height, unscaled_width;
	float glyph_top, glyph_left;

	char_info(char c) {
		int c_ = (int)c;
		if (c_ < 0) {
			c_ += 256;
		}
		int row = c_ / 25;
		int col = c_ % 25;
		unscaled_height = 20. / 312.;
		unscaled_width = 12./512.;
		glyph_left = static_cast<float>(col)*20./512.;
		glyph_top = static_cast<float>(row)*20./312.;
	}

	char_info() {}

};


class Font {

	Texture glyph;
	int tex_id_;
	float unscaled_line_height_;
	std::unordered_map<char, char_info> char_info_bank;
	//should be const, but initialization is awkward so left non-const for now

public:
	int getTexID() const {
		return tex_id_;
	}

	char_info getCharInfo(const char& c) const {
		return char_info_bank.at(c);
	}

	float getUnscaledLineHeight() const {
		return unscaled_line_height_;
	}


	Font(std::string glyph_fname, std::string path):
		glyph(glyph_fname, path),unscaled_line_height_(1.f/15.f) {
		for (int i = 0; i < 256; i++) {
			char_info_bank[static_cast<char>(i)] = char_info(static_cast<char>(i));
			switch (i) {
				case 'A': char_info_bank[static_cast<char>(i)].unscaled_width = 12. / 512.; break;
			}
		}
		unsigned int tex_id;
		glGenTextures(1, &(tex_id));
		glBindTexture(GL_TEXTURE_2D, tex_id);
		//this->tex_id = static_cast<int>(tex_id);
		tex_id_ = static_cast<int>(tex_id);


		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);


		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, glyph.width, glyph.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, glyph.getData().data());
		glGenerateMipmap(GL_TEXTURE_2D);

	}
	
	Font(std::string glyph_fname) : Font(glyph_fname, Texture::default_path) {}

};

struct TextGraphicsCache {
	unsigned int keyboard_VAO;
	unsigned int ps_VAO;
	unsigned int VBO[4];
	unsigned int EBO[2];
	constexpr static size_t VBO_size = 4;
	constexpr static size_t EBO_size = 2;
	unsigned int tex_id;
	size_t n_elems;

	TextGraphicsCache(unsigned int keyboard_VAO, unsigned int ps_VAO, unsigned int* VBO, unsigned int* EBO, unsigned int tex_id, size_t n_elems):
		keyboard_VAO(keyboard_VAO), ps_VAO(ps_VAO), VBO{ VBO[0],VBO[1],VBO[2],VBO[3] }, EBO{ EBO[0],EBO[1] },tex_id(tex_id),n_elems(n_elems) {

	}
};

class TextGraphics : public Graphics<Textbox, TextGraphicsCache> {
								//textbox, VAO, tex_id, n_elems
	std::unordered_map<std::string, const Font*> named_fonts_;
	Font& default_font_;
	unsigned int position_location_;

	Textbox::Language display_language_;

	Cache makeDataCache(const Textbox& obj) const override {
		const Font* font;
		if (named_fonts_.contains(obj.font)) {
			font = named_fonts_.at(obj.font);
		}
		else {
			font = &default_font_;
		}

		//super lazy code...
		 


		const Model& PS_model = *makeTextboxModel(obj, *font,false,display_language_);
		const Model& keyboard_model = *makeTextboxModel(obj, *font,true, display_language_);

		// = model.flen();
		unsigned int VAO[2];
		glGenVertexArrays(2, VAO);
		unsigned int VBO[4];
		glGenBuffers(4, VBO);
		unsigned int EBO[2];
		glGenBuffers(2, EBO);

		glBindVertexArray(VAO[0]);

		glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * keyboard_model.vlen() * 3, keyboard_model.getVerts().data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * keyboard_model.getTexCoords().size(), keyboard_model.getTexCoords().data(), GL_STATIC_DRAW);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[0]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * keyboard_model.flen() * 3, keyboard_model.getFaces().data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		
		glBindVertexArray(VAO[1]);

		glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * PS_model.vlen() * 3, PS_model.getVerts().data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, VBO[3]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * PS_model.getTexCoords().size(), PS_model.getTexCoords().data(), GL_STATIC_DRAW);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[1]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * PS_model.flen() * 3, PS_model.getFaces().data(), GL_STATIC_DRAW);
		

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		/*GLenum error = glGetError();
		if (error != GL_NO_ERROR) {
			printf("Error during VAO creation: 0x%x\n", error);
		}*/

		//texture code:
		unsigned int glyph_tex_id = font->getTexID();
		int n_elems = PS_model.flen();
		delete &keyboard_model;
		delete &PS_model;
		return Cache{ TextGraphicsCache(VAO[0],VAO[1],VBO,EBO, glyph_tex_id,  n_elems)};
	};

	void deleteDataCache(Cache& cache) const override {
			glDeleteVertexArrays(1, &getKeyboardVAO(cache));
			glDeleteVertexArrays(1, &getPSVAO(cache));
			glDeleteBuffers(TextGraphicsCache::VBO_size, getVBO(cache));
			glDeleteBuffers(TextGraphicsCache::EBO_size, getEBO(cache));

	};


	const unsigned int& getKeyboardVAO(const Cache& cache) const {
		return std::get<0>(cache).keyboard_VAO;
	}

	const unsigned int& getPSVAO(const Cache& cache) const {
		return std::get<0>(cache).ps_VAO;
	}
	const unsigned int* getVBO(const Cache& cache) const {
		return std::get<0>(cache).VBO;
	}
	const unsigned int* getEBO(const Cache& cache) const {
		return std::get<0>(cache).EBO;
	}

	const unsigned int& getTexID(const Cache& cache) const {
		return std::get<0>(cache).tex_id;
	}

	const size_t& getNElems(const Cache& cache) const {
		return std::get<0>(cache).n_elems;
	}

	void drawObj(const Textbox& obj, const Cache& cache) const override {
		glBindTexture(GL_TEXTURE_2D, getTexID(cache));
		if (InternalObject::isControllerConnected()) {
			glBindVertexArray(getPSVAO(cache));
		} else {
			glBindVertexArray(getKeyboardVAO(cache));
		}

		Eigen::Matrix4f position_centered = obj.getPosition();
		position_centered(0, 3) -= obj.box_width / 2;
		position_centered(1, 3) += obj.box_height / 2;
		glUniformMatrix4fv(position_location_, 1, GL_FALSE, position_centered.data());

		glDrawElements(GL_TRIANGLES, 3 * getNElems(cache), GL_UNSIGNED_INT, 0);
	}


	void beginDraw() const override {
		glDisable(GL_DEPTH_TEST);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		//default3d specific code
	}

	void endDraw() const override {
		//default3d specific code
	}

public:
	void addFont(std::string name, const Font& font) {
		named_fonts_[name] = &font;
	}
	
	void setLanguage(Textbox::Language language) {
		display_language_ = language;
		refreshAll();
	}

	TextGraphics(Font& default_font):
		display_language_(Textbox::Language::English),
		default_font_(default_font),
		position_location_(glGetUniformLocation(gl_id, "position_matrix")){
	}

	//renamed from get to make since it is allocating memory
//nodiscard since memory needs to be deleted later. this should be a smart pointer eventually
	[[nodiscard]] static Model* makeTextboxModel(const Textbox& textbox, const Font& font, bool convert_PS_to_keyboard, Textbox::Language language) {
		float line_length = 0;
		int line_num = 0;
		Model* textbox_model = new Model(std::vector<float>{},
			std::vector<float>{},
			std::vector<float>{},
			std::vector<unsigned int>{},
			std::vector<unsigned int>{},
			std::vector<unsigned int>{});

		std::string text;
		float translation_scale_factor = 1.0f;
		float font_size;
		if (language == Textbox::Language::English || !textbox.translations.contains(language)) {
			text = textbox.text;
			translation_scale_factor = 1.0f;
		} else if(textbox.translations.contains(language)){
			text = textbox.translations.at(language);
			translation_scale_factor = std::min(1.0f,((float)textbox.text.size()) / ((float)text.size()));
			translation_scale_factor = std::max(translation_scale_factor, .7f);
		}
		font_size = textbox.font_size * translation_scale_factor;

		const size_t& strlen = text.size();
		for (size_t i = 0; i < strlen; i++) {
			char c = text[i];
			if (convert_PS_to_keyboard && textbox.ps_to_keyboard_.contains(c)) {
				c = textbox.ps_to_keyboard_.at(c);
			}
			char_info char_info_ = font.getCharInfo(c);
			int word_count = 0;
			float word_len = 0.0f;
			while (word_count + i < strlen) {
				if (text[i + word_count] == '\n' || text[i + word_count] == ' ') {
					break;
				} else {
					char c_temp = textbox.text[i];
					if (convert_PS_to_keyboard && textbox.ps_to_keyboard_.contains(c_temp)) {
						c_temp = textbox.ps_to_keyboard_.at(c_temp);
					}
					word_len += font.getCharInfo(c_temp).unscaled_width * font_size;
					word_count++;
				}
			}
			float word_end = line_length + word_len;
			float char_end = line_length + char_info_.unscaled_width * font_size;
			if (word_len <= textbox.box_width && word_end > textbox.box_width ) {
				char_end = char_info_.unscaled_width * font_size;
				line_num++;
				line_length = 0.0f;
			} else if (char_end > textbox.box_width || c == '\n') {
				char_end = char_info_.unscaled_width * textbox.font_size;
				line_num++;
				line_length = 0.0f;
			}
			if (line_length == 0.0f && c == ' ') {
				char_end = 0.0f;
			}
			float line_top = -line_num * font.getUnscaledLineHeight() * font_size;
			int vert_index_offset = textbox_model->getVerts().size() / 3;
			//add verts model
			textbox_model->addVert(Eigen::Vector3f(line_length, line_top, 0));
			textbox_model->addVert(Eigen::Vector3f(line_length, line_top - font_size * char_info_.unscaled_height, 0));
			textbox_model->addVert(Eigen::Vector3f(char_end, line_top, 0));
			textbox_model->addVert(Eigen::Vector3f(char_end, line_top - font_size * char_info_.unscaled_height, 0));//bottom right
			//add tex coords to model
			textbox_model->addTexCoord(char_info_.glyph_left, 1-(char_info_.glyph_top));
			textbox_model->addTexCoord(char_info_.glyph_left, 1-(char_info_.glyph_top + char_info_.unscaled_height));
			textbox_model->addTexCoord(char_info_.glyph_left + char_info_.unscaled_width, 1- char_info_.glyph_top);
			textbox_model->addTexCoord(char_info_.glyph_left + char_info_.unscaled_width, 1-(char_info_.glyph_top + char_info_.unscaled_height));
			//add face to model
			textbox_model->addFace(vert_index_offset, vert_index_offset + 1, vert_index_offset + 2);
			textbox_model->addFace(vert_index_offset + 1, vert_index_offset + 2, vert_index_offset + 3);
			line_length = char_end;
		}
		return textbox_model;
	}


};

const char* TextGraphics::vertex_code = "\n"
"#version 330 core\n"
"layout (location = 0) in vec3 pos;\n"
"layout (location = 1) in vec2 vt;\n"
//"layout (location = 2) int vt_offset;\n"

//"uniform float line_height;\n"
//"uniform float numeric_char_width;\n"
"uniform mat4 position_matrix;\n"

"out vec2 texCoord;\n"

"void main()\n"
"{\n"
"   gl_Position = position_matrix * vec4(pos.x, pos.y, 0, 1.0);\n"
//"	if(vt_offset > 1) {vt.y += line_height;vt.x = numeric_char_width*(vt_offset-2);}\n"
//"	else if (vt_offset == 1) {vt += numeric_char_width;}\n"
"	texCoord = vt;\n"
"}\0";

const char* TextGraphics::fragment_code = "#version 330 core\n"
"in vec2 texCoord;\n "
"uniform sampler2D tex;\n"

"out vec4 FragColor;\n"

"void main()\n"
"{\n"
"	vec4 pointColor = texture(tex,texCoord);\n"
"	if(pointColor.a < .01) discard;"
"	FragColor = pointColor;\n"
//"	FragColor = vec4(0.,0.,0.,1.);\n"

" } ";
#endif
