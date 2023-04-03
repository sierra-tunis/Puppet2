#pragma once

#ifndef PUPPET_GRAPHICS_DEBUGGRAPHICS
#define PUPPET_GRAPHICS_DEGUGGRAPHICS

#include "Hitbox.h"
#include "Graphics.h"
#include "debug_camera.h"

/*
class HboxGraphics : public Graphics<DebugCamera, int, int, size_t, std::vector<float>*> {

	const unsigned int model_location_;
	const unsigned int perspective_location_;
	const unsigned int camera_location_;

	//const GraphicsObject base_shape;

	const InternalObject& camera_;
	Eigen::Matrix4f perspective_;

	constexpr unsigned int getVAO(Cache cache) const {
		return static_cast<unsigned int>(std::get<0>(cache));
	}

	constexpr unsigned int getColorVBO(Cache cache) const {
		return static_cast<unsigned int>(std::get<1>(cache));
	}

	constexpr unsigned int getNElems(Cache cache) const {
		return static_cast<unsigned int>(std::get<2>(cache));
	}
	constexpr std::vector<float>* getVertColors(Cache cache) const {
		return std::get<3>(cache);
	}


	virtual Cache makeDataCache(const DebugCamera& obj) const override {
		const MeshSurface& mesh = obj.getHitbox();
		// = model.flen();
		unsigned int VAO;
		glGenVertexArrays(1, &(VAO));
		//this->VAO = static_cast<int>(VAO);
		unsigned int VBO[2];
		glGenBuffers(2, VBO);
		unsigned int EBO;
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
		std::vector<float> mesh_verts;
		int n_verts = mesh.getVerts().size();
		for (int i = 0; i < n_verts; i++) {
			const Eigen::Vector3f& vert = mesh.getVerts()[i];
			mesh_verts.push_back(vert[0]);
			mesh_verts.push_back(vert[1]);
			mesh_verts.push_back(vert[2]);
		}
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * n_verts * 3,mesh_verts.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);


		std::vector<float>* vert_colors = new std::vector<float>(3 * n_verts);
		vert_colors->reserve(3 * n_verts);

		//size_t n_verts = obj.getHitbox().getVerts().size();
		for (size_t i = 0; i < n_verts * 3; i++) {
			(*vert_colors)[i] = 0.;
		}
		glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * n_verts * 3, vert_colors->data(), GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);


		std::vector<unsigned int> mesh_faces;
		int n_faces = mesh.getFaces().size();
		for (int i = 0; i < n_faces; i++) {
			const std::tuple<int, int, int>& face = mesh.getFaces()[i];
			mesh_faces.push_back(static_cast<unsigned int>(std::get<0>(face)));
			mesh_faces.push_back(static_cast<unsigned int>(std::get<1>(face)));
			mesh_faces.push_back(static_cast<unsigned int>(std::get<2>(face)));
		}
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);//have to change this to face length not vertex len
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * n_faces * 3, mesh_faces.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		return Cache{VAO, VBO[1], n_faces,vert_colors};
	}

	void drawObj(const DebugCamera& obj,Cache cache) const override{
		glBindVertexArray(getVAO(cache));
		glUniformMatrix4fv(model_location_, 1, GL_FALSE, obj.getPosition().data());

		
		//size_t n_verts = obj.getHitbox().getVerts().size();
		//for (size_t i = 0; i < n_verts * 3; i++) {
		//	(*getVertColors(cache))[i] = 0.;
		//}
		//glBindBuffer(GL_ARRAY_BUFFER, getColorVBO(cache));
		//glBufferData(GL_ARRAY_BUFFER, sizeof(float) * n_verts *3, getVertColors(cache)->data(), GL_DYNAMIC_DRAW);
		//glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		//glEnableVertexAttribArray(1);
		
		glDrawElements(GL_TRIANGLES, 3 * getNElems(cache), GL_UNSIGNED_INT, 0);
	}

	virtual void deleteDataCache(Cache cache) const override{

	}


	void beginDraw() {
		Graphics::beginDraw();
		glEnable(GL_DEPTH_TEST);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glUniformMatrix4fv(perspective_location_, 1, GL_FALSE, perspective_.data());
		glUniformMatrix4fv(camera_location_, 1, GL_FALSE, Matrix4f(camera_.getPosition().inverse()).data());

	}

	void endDraw() {
		Graphics::endDraw();
	}

public:
	HboxGraphics(const Camera& camera, float near_clip, float far_clip, float fov) :
		model_location_(glGetUniformLocation(gl_id, "model")),
		camera_location_(glGetUniformLocation(gl_id, "camera")),
		camera_(camera),
		perspective_location_(glGetUniformLocation(gl_id, "perspective")){

		float S = 1. / (tan(fov / 2. * M_PI / 180.));
		perspective_ << S, 0, 0, 0,
			0, S, 0, 0,
			0, 0, -(far_clip) / (far_clip - near_clip), -2 * far_clip * near_clip / (far_clip - near_clip),
			0, 0, -1., 0;
		//base_shape(Model("sphere.obj"), Texture("obamna.jpg"), Eigen::Matrix4f::Identity()){
	}
};
const char* HboxGraphics::vertex_code = "\n"
"#version 330 core\n"
"layout (location = 0) in vec3 pos;\n"
"layout (location = 1) in vec3 color;\n"

"uniform mat4 perspective;\n"
"uniform mat4 camera;\n"
"uniform mat4 model;\n"

"out vec3 vert_color;\n"

"void main()\n"
"{\n"
"   gl_Position = perspective * camera * model * vec4(pos.x, pos.y, pos.z, 1.0);\n"
"	vert_color = color;"
"}\0";

const char* HboxGraphics::fragment_code = "#version 330 core\n"

"in vec3 vert_color;\n"
"out vec4 FragColor;\n"

"void main()\n"
"{\n"
"	FragColor = vec4(floor(vert_color.x + .01),floor(vert_color.y + .01),floor(vert_color.z + .01),1.f);\n"
" }\0";
*/ 


