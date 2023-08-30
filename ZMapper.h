#pragma once


#ifndef PUPPET_ZMAPPER
#define PUPPET_ZMAPPER


#include <cmath>

#include "math_constants.hpp"
#include "Graphics.hpp"
#include "camera.h"
#include "zdata.hpp"

//An alternative, non-zmap based approach would be breaking up the level into large voxels,
// each voxel contains an array of faces to check collisions on. this way the collision checking is
// o(rho) where rho is the average face per voxel density. each collision check needs to be done per vertex.
// each vertex is either concave or convex (since they are triangles) based on face orientation.
// concvex points are simple to compute (on the correct side of the column extruded from of all 3 faces) 
// so simply check for entering convec parts and put it in a queue, then when you leave it, check if it is
// passing through the open face and if it is, then remove it from the queue, otherwise signal a collision. 

// we can also take a hybrid approach where we determine the z height using the zmap and then adjust using collisions
// in fact collision can be tied to hitbox. a primary hitbox would be a mesh based hitbox and then the secondary hitbox would be
// a primitive with easy to compute collisions and get precise contact positioning. zmap can also be used for true out of bounds
// information that supercedes mesh collisions

class ZMapper : public Graphics<GameObject, int, size_t, uint8_t> { //note zmap is really the y direction in opengl, however Z usually represents the height dimension
private:
	//const float step_height_; //data above step height will be clipped leaving only the background (cannot move onto background)
								//in most cases this is "step height" i.e. the maximum height a player can step over small discontinuities
								//however in some cases you may want the whole stage to be measured (i.e. no Z overlapping)
	//const float partition_height_;// if we represent the zmap as an array of lists, we can store all the zdata for a whole stage as
							//one object. then to lookup the z height it just checks values at that location until the 
							//last one with a height less than current height+step height is found and returns that one
							//this would be O(n_overlaps) 
							//then before the character moves, simply check that the new position is valid otherwise dont move
							//(or move to a position that is valid)

	const unsigned int camera_location_;
	const unsigned int position_location_;
	const unsigned int ZClip_location_;
	const unsigned int room_id_location_;
	static uint8_t last_room_id_;

	Camera camera_; //must always be pointing down (R = R_x(90))
	Eigen::Matrix4f ZClip_;

	unsigned int FBO_;

	constexpr int& getVAO(Cache cache) const {
		return std::get<0>(cache);
	}

	constexpr size_t& getNElems(Cache cache) const {
		return std::get<1>(cache);
	}

	constexpr uint8_t& getRoomID(Cache cache) const {
		return std::get<2>(cache);
	}

	/*
	constexpr float& getHeight(Cache cache) const {
		return std::get<2>(cache);
	}

	constexpr float& getWidth(Cache cache) const {
		return std::get<3>(cache);
	}*/

	void makeZclip(int y_resolution, int x_resolution, float height, float width,float y_center, float x_center, float z_step, float padding) {
		
		ZClip_ = (Eigen::Matrix4f()<<2. / (width*(1.+padding)), 0.0, 0.0, 0.,
				0.0, 2. / (height*(1.+padding)), 0.0, 0.,
				0.0, 0.0, 2.0 / z_step, -1.,
				0.0, 0.0, 0.0, 1.0).finished();

	}


