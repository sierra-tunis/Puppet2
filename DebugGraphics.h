#pragma once

#ifndef PUPPET_GRAPHICS_DEBUGGRAPHICS
#define PUPPET_GRAPHICS_DEGUGGRAPHICS

#include "Hitbox.h"
#include "Graphics.h"
#include "debug_camera.h"


class HboxGraphics : public Graphics<DebugCamera, int, int, size_t, std::vector<float>*> {

	const unsigned int model_location;
	const unsigned int hbox_dims_location;
	//const GraphicsObject base_shape;

	constexpr unsigned int getVAO(Cache cache) const {
		return static_cast<unsigned int>(std::get<0>(cache));
	}

	constexpr unsigned int getColorVBO(Cache cache) const {
		return static_cast<unsigned int>(std::get<1>(cache));
	}

	constexpr unsigned int getN_elems(Cache cache) const {
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

		std::vector<float>* vert_colors = new std::vector<float>;
		vert_colors->reserve(3 * n_verts);
		return Cache{VAO, VBO[1], n_faces,vert_colors};


	}

	void drawObj(const DebugCamera& obj,Cache cache) const override{
		/*if (*obj->collision_flag) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		else {
		}*/
		glBindVertexArray(getVAO(cache));
		glUniformMatrix4fv(model_location, 1, GL_FALSE, obj.getPosition().data());

		//glUniform3fv(hbox_dims_location, 1, obj.getHbox().getShape().data());
		size_t n_verts = obj.getHitbox().getVerts().size();
		for (size_t i = 0; i < n_verts; i++) {
			(*getVertColors(cache))[i] = 0.;
		}
		glBindBuffer(GL_ARRAY_BUFFER, getColorVBO(cache));
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * n_verts *3, getVertColors(cache)->data(), GL_DYNAMIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);

		glDrawElements(GL_TRIANGLES, 3 * getN_elems(cache), GL_UNSIGNED_INT, 0);
	}

	virtual void deleteDataCache(Cache cache) const override{

	}


	void beginDraw() {
		Graphics::beginDraw();
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	}

	void endDraw() {
		Graphics::endDraw();
	}

public:
	HboxGraphics() :model_location(glGetUniformLocation(gl_id, "model")),
		hbox_dims_location(glGetUniformLocation(gl_id, "dims")){
		//base_shape(Model("sphere.obj"), Texture("obamna.jpg"), Eigen::Matrix4f::Identity()){
	}
};
const char* HboxGraphics::vertex_code = "\n"
"#version 330 core\n"
"layout (location = 0) in vec3 pos;\n"
"layout (location = 1) in vec3 color;\n"

"uniform mat4 model;\n"
"uniform vec3 dims;\n"

"void main()\n"
"{\n"
"   gl_Position = model * vec4(pos.x*dims.x, pos.y*dims.y, pos.z*dims.z, 1.0);\n"
"	vert_color = color;"
"}\0";
const char* HboxGraphics::fragment_code = "#version 330 core\n"

"in vec3 vert_color;\n"
"out vec4 FragColor;\n"

"void main()\n"
"{\n"
"	FragColor = vec4(floor(vert_color.x + .01),floor(vert_color.y + .01),floor(vert_color.z + .01),1.f);\n"
" }\0";

#endif