using Eigen::Matrix4f;

class HboxGraphics : public Graphics<DebugCamera, int, int, size_t,std::vector<float>*> { //VAO, tex_id, n_elems

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

	constexpr unsigned int getColorVBO(Cache cache) const {
		return static_cast<unsigned int>(std::get<1>(cache));
	}
	constexpr size_t& getNElems(Cache cache) const {
		return std::get<2>(cache);
	}
	constexpr std::vector<float>* getVertColors(Cache cache) const {
		return std::get<3>(cache);
	}

	virtual Cache makeDataCache(const DebugCamera& obj) const override {
		const MeshSurface& mesh = obj.getHitbox();
		// = model.flen();
		unsigned int VAO;
		glGenVertexArrays(1, &(VAO));
		//this->VAO = static_cast<int>(VAO);
		unsigned int VBO[2];
		glGenBuffers(2, VBO);
		unsigned int EBO;
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
		std::vector<float> mesh_verts;
		int n_verts = mesh.getVerts().size();
		for (int i = 0; i < n_verts; i++) {
			const Eigen::Vector3f& vert = mesh.getVerts()[i];
			mesh_verts.push_back(vert[0]);
			mesh_verts.push_back(vert[1]);
			mesh_verts.push_back(vert[2]);
		}
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * n_verts * 3, mesh_verts.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);


		std::vector<float>* vert_colors = new std::vector<float>(3 * n_verts);
		vert_colors->reserve(3 * n_verts);
		for (int i = 0; i < vert_colors->size(); i++) {
			(*vert_colors)[i] = 0.;
		}

		//size_t n_verts = obj.getHitbox().getVerts().size();
		

		std::vector<unsigned int> mesh_edges;
		int n_edges = mesh.getEdges().size();
		for (int i = 0; i < n_edges; i++) {
			const std::pair<int, int>& edge = mesh.getEdges()[i];
			mesh_edges.push_back(static_cast<unsigned int>(std::get<0>(edge)));
			mesh_edges.push_back(static_cast<unsigned int>(std::get<1>(edge)));
		}
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);//have to change this to face length not vertex len
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * n_edges * 2, mesh_edges.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		return Cache{ VAO, VBO[1], n_edges,vert_colors };
	}

	virtual void deleteDataCache(Cache cache) const override {
		//...
	}

