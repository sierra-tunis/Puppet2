#pragma once

#ifndef PUPPET_GRAPHICS_DYNAMIC3D
#define PUPPET_GRAPHICS_DYNAMIC3D

#include <Eigen/Dense>
#include <cmath>

#include "Graphics.hpp"
#include "camera.h"
#include "GameObject.h"
#include "scene.hpp"
#include "dynamic_model.hpp"
#include "tuple"

using Eigen::Matrix4f;


struct Dynamic3dCache {
	unsigned int VAO;
	unsigned int tex_id;
	size_t n_elems;
	unsigned int pos_vbo;
	unsigned int norm_vbo;

	std::vector<std::tuple<unsigned int, unsigned int, const Eigen::Matrix4f*>> static_VAOs;

	Eigen::Vector4f overlay_color;

	Dynamic3dCache() : VAO(-1), tex_id(-1), n_elems(0), pos_vbo(0),norm_vbo(0), overlay_color(0, 0, 0, 0) {
	};
	Dynamic3dCache(int VAO, int tex_id, size_t n_elems,unsigned int pos_vbo,unsigned int norm_vbo, std::vector<std::tuple<unsigned int, unsigned int, const Eigen::Matrix4f*>> static_VAOs)
		: VAO(VAO), tex_id(tex_id), n_elems(n_elems),
			pos_vbo(pos_vbo), norm_vbo(norm_vbo),static_VAOs(static_VAOs),
			overlay_color(0.0f, 0.0f, 0.0f, 0.0f) {
	};
};

class Dynamic3d : public Graphics<GameObject,Dynamic3dCache> { //VAO, tex_id, n_elems, pos vbo, norm vbo, static vaos(VAO,n_elems,position)

private:
	const unsigned int perspective_location_;
	const unsigned int camera_location_;
	const unsigned int model_location_;

	static constexpr int max_lights = 3;


	unsigned int& getVAO(Cache cache) const {
		return std::get<0>(cache).VAO;
	}

	unsigned int& getTexID(Cache cache) const {
		return std::get<0>(cache).tex_id;
	}
	size_t& getNElems(Cache cache) const {
		return std::get<0>(cache).n_elems;
	}
	unsigned int& getPosVBO(Cache cache) const {
		return std::get<0>(cache).pos_vbo;
	}

	unsigned int& getNormVBO(Cache cache) const {
		return std::get<0>(cache).norm_vbo;
	}

	const std::vector<std::tuple<unsigned int, unsigned int, const Eigen::Matrix4f*>>& getStaticVAOs(Cache& cache) const {
		return std::get<0>(cache).static_VAOs;
	}

	virtual Cache makeDataCache(const GameObject& obj) const override {
		const Model& model = *(obj.getModel());
		const Texture& tex = *(obj.getTexture());

		unsigned int VAO;
		glGenVertexArrays(1, &(VAO));
		unsigned int VBO[3];
		glGenBuffers(3, VBO);
		
		//unsigned int EBO;
		//glGenBuffers(1, &EBO);

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

		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * model.flen() * 3, model.getFaces().data(), GL_STATIC_DRAW);

		const DynamicModel* dyn_model = dynamic_cast<const DynamicModel*>(obj.getModel());

		std::vector<std::tuple<unsigned int, unsigned int, const Eigen::Matrix4f*>> static_VAOs;
		if (dyn_model != nullptr) {
			for (auto& stat_mod : dyn_model->getStaticModels()) {
				Model static_model = *stat_mod.second;

				unsigned int sVAO;
				glGenVertexArrays(1, &sVAO);
				static_VAOs.push_back({ sVAO,static_model.flen(),stat_mod.first->getTform() });
				unsigned int sVBO[3];
				glGenBuffers(3, sVBO);

				glBindVertexArray(sVAO);

				glBindBuffer(GL_ARRAY_BUFFER, sVBO[0]);
				glBufferData(GL_ARRAY_BUFFER, sizeof(float) * static_model.flen() * 9, static_model.getVerts().data(), GL_STATIC_DRAW);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
				glEnableVertexAttribArray(0);

				glBindBuffer(GL_ARRAY_BUFFER, sVBO[1]); //size is wrong here! need to change it depending on how we implement norm EBO
				glBufferData(GL_ARRAY_BUFFER, sizeof(float) * static_model.flen() * 9, static_model.getNorms().data(), GL_STATIC_DRAW);
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
				glEnableVertexAttribArray(1);

				glBindBuffer(GL_ARRAY_BUFFER, sVBO[2]);
				glBufferData(GL_ARRAY_BUFFER, sizeof(float) * static_model.flen() * 6, static_model.getTexCoords().data(), GL_STATIC_DRAW);
				glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
				glEnableVertexAttribArray(2);

				
			}
		}

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
		