	//copy pasted from default3d (i.e. bad practice)
	virtual Cache makeDataCache(const GameObject& obj) const override {
		const Model& model = *obj.getModel();
		unsigned int VAO;
		glGenVertexArrays(1, &(VAO));
		//this->VAO = static_cast<int>(VAO);
		unsigned int VBO[2];
		glGenBuffers(2, VBO);
		unsigned int EBO;
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * model.vlen() * 3, model.getVerts().data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, VBO[1]); //size is wrong here! need to change it depending on how we implement norm EBO
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * model.getNorms().size(), model.getNorms().data(), GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);//have to change this to face length not vertex len
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * model.flen() * 3, model.getFaces().data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		//return std::tuple<int, size_t, float, float>{VAO, model.flen(), model.getBoundingBox()[0], model.getBoundingBox()[2]};
		return Cache{VAO, model.flen(), last_room_id_++,};
	}

	virtual void deleteDataCache(Cache cache) const override {

	}

	void clearCache() {
		cached_data_.clear();
		last_room_id_ = 1;
	}

	void drawObj(const GameObject& obj, Cache cache) const override {
		glBindVertexArray(getVAO(cache));
		glUniform1f(room_id_location_, static_cast<float>(getRoomID(cache))/256.);
		glUniformMatrix4fv(position_location_, 1, GL_FALSE,obj.getPosition().data());
		//glUniformMatrix4fv(position_location_, 1, GL_FALSE, Matrix4f( Matrix4f::Identity()).data());

		float tmp = static_cast<float>(getRoomID(cache)) / 256.;
		//should remove inverse here

		glDrawElements(GL_TRIANGLES, 3 * getNElems(cache), GL_UNSIGNED_INT, 0);
		//for (auto const& o : obj.getChildren()) {
		//	draw(*o);
		//}
	}

	void beginDraw() const override {

		glEnable(GL_DEPTH_TEST);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		//glEnable(GL_CULL_FACE);
		//glCullFace(GL_BACK);


		glUniformMatrix4fv(ZClip_location_, 1, GL_FALSE, ZClip_.data());
		glUniformMatrix4fv(camera_location_, 1, GL_FALSE, Matrix4f(camera_.getPosition().inverse()).data());


		//default3d specific code
	}

	void endDraw() const override {
		//default3d specific code

		glDisable(GL_CULL_FACE);
		//glCullFace(GL_BACK);

	}

public:

	ZMapper() :camera_(Camera("zmapper camera")),
		camera_location_(glGetUniformLocation(gl_id, "camera")),
		ZClip_location_(glGetUniformLocation(gl_id, "ZClip")),
		room_id_location_(glGetUniformLocation(gl_id,"room_id")),
		position_location_(glGetUniformLocation(gl_id, "model_position"))
	{
		camera_.rotateX(M_PI / 2);
	}

	bool renderZstep(const GameObject& level, int y_resolution, int x_resolution, float z, float z_step,float xy_padding, std::vector<uint8_t>* data, const std::vector<const GameObject*>& neighbors,std::string fname) {
		constexpr int n_channels = 4;
		const Model& model = *level.getModel();
		//camera centers on model (primary model)
		makeZclip(y_resolution, x_resolution, model.getBoundingBox()[2], model.getBoundingBox()[0], level.getPosition()(2,3), level.getPosition()(0, 3), z_step, xy_padding);
		int i = 0;
		add(level);
		for (const auto& neig : neighbors) {
			add(*neig);
		}
		camera_.moveTo(level.getPosition()(seq(0,2), 3));
		camera_.translate(0, z, 0);
		//create zclip matrix
		startScreenshot(x_resolution, y_resolution);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0., 0., 0., 0.);
		Eigen::Matrix4f mat = Matrix4f(camera_.getPosition().inverse()) * level.getPosition();
		beginDraw();
		drawAll();
		endDraw();
		//...see https://stackoverflow.com/questions/12157646/how-to-render-offscreen-on-opengl
		data->reserve(y_resolution * x_resolution * n_channels);
		//finishScreenshot<uint8_t, GL_UNSIGNED_BYTE>(data,"C:\\Users\\Justin\\source\\repos\\Puppet2\\Puppet2" + fname);
		finishScreenshot<uint8_t, GL_UNSIGNED_BYTE>(data);

		unload(level);
		for (const auto& neig : neighbors) {
			unload(*neig);
		}
		last_room_id_ = 1;

		return true; //here in case I want to do checks on if the data was transfered. should also be [[nodiscard]]
	}
	/*
	Zmap createZmap(Model model,int height, int width, float z_step,zdata background_fill) {
		Zmap(height,width,model_bbox[2],model_bbox[0], background_fill);
		
		//this->remove(model);
	}
	*/

};

#endif