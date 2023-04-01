#pragma once

#ifndef PUPPET_GRAPHICS_DEFAULT3D
#define PUPPET_GRAPHICS_DEFAULT3D

#include <Eigen/Dense>
#define _USE_MATH_DEFINES
#include <cmath>

#include "Graphics.h"
#include "camera.h"
#include "GameObject.h"

using Eigen::Matrix4f;

class Default3d : public Graphics<GameObject,int,int,size_t> { //VAO, tex_id, n_elems

private:
	const unsigned int perspective_location_;
	const unsigned int camera_location_;
	const unsigned int model_location_;

	const InternalObject& camera_;
	//all this should be moved to camera
	float fov_;
	float near_clip_;
	float far_clip_;
	Matrix4f perspective_;

	constexpr int& getVAO(Cache cache) const {
		return std::get<0>(cache);
	}

	constexpr int& getTexID(Cache cache) const {
		return std::get<1>(cache);
	}
	constexpr size_t& getNElems(Cache cache) const {
		return std::get<2>(cache);
	}

	virtual std::tuple<int, int, size_t> makeDataCache(const GameObject& obj) const override {
		const Model& model = *(obj.getModel());
		const Texture& tex = *(obj.getTexture());
		// = model.flen();
		unsigned int VAO;
		glGenVertexArrays(1, &(VAO));
		//this->VAO = static_cast<int>(VAO);
		unsigned int VBO[3];
		glGenBuffers(3, VBO);
		unsigned int EBO;
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * model.vlen() * 3, model.getVerts().data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, VBO[1]); //size is wrong here! need to change it depending on how we implement norm EBO
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * model.getNorms().size(), model.getNorms().data(), GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * model.getTexCoords().size(), model.getTexCoords().data(), GL_STATIC_DRAW);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(2);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);//have to change this to face length not vertex len
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * model.flen() * 3, model.getFaces().data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		//texture code:

		unsigned int tex_id;
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

		return std::tuple<int, int, size_t>{VAO, tex_id, model.flen()};
	}

	virtual void deleteDataCache(Cache cache) const override {
		//...
	}

public:

	void drawObj(const GameObject& obj, Cache cache) const override {
		glBindTexture(GL_TEXTURE_2D, getTexID(cache));
		glBindVertexArray(getVAO(cache));
		//should remove inverse here
	


		glUniformMatrix4fv(model_location_, 1, GL_FALSE, obj.getPosition().data());
		glDrawElements(GL_TRIANGLES, 3*getNElems(cache), GL_UNSIGNED_INT, 0);
		//for (auto const& o : obj.getChildren()) {
		//	draw(*o);
		//}
	}

	void beginDraw() const override {
		Graphics::beginDraw();
		glEnable(GL_DEPTH_TEST);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glUniformMatrix4fv(perspective_location_, 1, GL_FALSE, perspective_.data());
		glUniformMatrix4fv(camera_location_, 1, GL_FALSE, Matrix4f(camera_.getPosition().inverse()).data());
		//default3d specific code
	}

	void endDraw() const override {
		Graphics::endDraw();
		//default3d specific code
	}

	Default3d(const Camera& camera,float near_clip,float far_clip, float fov):
		model_location_(glGetUniformLocation(gl_id, "model")),
		camera_location_(glGetUniformLocation(gl_id, "camera")),
		perspective_location_(glGetUniformLocation(gl_id, "perspective")),
		camera_(camera),
		near_clip_(near_clip),
		far_clip_(far_clip),
		fov_(fov){

		float S = 1./(tan(fov_/2.*M_PI/180.));
		perspective_ << S,0,0,0,
						0,S,0,0,
						0,0,-(far_clip_)/(far_clip_-near_clip_),-2*far_clip_ * near_clip_ / (far_clip_ - near_clip_),
						0,0,-1.,0;
		//perspective_ << 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1;
	}

};
const char* Default3d::vertex_code = "\n"
"#version 330 core\n"
"layout (location = 0) in vec3 pos;\n"
"layout (location = 1) in vec3 norm;\n"
"layout (location = 2) in vec2 vt;\n"

"uniform mat4 perspective;\n"
"uniform mat4 camera;\n"
"uniform mat4 model;\n"

"out vec2 texCoord;\n"

"void main()\n"
"{\n"
"   gl_Position = perspective * camera * model * vec4(pos.x, pos.y, pos.z, 1.0);\n"
"	texCoord = vt;\n"
"}\0";
const char* Default3d::fragment_code = "#version 330 core\n"
"in vec2 texCoord;\n "
"uniform sampler2D tex;\n"

"out vec4 FragColor;\n"

"void main()\n"
"{\n"
"	FragColor = texture(tex,texCoord);\n"
" } ";
/*
Matrix4f Default3d::perspective_((Matrix4f() << 1.0, 0.0, 0.0, 0.0,
	0.0, 1.0, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.0, 0.0, 0.0, 1.0).finished());*/

#endif
