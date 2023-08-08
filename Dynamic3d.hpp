#pragma once

#ifndef PUPPET_GRAPHICS_DYNAMIC3D
#define PUPPET_GRAPHICS_DYNAMIC3D

#include <Eigen/Dense>
#include <cmath>

#include "Graphics.hpp"
#include "camera.h"
#include "GameObject.h"

using Eigen::Matrix4f;

class Dynamic3d : public Graphics<GameObject,int, int, size_t, unsigned int, unsigned int> { //VAO, tex_id, n_elems, pos vbo, norm vbo, pos_vector, norm_vector

private:
	const unsigned int perspective_location_;
	const unsigned int camera_location_;
	const unsigned int model_location_;

	const Camera* camera_;


	constexpr int& getVAO(Cache cache) const {
		return std::get<0>(cache);
	}

	constexpr int& getTexID(Cache cache) const {
		return std::get<1>(cache);
	}
	constexpr size_t& getNElems(Cache cache) const {
		return std::get<2>(cache);
	}
	constexpr unsigned int& getPosVBO(Cache cache) const {
		return std::get<3>(cache);
	}

	constexpr unsigned int& getNormVBO(Cache cache) const {
		return std::get<4>(cache);
	}

	virtual Cache makeDataCache(const GameObject& obj) const override {
		const Model& model = *(obj.getModel());
		const Texture& tex = *(obj.getTexture());

		unsigned int VAO;
		glGenVertexArrays(1, &(VAO));
		unsigned int VBO[3];
		glGenBuffers(3, VBO);
		unsigned int EBO;
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);

		std::vector<float>* vert_pos = new std::vector<float>(3 * model.vlen());
		vert_pos->reserve(3 * model.vlen());
		for (int i = 0; i < vert_pos->size(); i++) {
			(*vert_pos)[i] = 0.;
		}

		std::vector<float>* vert_norm = new std::vector<float>(3 * model.vlen());
		vert_norm->reserve(3 * model.vlen());
		for (int i = 0; i < vert_norm->size(); i++) {
			(*vert_norm)[i] = 0.;
		}
		glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * model.getTexCoords().size(), model.getTexCoords().data(), GL_STATIC_DRAW);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(2);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
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
		if (tex.n_channels == 3) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex.width, tex.height, 0, GL_RGB, GL_UNSIGNED_BYTE, tex.getData().data());
		}
		else if (tex.n_channels == 4) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex.width, tex.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex.getData().data());

		}
		glGenerateMipmap(GL_TEXTURE_2D);

		return Cache{VAO,tex_id, model.flen(), VBO[0], VBO[1]};
	}

	virtual void deleteDataCache(Cache cache) const override {
		//...
	}

public:

	void drawObj(const GameObject& obj, Cache cache) const override {
		if (!obj.isHidden()) {
			glUniformMatrix4fv(model_location_, 1, GL_FALSE, obj.getPosition().data());

			glBindTexture(GL_TEXTURE_2D, getTexID(cache));
			glBindVertexArray(getVAO(cache));

			glBindBuffer(GL_ARRAY_BUFFER, getPosVBO(cache));

			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * obj.getModel()->vlen() * 3, obj.getModel()->getVerts().data(), GL_DYNAMIC_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);

			glBindBuffer(GL_ARRAY_BUFFER, getNormVBO(cache));
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * obj.getModel()->getNorms().size(), obj.getModel()->getNorms().data(), GL_DYNAMIC_DRAW);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);

			glDrawElements(GL_TRIANGLES, 3 * getNElems(cache), GL_UNSIGNED_INT, 0);
		}
		//for (auto const& o : obj.getChildren()) {
		//	draw(*o);
		//}
	}

	void beginDraw() const override {
		glEnable(GL_DEPTH_TEST);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glUniformMatrix4fv(perspective_location_, 1, GL_FALSE, camera_->getPerspective().data());
		glUniformMatrix4fv(camera_location_, 1, GL_FALSE, camera_->getCameraMatrix().data());

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		//default3d specific code
	}

	void endDraw() const override {
		//default3d specific code
	}

	void setCamera(Camera* camera) {
		camera_ = camera;
	}

	Dynamic3d() :
		model_location_(glGetUniformLocation(gl_id, "model")),
		camera_location_(glGetUniformLocation(gl_id, "camera")),
		perspective_location_(glGetUniformLocation(gl_id, "perspective")),
		camera_(nullptr) {

		//perspective_ << 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1;
	}

};
/*
Matrix4f Default3d::perspective_((Matrix4f() << 1.0, 0.0, 0.0, 0.0,
	0.0, 1.0, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.0, 0.0, 0.0, 1.0).finished());*/

#endif

