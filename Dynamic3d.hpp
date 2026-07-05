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

struct Dynamic3dStaticSubcache {
	unsigned int static_VAO;
	unsigned int static_VBO[3];
	unsigned int flen;
	const Eigen::Matrix4f* root_tform;

	Dynamic3dStaticSubcache(unsigned int static_VAO, unsigned int* static_VBO, unsigned int flen,const Eigen::Matrix4f* root_tform):
		static_VAO(static_VAO), static_VBO{static_VBO[0],static_VBO[1],static_VBO[2]},flen(flen),root_tform(root_tform) {

	}

};

struct Dynamic3dCache {
	unsigned int VAO;
	unsigned int VBO[3];
	unsigned int tex_id[2];
	size_t n_elems;
	unsigned int pos_vbo;
	unsigned int norm_vbo;

	std::vector<std::tuple<Dynamic3dStaticSubcache>> static_VAOs;

	Eigen::Vector4f overlay_color;

	Dynamic3dCache() : VAO(-1), VBO{ 0,0,0 }, tex_id{ 0,0 }, n_elems(0), pos_vbo(0), norm_vbo(0), overlay_color(0, 0, 0, 0) {
	};
	Dynamic3dCache(unsigned int VAO, unsigned int* VBO, unsigned int* tex_id, size_t n_elems,unsigned int pos_vbo,unsigned int norm_vbo, std::vector<std::tuple<Dynamic3dStaticSubcache>> static_VAOs)
		: VAO(VAO), VBO{VBO[0],VBO[1],VBO[2]}, tex_id{tex_id[0],tex_id[1]}, n_elems(n_elems),
			pos_vbo(pos_vbo), norm_vbo(norm_vbo),static_VAOs(static_VAOs),
			overlay_color(0.0f, 0.0f, 0.0f, 0.0f) {
	};
};

class Dynamic3d : public Graphics<GameObject,Dynamic3dCache> { //VAO, tex_id, n_elems, pos vbo, norm vbo, static vaos(VAO,n_elems,position)

private:
	const unsigned int perspective_location_;
	const unsigned int camera_location_;
	const unsigned int model_location_;

	const unsigned int tex_location_;
	const unsigned int overlay_tex_location_;

	static constexpr int max_lights = 3;


	const unsigned int& getVAO(const Cache& cache) const {
		return std::get<0>(cache).VAO;
	}

	const unsigned int* getVBO(const Cache& cache) const {
		return std::get<0>(cache).VBO;
	}
	const unsigned int* getTexID(const Cache& cache) const {
		return std::get<0>(cache).tex_id;
	}
	const size_t& getNElems(const Cache& cache) const {
		return std::get<0>(cache).n_elems;
	}
	const unsigned int& getPosVBO(const Cache& cache) const {
		return std::get<0>(cache).pos_vbo;
	}

	const unsigned int& getNormVBO(const Cache& cache) const {
		return std::get<0>(cache).norm_vbo;
	}

	const std::vector<std::tuple<Dynamic3dStaticSubcache>>& getStaticVAOs(const Cache& cache) const {
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

		glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * obj.getModel()->vlen() * 3, obj.getModel()->getVerts().data(),GL_DYNAMIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * obj.getModel()->getNorms().size(), obj.getModel()->getNorms().data(), GL_DYNAMIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * model.getTexCoords().size(), model.getTexCoords().data(), GL_STATIC_DRAW);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(2);

		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * model.flen() * 3, model.getFaces().data(), GL_STATIC_DRAW);

		const DynamicModel* dyn_model = dynamic_cast<const DynamicModel*>(obj.getModel());

		std::vector<std::tuple<Dynamic3dStaticSubcache>> static_VAOs;
		if (dyn_model != nullptr) {
			for (auto& stat_mod : dyn_model->getStaticModels()) {
				Model static_model = *stat_mod.second;

				unsigned int sVAO;
				glGenVertexArrays(1, &sVAO);
				unsigned int sVBO[3];
				glGenBuffers(3, sVBO);

				static_VAOs.push_back(std::tuple<Dynamic3dStaticSubcache>{Dynamic3dStaticSubcache(sVAO, sVBO, static_model.flen(), stat_mod.first->getTform())});


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

		unsigned int tex_id[2];
		glGenTextures(2, tex_id);
		glBindTexture(GL_TEXTURE_2D, tex_id[0]);
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

		glBindTexture(GL_TEXTURE_2D, tex_id[1]);
		const Texture* overlay_tex;
		if (obj.getOverlayTexture() == nullptr) {
			overlay_tex = new Texture(2, 2, 4, { 0,0,0,0,
										0,0,0,0,
										0,0,0,0,
										0,0,0,0 });
		} else {
			overlay_tex = obj.getOverlayTexture();
		}

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		if (tex.n_channels == 3) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, overlay_tex->width, overlay_tex->height, 0, GL_RGB, GL_UNSIGNED_BYTE, overlay_tex->getData().data());
		}
		else if (tex.n_channels == 4) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, overlay_tex->width, overlay_tex->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, overlay_tex->getData().data());

		}
		glGenerateMipmap(GL_TEXTURE_2D);
		
		delete vert_norm;
		delete vert_pos;

		return Cache{ Dynamic3dCache(VAO,VBO,tex_id, model.flen(), VBO[0], VBO[1],static_VAOs) };
	}

	virtual void deleteDataCache(Cache& cache) const override {
		glDeleteVertexArrays(1, &getVAO(cache));
		glDeleteBuffers(3, getVBO(cache));
		glDeleteTextures(2, getTexID(cache));
		for (const std::tuple<Dynamic3dStaticSubcache>& s : getStaticVAOs(cache)) {
			glDeleteVertexArrays(1, &std::get<0>(s).static_VAO);
			glDeleteBuffers(3, std::get<0>(s).static_VBO);
		}
	}

	static bool closerToPlayer(const Scene::light* a, const Scene::light* b) {
		Eigen::Vector3f player_pos = InternalObject::getNamedObjectAs<GameObject>("player")->getPosition()(seq(0, 2), 3);
		return (a->position - player_pos).norm() < (b->position - player_pos).norm();
	}

