#pragma once

#include "Graphics.h"
#include "GameObject.h"

class Default2d : public Graphics<GameObject, int, int> {
											//vao, tex_id
	const unsigned int position_location_;


	constexpr int& getVAO(Cache cache) const {
		return std::get<0>(cache);
	}

	constexpr int& getTexID(Cache cache) const {
		return std::get<1>(cache);
	}

	Cache makeDataCache(const GameObject& obj) const override {
		const Model& model = *(obj.getModel());
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
		unsigned int tex_id;
		if (obj.getTexture() != nullptr) {
			const Texture& tex = *(obj.getTexture());

			glGenTextures(1, &(tex_id));
			glBindTexture(GL_TEXTURE_2D, tex_id);
			//this->tex_id = static_cast<int>(tex_id);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex.width, tex.height, 0, GL_RGB, GL_UNSIGNED_BYTE, tex.getData().data());
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else {
			tex_id = 0;
		}
		return Cache{VAO, tex_id};
	}

	void deleteDataCache(Cache cache) const override {

	}

public:
	Default2d() : position_location_(glGetUniformLocation(gl_id, "position")) {}

	void beginDraw() const override {
		Graphics::beginDraw();
		glDisable(GL_DEPTH_TEST);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		//default3d specific code
	}

	void drawObj(const GameObject& obj, Cache cache) const override {
		glBindTexture(GL_TEXTURE_2D, getTexID(cache));
		//should remove inverse here



		//glUniformMatrix4fv(position_location_, 1, GL_FALSE, obj.getPosition().data());
		glBindVertexArray(getVAO(cache));
		glDrawArrays(GL_TRIANGLE_STRIP, 0,4);
		//for (auto const& o : obj.getChildren()) {
		//	draw(*o);
		//}
	}
};

const char* Default2d::vertex_code = "\n"
"#version 330 core\n"
"layout (location = 0) in vec3 pos;\n"
"layout (location = 1) in vec2 vtx;\n"


"uniform mat4 position_matrix;\n"

"out vec2 texCoord;\n"

"void main()\n"
"{\n"
"   gl_Position = vec4(pos.x, pos.y, 0, 1.0);\n"
"	texCoord = vt;\n"
"}\0";
const char* Default2d::fragment_code = "#version 330 core\n"
"in vec2 texCoord;\n "
"uniform sampler2D tex;\n"

"out vec4 FragColor;\n"

"void main()\n"
"{\n"
//"	FragColor = texture(tex,texCoord);\n"
"	FragColor = vec4(0.,0.,0.,1.);\n"
" } ";