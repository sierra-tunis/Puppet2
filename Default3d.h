#pragma once

#ifndef PUPPET_GRAPHICS_DEFAULT3D
#define PUPPET_GRAPHICS_DEFAULT3D

#include <Eigen/Dense>
#include <cmath>

#include "Graphics.hpp"
#include "camera.h"
#include "GameObject.h"
#include "scene.hpp"

using Eigen::Matrix4f;

struct Default3dCache {
	unsigned int VAO;
	unsigned int* VBO;
	unsigned int tex_id;
	size_t n_elems;
	Eigen::Vector4f overlay_color;

	Default3dCache() : VAO(-1), VBO(nullptr), tex_id(-1), n_elems(0), overlay_color(0, 0, 0, 0) {
	};
	Default3dCache(unsigned int VAO, unsigned int* VBO, unsigned int tex_id, size_t n_elems) : VAO(VAO),VBO(VBO),tex_id(tex_id), n_elems(n_elems),overlay_color(0.0f,0.0f,0.0f,0.0f){
	};


};

class Default3d : public Graphics<GameObject, Default3dCache> { //VAO, tex_id, n_elems

private:
	const unsigned int perspective_location_;
	const unsigned int camera_location_;
	const unsigned int model_location_;

	//const Camera* camera_;

	//Eigen::Vector3f atmosphere_color_;
	//float atmosphere_strength_;

	static constexpr int max_lights = 6;

	unsigned int& getVAO(Cache cache) const {
		return std::get<0>(cache).VAO;
	}

	unsigned int* getVBO(Cache cache) const {
		return std::get<0>(cache).VBO;
	}

	unsigned int& getTexID(Cache cache) const {
		return std::get<0>(cache).tex_id;
	}
	size_t& getNElems(Cache cache) const {
		return std::get<0>(cache).n_elems;
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

		glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * model.flen() * 9, model.getVerts().data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, VBO[1]); //size is wrong here! need to change it depending on how we implement norm EBO
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * model.flen() * 9, model.getNorms().data(), GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * model.flen() * 6, model.getTexCoords().data(), GL_STATIC_DRAW);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(2);

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

		return Default3dCache(VAO, &(VBO[0]), tex_id, model.flen());
	}

	virtual void deleteDataCache(Cache cache) const override {
		glDeleteVertexArrays(1, &getVAO(cache));
		glDeleteBuffers(3, getVBO(cache));
		glDeleteTextures(1, &getTexID(cache));
	}

	static bool closerToPlayer(const Scene::light* a, const Scene::light* b) {
		Eigen::Vector3f player_pos = InternalObject::getNamedObjectAs<GameObject>("player")->getPosition()(seq(0,2),3);
		return (a->position - player_pos).norm() < (b->position - player_pos).norm();
	}

public:

	void setAtmosphere(Eigen::Vector3f color, float strength) {
		if (scene_ == nullptr) {
			scene_ = new Scene();
		}
		scene_->atmosphere_color = color;
		scene_->atmosphere_strength = strength;
	}

	void drawObj(const GameObject& obj, Cache cache) const override {
			glBindTexture(GL_TEXTURE_2D, getTexID(cache));
			glBindVertexArray(getVAO(cache));

			glUniform4fv(glGetUniformLocation(gl_id, "overlay_color"), 1, std::get<0>(cache).overlay_color.data());

			//should remove inverse here

			glUniformMatrix4fv(model_location_, 1, GL_FALSE, obj.getPosition().data());
			glDrawArrays(GL_TRIANGLES, 0, 3 * getNElems(cache));


			//glDrawElements(GL_TRIANGLES, 3 * getNElems(cache), GL_UNSIGNED_INT, 0);
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
		glUniform1f(glGetUniformLocation(gl_id, "white_reduction"), scene_->white_reduction);
		glUniform1f(glGetUniformLocation(gl_id, "ambient_light"), scene_->ambient_light);

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
	}

	

	void setScene(Scene* scene) {
		scene_ = scene;
	}

	void setOverlayColor(const GameObject& obj, Eigen::Vector4f color) {
		std::get<0>(getCache(obj)).overlay_color = color;
	}

	Default3d():
		model_location_(glGetUniformLocation(gl_id, "model")),
		camera_location_(glGetUniformLocation(gl_id, "camera")),
		perspective_location_(glGetUniformLocation(gl_id, "perspective")){

		//perspective_ << 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1;
	}

};

#endif
