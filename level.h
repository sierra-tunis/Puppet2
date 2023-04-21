#pragma once

#ifndef PUPPET_LEVEL
#define PUPPET_LEVEL

#include <map>

#include "GameObject.h"
#include "Graphics.hpp"
#include "zmap.h"

#include <GLFW/glfw3.h>


using std::array;
using std::vector;

class Level : public GameObject {
private:

	//static std::vector <Level> level_catalog_;

	enum status { active, standby, frozen }; //by default, active is fully loaded and updated, standby is loaded but not updated, frozen is unloaded and unupdated
	std::vector<InternalObject*> contents_;
	GLFWwindow* window_;
	int load_value_;//how "loaded" the level is
	std::string fname;
	std::vector<Level*> neighbors_; //neighbors enter standby when this is active
	Zmap zmap_;

	//we can render the floor like an image with color corresponding to the height.
	// use some sentinel color for the background to indicate out of bounds regions.
	//this lets us rectangularize the whole level and only need to figure out the player position
	//and calculate the height from the "image"
	//only need to take a picture once at the beginning and then it can be saved


	static std::array<int, 2> getWindowSize(GLFWwindow* window) {
		std::array<int, 2> ret;
		glfwGetWindowSize(window, &ret[0], &ret[1]);
		return ret;
	}

public:

	void update() {
		for (auto& c : contents_) {
			c->update(window_);
		}
	}

	Level(std::vector<InternalObject*> layout, GLFWwindow* window, Model* model, Texture* texture, std::string room_name) :
		GameObject(room_name),
		window_(window),
		fname(""),
		zmap_(Level::getWindowSize(window)[1], Level::getWindowSize(window)[0], model)
		//for now this uses current window size as resolution since thats what ZMapper will output as
	{
		setModel(model);
		setTexture(texture);
		for (auto& obj : layout) {
			contents_.push_back(obj);
		}
		moveTo(model->getBoxCenter());
		model->centerVerts();
	}

	//Level(std::string fname, GLFWwindow* window):fname(fname),window_(window) {}

	bool save() const {
		if (fname == "") {/*error*/ }
	}

	const Zmap& getZmap() const {
		return zmap_;
	}

	void initZmap(unsigned int n_steps) {
		std::vector<const Model*> neighbor_models;
		for (auto& neig : neighbors_) {
			neighbor_models.push_back(neig->getModel());
		}
		zmap_.createData(*getModel(), n_steps, neighbor_models);
	}
	/*
	const Model& getModel() const override {
		return this->model_;
	}
	const Texture& getTexture() const override {
		return this->texture_;
	}*/

	std::vector<Level*>& getNeighbors() {
		return neighbors_;
	}

	void add(InternalObject& obj) {
		contents_.push_back(&obj);
	}

	void addNeighbor(Level* neighbor) {
		neighbors_.push_back(neighbor);
	}

	void activate() {
	}
	void deactivate() {
	}

	void activateNeighbor(int neighbor_index) {
		deactivate();
		neighbors_[neighbor_index]->activate();
	}

	const std::vector<InternalObject*>& getContents() const {
		return contents_;
	}


	/*
	void activate() {
		this->load_value_ = active;
		for (auto& n : neighbors_) {
			n->ready();
		}
	}*/
};

#endif