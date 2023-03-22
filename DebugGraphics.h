#pragma once
#include "Hitbox.h"
#include "Graphics.h"
#include "GameObject.h"
#include "debug_camera.h"

#ifndef PUPPET_GRAPHICS_DEBUGGRAPHICS
#define PUPPET_GRAPHICS_DEGUGGRAPHICS

class HboxGraphics : public Graphics<DebugCamera, int, int, size_t> {

	const unsigned int model_location;
	const unsigned int hbox_dims_location;
	const GraphicsObject base_shape;

	virtual Cache makeDataCache(const DebugCamera& obj) const override {

	}

	void drawObj(const DebugCamera& obj,Cache cache) const override{
		/*if (*obj->collision_flag) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}*/
		glBindVertexArray(base_shape.VAO);
		glUniformMatrix4fv(model_location, 1, GL_FALSE, obj.getPosition().data());
		glUniform3fv(hbox_dims_location, 1, obj.getHbox().getShape().data());

		glDrawElements(GL_TRIANGLES, 3 * base_shape.n_elems, GL_UNSIGNED_INT, 0);

	}

	virtual Cache makeDataCache(const InternalObject& obj) const const {

	}
	virtual void deleteDataCache(Cache cache) const override{

	}


	void beginDraw() {
		Graphics::beginDraw();
	}

	void endDraw() {
		Graphics::endDraw();
	}

public:
	HboxGraphics() :model_location(glGetUniformLocation(gl_id, "model")), 
		hbox_dims_location(glGetUniformLocation(gl_id, "dims")),
		base_shape(Model("sphere.obj"), Texture("obamna.jpg"), Eigen::Matrix4f::Identity()){}

};
const char* HboxGraphics::vertex_code = "\n"
"#version 330 core\n"
"layout (location = 0) in vec3 pos;\n"

"uniform mat4 model;\n"
"uniform vec3 dims;\n"

"void main()\n"
"{\n"
"   gl_Position = model * vec4(pos.x*dims.x, pos.y*dims.y, pos.z*dims.z, 1.0);\n"
"}\0";
const char* HboxGraphics::fragment_code = "#version 330 core\n"

"out vec4 FragColor;\n"

"void main()\n"
"{\n"
"	FragColor = vec4(1.0f,0.f,0.f,1.f);\n"
" }\0";

#endif