		delete vert_norm;
		delete vert_pos;

		return Dynamic3dCache(VAO,tex_id, model.flen(), VBO[0], VBO[1],static_VAOs);
	}

	virtual void deleteDataCache(Cache cache) const override {
		glDeleteVertexArrays(1, &getVAO(cache));
		glDeleteTextures(1, &getTexID(cache));
	}

public:

	void drawObj(const GameObject& obj, Cache cache) const override {
		if (!obj.isHidden()) {
			glUniformMatrix4fv(model_location_, 1, GL_FALSE, obj.getPosition().data());
			glUniform4fv(glGetUniformLocation(gl_id, "overlay_color"), 1, std::get<0>(cache).overlay_color.data());


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

			glDrawArrays(GL_TRIANGLES, 0, 3 * getNElems(cache));

			const DynamicModel* dyn_model = dynamic_cast<const DynamicModel*>(obj.getModel());
			if (dyn_model != nullptr) {
				
				for (int i = 0; i < getStaticVAOs(cache).size(); i++) {
					const auto& sVAO_pos_pair = getStaticVAOs(cache)[i];
					//glBindTexture(GL_TEXTURE_2D, getTexID(cache));
					glBindVertexArray(std::get<0>(sVAO_pos_pair));
					
					glUniformMatrix4fv(model_location_, 1, GL_FALSE, std::get<2>(sVAO_pos_pair)->data());
					glDrawArrays(GL_TRIANGLES, 0, 3 * std::get<1>(sVAO_pos_pair));
				}
			}


			//glDrawElements(GL_TRIANGLES, 3 * getNElems(cache), GL_UNSIGNED_INT, 0);
		}
		//for (auto const& o : obj.getChildren()) {
		//	draw(*o);
		//}
	}

	void beginDraw() const override {
		glEnable(GL_DEPTH_TEST);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glUniformMatrix4fv(perspective_location_, 1, GL_FALSE, scene_->camera->getPerspective().data());
		glUniformMatrix4fv(camera_location_, 1, GL_FALSE, scene_->camera->getCameraMatrix().data());

		glUniform4f(glGetUniformLocation(gl_id, "atmosphere_color"), scene_->atmosphere_color(0), scene_->atmosphere_color(1), scene_->atmosphere_color(2), scene_->atmosphere_strength);
		if (scene_->primary_light_ != nullptr) {
			glUniform3fv(glGetUniformLocation(gl_id, "light_position"), 1, scene_->primary_light_->position.data());
			glUniform3fv(glGetUniformLocation(gl_id, "light_color"), 1, scene_->primary_light_->color.data());
			glUniform1f(glGetUniformLocation(gl_id, "light_strength"), scene_->primary_light_->brightness);
		}
		else {
			glUniform1f(glGetUniformLocation(gl_id, "light_strength"), 0);
		}
		for (int i = 0; i < max_lights; i++) {
			if (i < scene_->secondary_lights_.size()) {
				glUniform3fv(glGetUniformLocation(gl_id, ("light_position_" + std::to_string(i + 1)).c_str()), 1, scene_->secondary_lights_[i]->position.data());
				glUniform3fv(glGetUniformLocation(gl_id, ("light_color_" + std::to_string(i + 1)).c_str()), 1, scene_->secondary_lights_[i]->color.data());
				glUniform1f(glGetUniformLocation(gl_id, ("light_strength_" + std::to_string(i + 1)).c_str()), scene_->secondary_lights_[i]->brightness);
			}
			else {
				glUniform1f(glGetUniformLocation(gl_id, ("light_strength_" + std::to_string(i + 1)).c_str()), 0);
			}
		}
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		//default3d specific code
	}

	void endDraw() const override {
		//default3d specific code
	}

	/*void setCamera(Camera* camera) {
		if (scene_ == nullptr) {
			scene_ = new Scene();
		}
		scene_->camera = camera;
	}*/

	void setScene(Scene* scene) {
		scene_ = scene;
	}

	void setOverlayColor(const GameObject& obj, Eigen::Vector4f color) {
		std::get<0>(getCache(obj)).overlay_color = color;
	}

	Dynamic3d() :
		model_location_(glGetUniformLocation(gl_id, "model")),
		camera_location_(glGetUniformLocation(gl_id, "camera")),
		perspective_location_(glGetUniformLocation(gl_id, "perspective")){

		//perspective_ << 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1;
	}

};
/*
Matrix4f Default3d::perspective_((Matrix4f() << 1.0, 0.0, 0.0, 0.0,
	0.0, 1.0, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.0, 0.0, 0.0, 1.0).finished());*/

#endif

