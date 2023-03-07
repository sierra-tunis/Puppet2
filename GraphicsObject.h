#pragma once
#include <Eigen\Dense>
#include "Model.h"
#include "Texture.h"
#include "Hitbox.h"
#include <glad/glad.h>

#ifndef PUPPET_GRAPHICSOBJECT
#define PUPPET_GRAPHICSOBJECT

/*template <class... Refs>
struct Observer {
	std::tuple<const Refs...&> references;
	static std::tuple<Refs...> default_values;

	Observer() : references(default_values) {}
	Observer(const Refs...& refs) : references(refs) {}
};*/

struct GraphicsObject {
	int VAO;
	int tex_id;//might be possible to combine with VAO?
	size_t n_elems;
	const Eigen::Matrix4f& position;

	void initialize(const Model& model, const Texture& tex) {
		//model code:
		this->n_elems = model.flen();
		unsigned int VAO;
		glGenVertexArrays(1, &(VAO));
		this->VAO = static_cast<int>(VAO);
		unsigned int VBO[3];
		glGenBuffers(3, VBO);
		unsigned int EBO;
		glGenBuffers(1, &EBO);

		glBindVertexArray(this->VAO);

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
		this->tex_id = static_cast<int>(tex_id);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex.width, tex.height, 0, GL_RGB, GL_UNSIGNED_BYTE, tex.getData().data());
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	GraphicsObject() :VAO(-1), tex_id(-1), n_elems(0), position(Eigen::Matrix4f::Identity()) {}

	explicit GraphicsObject(Eigen::Matrix4f& position):position(position){}

	GraphicsObject(const Model& model, const Texture& tex, const Eigen::Matrix4f& pos):
	position(pos){
		initialize(model, tex);
	};

	explicit GraphicsObject(const Eigen::Matrix4f& pos) : position(pos),VAO(-1),tex_id(-1),n_elems(0) {}

	bool isInitialized() {
		return (VAO != -1)&&(tex_id!=-1);
	}
 };

struct Grobj_blinPhong : public GraphicsObject {
	float ka, kd, ks;
	Grobj_blinPhong(Model model, Texture tex, const Eigen::Matrix4f& pos):GraphicsObject(model,tex,pos) {}
	explicit Grobj_blinPhong(const Eigen::Matrix4f& pos) :GraphicsObject(pos) {}
};

struct HboxGrobj : public GraphicsObject {
	const Hitbox* hbox;
	const bool* collision_flag;

	HboxGrobj(const Eigen::Matrix4f& pos, const Hitbox* hbox,const bool* collision_flag):
	GraphicsObject(pos),
	hbox(hbox),
	collision_flag(collision_flag){}
};

#endif