public:

	void drawObj(const DebugCamera& obj, Cache cache) const override {
		glBindVertexArray(getVAO(cache));
		//should remove inverse here



		glUniformMatrix4fv(model_location_, 1, GL_FALSE, obj.getPosition().data());

		int n_verts = obj.getHitbox().getVerts().size();
		int n_edges = obj.getHitbox().getEdges().size();
		for (size_t i = 0; i < n_edges; i++) {
			//(*getVertColors(cache))[3*i+j] += 0.3/static_cast<float>(n_verts)*static_cast<float>(i)*obj.getdt();
			//(*getVertColors(cache))[3*i+j] = fmod((*getVertColors(cache))[i], 1.);
			std::pair<int, int> edge = obj.getHitbox().getEdges()[i];
			if (obj.getCollisionInfo()[i]) {
				(*getVertColors(cache))[3 * std::get<0>(edge)] = 1.;
				(*getVertColors(cache))[3 * std::get<1>(edge)] = 1.;
			} else {
				(*getVertColors(cache))[3 * std::get<0>(edge)] = 0.;
				(*getVertColors(cache))[3 * std::get<1>(edge)] = 0.;
			}
		}
		glBindBuffer(GL_ARRAY_BUFFER, getColorVBO(cache));
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * n_verts * 3, getVertColors(cache)->data(), GL_DYNAMIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);

		glDrawElements(GL_LINES, 2 * getNElems(cache), GL_UNSIGNED_INT, 0);
		//for (auto const& o : obj.getChildren()) {
		//	draw(*o);
		//}
	}

	void beginDraw() const override {
		Graphics::beginDraw();
		glEnable(GL_DEPTH_TEST);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glLineWidth(3.);

		glUniformMatrix4fv(perspective_location_, 1, GL_FALSE, perspective_.data());
		glUniformMatrix4fv(camera_location_, 1, GL_FALSE, Matrix4f(camera_.getPosition().inverse()).data());
		//default3d specific code
	}

	void endDraw() const override {
		Graphics::endDraw();
		//default3d specific code
	}

	HboxGraphics(const Camera& camera, float near_clip, float far_clip, float fov) :
		model_location_(glGetUniformLocation(gl_id, "model")),
		camera_location_(glGetUniformLocation(gl_id, "camera")),
		perspective_location_(glGetUniformLocation(gl_id, "perspective")),
		camera_(camera),
		near_clip_(near_clip),
		far_clip_(far_clip),
		fov_(fov) {

		float S = 1. / (tan(fov_ / 2. * M_PI / 180.));
		perspective_ << S, 0, 0, 0,
			0, S, 0, 0,
			0, 0, -(far_clip_) / (far_clip_ - near_clip_), -2 * far_clip_ * near_clip_ / (far_clip_ - near_clip_),
			0, 0, -1., 0;
		//perspective_ << 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1;
	}

};
const char* HboxGraphics::vertex_code = "\n"
"#version 330 core\n"
"layout (location = 0) in vec3 pos;\n"
"layout (location = 1) in vec3 color;\n"

"uniform mat4 perspective;\n"
"uniform mat4 camera;\n"
"uniform mat4 model;\n"

"out vec3 vert_color;\n"

"void main()\n"
"{\n"
"	vert_color = color;\n"
"   gl_Position = perspective * camera * model * vec4(pos.x, pos.y, pos.z, 1.0);\n"
"}\0";
const char* HboxGraphics::fragment_code = "#version 330 core\n"
"in vec3 vert_color;\n "

"out vec4 FragColor;\n"

"void main()\n"
"{\n"
"	FragColor = vec4(vert_color.x,vert_color.y,vert_color.z,1.);\n"
//"	FragColor = vec4(0.,0.,0.,1.);\n"
" } ";

#endif