#pragma once

#define _USE_MATH_DEFINES
#include <cmath>

#include "Graphics.h"
#include "InternalObject.h"
#include "camera.h"


#ifndef PUPPET_GRAPHICS_ZMAPPER
#define PUPPET_GRAPHICS_ZMAPPER

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

struct zdata {
	float z, x_slope, y_slope;
	int room_id; //room_id is the alpha bit and is transparent from the background. 0 is reserved for inaccessible
	zdata(std::array<float, 3> data, int room_id):z(data[0]),x_slope(data[1]),y_slope(data[2]),room_id(room_id) {}
	static zdata floor() {
		return zdata({ -INFINITY,0,0 }, 0);
	}
	static zdata ceiling() {
		return zdata({ INFINITY,0,0 }, 0);
	}

	bool operator!=(const zdata& other) {
		return (this->z != other.z || this->x_slope != other.x_slope || this->y_slope != other.y_slope);
	}

	static constexpr int BaseRoom = 0;

};

class ZMapper : public Graphics<Model, int, size_t, float, float> { //note zmap is really the y direction in opengl, however Z usually represents the height dimension
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
	const unsigned int ZClip_location_;

	InternalObject camera_; //must always be pointing down (R = R_x(90))
	Eigen::Matrix4f ZClip_;

	unsigned int FBO_;

	constexpr int& getVAO(Cache cache) const {
		return std::get<0>(cache);
	}

	constexpr size_t& getNElems(Cache cache) const {
		return std::get<1>(cache);
	}

	constexpr float& getHeight(Cache cache) const {
		return std::get<2>(cache);
	}

	constexpr float& getWidth(Cache cache) const {
		return std::get<3>(cache);
	}

	void makeZclip(int y_resolution, int x_resolution, float height, float width,float y_center, float x_center, float z_step) {
		
		ZClip_ << (Eigen::Matrix4f()<<2. / width, 0.0, 0.0, 0.,
				0.0, 2. / height, 0.0, 0.,
				0.0, 0.0, 1.0 / z_step, 0.0,
				0.0, 0.0, 0.0, 1.0).finished() * 
			(Eigen::Matrix4f() << 1., 0.0, 0.0, -x_center,
				0.0, 1., 0.0, -y_center,
				0.0, 0.0, 1.0, 0.0,
				0.0, 0.0, 0.0, 1.0).finished();
	}


	//copy pasted from default3d (i.e. bad practice)
	virtual std::tuple<int, size_t, float, float> makeDataCache(const Model& obj) const override {
		const Model& model = obj;
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

		return std::tuple<int, size_t, float, float>{VAO, model.flen(), model.getBoundingBox()[0], model.getBoundingBox()[2]};
	}

	virtual void deleteDataCache(Cache cache) const override {

	}

	void drawObj(const Model& obj, Cache cache) const override {
		glBindVertexArray(getVAO(cache));
		//should remove inverse here

		glDrawElements(GL_TRIANGLES, 3 * getNElems(cache), GL_UNSIGNED_INT, 0);
		//for (auto const& o : obj.getChildren()) {
		//	draw(*o);
		//}
	}

	void beginDraw() const override {

		Graphics::beginDraw();


		glEnable(GL_DEPTH_TEST);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		//glEnable(GL_CULL_FACE);
		//glCullFace(GL_BACK);


		glUniformMatrix4fv(ZClip_location_, 1, GL_FALSE, ZClip_.data());
		glUniformMatrix4fv(camera_location_, 1, GL_FALSE, Matrix4f(camera_.getPosition().inverse()).data());


		//default3d specific code
	}

	void endDraw() const override {
		Graphics::endDraw();
		//default3d specific code

		glDisable(GL_CULL_FACE);
		//glCullFace(GL_BACK);

	}

public:

	ZMapper() :camera_(Camera("zmapper camera")),
		camera_location_(glGetUniformLocation(gl_id, "camera")),
		ZClip_location_(glGetUniformLocation(gl_id, "ZClip")) {

		camera_.rotateX(M_PI / 2);
	}

	bool renderZstep(Model& model, int y_resolution, int x_resolution, float z, float z_step,std::vector<uint8_t>* data) {
		makeZclip(y_resolution, x_resolution, model.getBoundingBox()[2], model.getBoundingBox()[0], model.getBoxCenter()[2], model.getBoxCenter()[0], z_step);
		draw_targets_.insert({ 0, &model });
		if (cached_data_.find(0) == cached_data_.end()) {
			cached_data_.insert({ 0,makeDataCache(model) });
		}		
		camera_.moveTo(0, z, 0);
		camera_.updatePosition();
		//create zclip matrix
		startScreenshot(x_resolution, y_resolution);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0., 0., 0., 0.);
		beginDraw();
		drawAll();
		endDraw();
		//...see https://stackoverflow.com/questions/12157646/how-to-render-offscreen-on-opengl
		data->reserve(y_resolution * x_resolution * 4);
		//finishScreenshot<int, GL_INT>(data,"zmap_layer.png");
		finishScreenshot<uint8_t, GL_UNSIGNED_BYTE>(data, "zmap" + std::to_string(z) + "_layer.png");
		//finishScreenshot<uint8_t, GL_UNSIGNED_BYTE>(data);

		draw_targets_.erase(0);
		return true; //here in case I want to do checks on if the data was transfered. should also be [[nodiscard]]
	}
	/*
	Zmap createZmap(Model model,int height, int width, float z_step,zdata background_fill) {
		Zmap(height,width,model_bbox[2],model_bbox[0], background_fill);
		
		//this->remove(model);
	}
	*/

};

const char* ZMapper::vertex_code = "\n"
"#version 330 core\n"
"layout (location = 0) in vec3 pos;\n"
"layout (location = 1) in vec3 norm;\n"

"uniform mat4 camera;\n"
"uniform mat4 ZClip;"

"out vec4 Zdata;\n"

"void main()\n"
"{\n"
"	vec4 position = ZClip * camera * vec4(pos.x, pos.y, pos.z, 1.0);\n"//no model matrix since model does not move
"   gl_Position = position;\n"
"	Zdata = vec4(1.-position.z,norm.x/2.+.5,norm.z/2.+.5,1.0);"//red is Z height, green is y slope, blue is x slope, alpha is unused (for now)
"}\0";

const char* ZMapper::fragment_code = "#version 330 core\n"
"in vec4 Zdata;\n "

"out vec4 FragColor;\n"

"void main()\n"
"{\n"
"	FragColor = Zdata;\n"
" } ";
#endif