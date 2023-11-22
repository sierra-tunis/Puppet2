#pragma once

#ifndef PUPPET_PARAMETRIC3D
#define PUPPET_PARAMETRIC3D

#include "graphics_base.hpp"
#include "ParametricObject.hpp"

template<int n_dofs>
class Parametric3d : public Graphics<ParametricObject<n_dofs>,int,int,size_t,unsigned int, std::vector<float>*, unsigned int, std::vector<float>*> {

	const unsigned int perspective_location_;
	const unsigned int camera_location_;
	const unsigned int model_location_;


	Scene* scene_;

	constexpr int& getVAO(Cache cache) const {
		return std::get<0>(cache);
	}

	constexpr int& getTexID(Cache cache) const {
		return std::get<1>(cache);
	}
	constexpr size_t& getNElems(Cache cache) const {
		return std::get<2>(cache);
	}

	constexpr int& getVAO(Cache cache) const {
		return std::get<0>(cache);
	}

	constexpr int& getTexID(Cache cache) const {
		return std::get<1>(cache);
	}
	constexpr size_t& getNElems(Cache cache) const {
		return std::get<2>(cache);
	}

	constexpr unsigned int& getRotmVBO(Cache cache) const {
		return std::get<3>(cache);
	}
	constexpr std::vector<float>* getRotmMemory(Cache cache) const {
		return std::get<4>(cache);
	}
	constexpr unsigned int& getPosVBO(Cache cache) const {
		return std::get<5>(cache);
	}
	constexpr std::vector<float>* getPosMemory(Cache cache) const {
		return std::get<6>(cache);
	}

	virtual Cache makeDataCache(const ParametricObject<n_dofs>& obj) const override {
		const ParametricModel<n_dofs>* model = obj.getParametricModel();
		const Texture& tex = *(obj.getTexture());

		unsigned int VAO;
		glGenVertexArrays(1, &(VAO));
		unsigned int VBO[5];
		glGenBuffers(5, VBO);

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * model->flen() * 9, model->getVerts().data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, VBO[1]); //size is wrong here! need to change it depending on how we implement norm EBO
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * model->flen() * 9, model->getNorms().data(), GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * model.getTexCoords().size(), model.getTexCoords().data(), GL_STATIC_DRAW);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(2);

		std::vector<float>* vert_rotm = new std::vector<float>(9 * 3 * model.flen());
		vert_rotm->reserve(9 * model.vlen());
		for (int i = 0; i < vert_rotm->size(); i++) {
			(*vert_rotm)[i] = 0.;
		}

		std::vector<float>* vert_pos = new std::vector<float>(3 * 3 * model.flen());
		vert_pos->reserve(3 * model.vlen());
		for (int i = 0; i < vert_pos->size(); i++) {
			(*vert_pos)[i] = 0.;
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

		return Cache{ VAO,tex_id, model.flen(), VBO[3], vert_rotm, VBO[4], vert_pos};
	}


	void drawObj(const GameObject& obj, Cache cache) const override {
		if (!obj.isHidden()) {
			glUniformMatrix4fv(model_location_, 1, GL_FALSE, obj.getPosition().data());

			glBindTexture(GL_TEXTURE_2D, getTexID(cache));
			glBindVertexArray(getVAO(cache));

			glBindBuffer(GL_ARRAY_BUFFER, getRotmVBO(cache));

			obj.getParametricModel()->getRotations(getRotmMemory(cache));
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * obj.getParametricModel()->vlen() * 9, getRotmMemory(cache)->data(), GL_DYNAMIC_DRAW);
			glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(3);

			obj.getParametricModel()->getPositions(getPosMemory(cache));
			glBindBuffer(GL_ARRAY_BUFFER, getPosVBO(cache));
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * obj.getParametricModel()->vlen() * 3, getPosMemory(cache)->data(), GL_DYNAMIC_DRAW);
			glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(4);

			glDrawArrays(GL_TRIANGLES, 0, 3 * getNElems(cache));
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
		if (scene_->lights.size() > 0) {
			glUniform3fv(glGetUniformLocation(gl_id, "light_position"), 1, scene_->lights[0]->position.data());
			glUniform3fv(glGetUniformLocation(gl_id, "light_color"), 1, scene_->lights[0]->color.data());
			glUniform1f(glGetUniformLocation(gl_id, "light_strength"), scene_->lights[0]->brightness);
		}

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		//default3d specific code
	}



};

#endif