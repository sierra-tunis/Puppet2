#pragma once

#ifndef PUPPET_COLLISIONVISUALIZER
#define PUPPET_COLLISIONVISUALIZER

#include "Graphics.hpp"
#include "GameObject.h"
#include "scene.hpp"

class CollisionVisualizer : public Graphics<CollisionPair<MeshSurface,MeshSurface>,int,int,int,int> {

	const unsigned int perspective_location_;
	const unsigned int camera_location_;
	const unsigned int model_location_;
	const unsigned int color_location_;

	Scene* scene_;

	static Model mesh2Model(const MeshSurface& mesh) {
		Model output;
		for (auto& face : mesh.getFaces()) {
			output.addVert(mesh.getVerts()[std::get<0>(face)]);
			output.addVert(mesh.getVerts()[std::get<1>(face)]);
			output.addVert(mesh.getVerts()[std::get<2>(face)]);
			output.addFace(std::get<0>(face), std::get<1>(face), std::get<2>(face));// i think this should be 1,2,3...4,5,6
		}
		if (output.flen() == 0) { //no faces
			for (auto& edge : mesh.getEdges()) {
				output.addVert(mesh.getVerts()[std::get<0>(edge)]);
				output.addVert(mesh.getVerts()[std::get<1>(edge)]);
				output.addVert(mesh.getVerts()[std::get<0>(edge)]);
				output.addFace(std::get<0>(edge), std::get<1>(edge), std::get<0>(edge));
			}
		}
		output.finalize();
		return output;
	}

	Cache makeDataCache(const CollisionPair<MeshSurface, MeshSurface>& obj) const override {
		Model primary_model = mesh2Model(obj.first);
		Model secondary_model = mesh2Model(obj.second);

		unsigned int VAO[2];
		glGenVertexArrays(2, VAO);
		unsigned int VBO[2];
		glGenBuffers(2, VBO);
		//unsigned int EBO;
		//glGenBuffers(1, &EBO);

		glBindVertexArray(VAO[0]);

		glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * primary_model.flen() * 9, primary_model.getVerts().data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glBindVertexArray(VAO[1]);

		glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * secondary_model.flen() * 9, secondary_model.getVerts().data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);


		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		return std::tuple<int, int,int,int>{VAO[0], primary_model.flen(),VAO[1],secondary_model.flen()};
	};

	void drawObj(const CollisionPair<MeshSurface, MeshSurface>& obj, Cache cache) const {
		Eigen::Vector3f primary_model_color = Eigen::Vector3f(0.0, 1.0, 0.0);
		Eigen::Vector3f primary_model_collision_color = Eigen::Vector3f(1.0, 0.0, 0.0);
		Eigen::Vector3f secondary_model_color = Eigen::Vector3f(0.0, 1.0, 1.0);
		Eigen::Vector3f secondary_model_collision_color = Eigen::Vector3f(1.0, 0.0, 1.0);

		glBindVertexArray(std::get<0>(cache));
		//should remove inverse here

		glUniformMatrix4fv(model_location_, 1, GL_FALSE, obj.getPrimaryPosition().data());
		if (obj.getCollisionInfo().is_colliding) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glUniform3fv(color_location_, 1, primary_model_collision_color.data());
		}
		else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glUniform3fv(color_location_, 1, primary_model_color.data());
		}
		glDrawArrays(GL_TRIANGLES, 0, 3 * std::get<1>(cache));
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		glBindVertexArray(std::get<2>(cache));
		//should remove inverse here

		glUniformMatrix4fv(model_location_, 1, GL_FALSE, obj.getSecondaryPosition().data());
		if (obj.getCollisionInfo().is_colliding) {
			glUniform3fv(color_location_, 1, secondary_model_collision_color.data());
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glUniform3fv(color_location_, 1, secondary_model_color.data());

		}

		glDrawArrays(GL_TRIANGLES, 0, 3 * std::get<3>(cache));
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	}

	void beginDraw() const {
	
		glDisable(GL_DEPTH_TEST);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		glUniformMatrix4fv(perspective_location_, 1, GL_FALSE, scene_->camera->getPerspective().data());
		glUniformMatrix4fv(camera_location_, 1, GL_FALSE, scene_->camera->getCameraMatrix().data());
		
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	}
	void endDraw() const {
		glEnable(GL_DEPTH_TEST);
	}
	
	virtual void deleteDataCache(Cache cache) const override {
		//...
	}

public:
	CollisionVisualizer():model_location_(glGetUniformLocation(gl_id, "model")),
		camera_location_(glGetUniformLocation(gl_id, "camera")),
		perspective_location_(glGetUniformLocation(gl_id, "perspective")),
		color_location_(glGetUniformLocation(gl_id, "color")),
		scene_(nullptr) {

		//perspective_ << 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1;
	}

	void setScene(Scene* scene) {
		scene_ = scene;
	}


};




#endif // !PUPPET_COLLISIONVISUALIZER