public:

	void drawObj(const GameObject& obj, const Cache& cache) const override {
		if (!obj.isHidden()) {
			glUniformMatrix4fv(model_location_, 1, GL_FALSE, obj.getPosition().data());
			glUniform4fv(glGetUniformLocation(gl_id, "overlay_color"), 1, std::get<0>(cache).overlay_color.data());

			glActiveTexture(GL_TEXTURE0 + 0);
			glBindTexture(GL_TEXTURE_2D, getTexID(cache)[0]);

			glActiveTexture(GL_TEXTURE0 + 1);
			glBindTexture(GL_TEXTURE_2D, getTexID(cache)[1]);

			glBindVertexArray(getVAO(cache));

			glBindBuffer(GL_ARRAY_BUFFER, getPosVBO(cache));
			glBufferSubData(GL_ARRAY_BUFFER,0, sizeof(float) * obj.getModel()->vlen() * 3, obj.getModel()->getVerts().data());
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);

			glBindBuffer(GL_ARRAY_BUFFER, getNormVBO(cache));
			glBufferSubData(GL_ARRAY_BUFFER,0, sizeof(float) * obj.getModel()->getNorms().size(), obj.getModel()->getNorms().data());
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);

			glDrawArrays(GL_TRIANGLES, 0, 3 * getNElems(cache));

			const DynamicModel* dyn_model = dynamic_cast<const DynamicModel*>(obj.getModel());
			if (dyn_model != nullptr) {
				
				for (int i = 0; i < getStaticVAOs(cache).size(); i++) {
					const Dynamic3dStaticSubcache& sVAO_pos_pair = std::get<0>(getStaticVAOs(cache)[i]);
					//glBindTexture(GL_TEXTURE_2D, getTexID(cache));
					glBindVertexArray(sVAO_pos_pair.static_VAO);
					
					glUniformMatrix4fv(model_location_, 1, GL_FALSE, sVAO_pos_pair.root_tform->data());
					glDrawArrays(GL_TRIANGLES, 0, 3 * sVAO_pos_pair.flen);
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

		glUniform1i(tex_location_, 0);
		glUniform1i(overlay_tex_location_, 1);

		glUniform4f(glGetUniformLocation(gl_id, "atmosphere_color"), scene_->atmosphere_color(0), scene_->atmosphere_color(1), scene_->atmosphere_color(2), scene_->atmosphere_strength);
		glUniform1f(glGetUniformLocation(gl_id, "ambient_light"), scene_->ambient_light);
		glUniform1f(glGetUniformLocation(gl_id, "white_reduction"), scene_->white_reduction);

		if (scene_->primary_light_ != nullptr) {
			glUniform3fv(glGetUniformLocation(gl_id, "light_position"), 1, scene_->primary_light_->position.data());
			glUniform3fv(glGetUniformLocation(gl_id, "light_color"), 1, scene_->primary_light_->color.data());
			glUniform1f(glGetUniformLocation(gl_id, "light_strength"), scene_->primary_light_->brightness);
		}
		else {
			glUniform1f(glGetUniformLocation(gl_id, "light_strength"), 0);
		}
		std::set<const Scene::light*, decltype(&closerToPlayer)> ordered_lights_(&closerToPlayer);
		for (const auto& l : scene_->secondary_lights_) {
			if (l->on) {
				ordered_lights_.insert(l);
			}
		}
		int i = 0;
		for (auto& l : ordered_lights_) {
			if (i < ordered_lights_.size()) {
				glUniform3fv(glGetUniformLocation(gl_id, ("light_position_" + std::to_string(i + 1)).c_str()), 1, l->position.data());
				glUniform3fv(glGetUniformLocation(gl_id, ("light_color_" + std::to_string(i + 1)).c_str()), 1, l->color.data());
				glUniform1f(glGetUniformLocation(gl_id, ("light_strength_" + std::to_string(i + 1)).c_str()), l->brightness);
			}
			else {
				glUniform1f(glGetUniformLocation(gl_id, ("light_strength_" + std::to_string(i + 1)).c_str()), 0);
			}
			i++;
			if (i == max_lights) {
				break;
			}
		}
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		//default3d specific code
	}

	void endDraw() const override {
		//default3d specific code
		glActiveTexture(GL_TEXTURE0);
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
		perspective_location_(glGetUniformLocation(gl_id, "perspective")),
		tex_location_(glGetUniformLocation(gl_id,"tex")),
		overlay_tex_location_(glGetUniformLocation(gl_id, "overlay_tex")){

		//perspective_ << 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1;
	}

};
/*
Matrix4f Default3d::perspective_((Matrix4f() << 1.0, 0.0, 0.0, 0.0,
	0.0, 1.0, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.0, 0.0, 0.0, 1.0).finished());*/

#endif

