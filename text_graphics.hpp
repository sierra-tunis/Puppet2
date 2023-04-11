#pragma once

#ifndef PUPPET_GRAPHICSTEXT
#define PUPPET_GRAPHICSTEXT

#include <string>

#include "Graphics.h"

//e.g. of how to implement tediously large number of cached argumend

struct char_info {
	float unscaled_height, unscaled_width;
};

struct Textbox{
	std::string text;
	std::string font;
	float top, left;
	float box_width, box_height;
	float font_size;
};

class Font {

	Texture glyph;
	int tex_id;
	const std::unordered_map<char, char_info> char_info_bank;

public:
	int getTexID() const {
		return tex_id;
	}

	char_info getCharInfo(const char& c) const {
		return char_info_bank.at(c);
	}
};

class TextGraphics : Graphics<Textbox, unsigned int, unsigned int> {
								//text to draw, VAO, textbox_info
	std::unordered_map<std::string, Font> named_fonts_;

	static Model getTextboxModel(Textbox textbox, Font font) {
		float line_length = 0;
		int line_num = 0;
		for (const char& c : textbox.text) {
			float char_end = line_length + font.getCharInfo(c).unscaled_width * textbox.font_size;
			if (char_end > textbox.box_width) {
				line_num++;
				line_length = 0;
			} else {
				line_length = char_end;
			}
			//add verts and vtx to model;
		}
	}

	Cache makeDataCache(const Textbox& obj) const override {
		const Model& model = getTextboxModel(obj,named_fonts_.at(obj.font));
		// = model.flen();
		unsigned int VAO;
		glGenVertexArrays(1, &(VAO));
		//this->VAO = static_cast<int>(VAO);
		unsigned int VBO[2];
		glGenBuffers(2, VBO);

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);//getverts must be xy only!//only pulls first 4
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * 3, model.getVerts().data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * 2, model.getTexCoords().data(), GL_STATIC_DRAW);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		//texture code:
		unsigned int glyph_tex_id = named_fonts_.at(obj.font).getTexID();
		return Cache{ VAO, glyph_tex_id };
	};

	void deleteDataCache(Cache cache) const override {

	};

};

#endif
