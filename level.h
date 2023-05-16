#pragma once

#ifndef PUPPET_LEVEL
#define PUPPET_LEVEL

#define _USE_MATH_DEFINES
#include <map>

#include "GameObject.h"
#include "Graphics.hpp"
#include "zmap.h"
#include "ZMapper.h"
#include "sound.hpp"

#include <GLFW/glfw3.h>


using std::array;
using std::vector;

class Level : public GameObject {
private:

	//static std::vector <Level> level_catalog_;

	enum LoadStatus { active, standby, frozen }; //by default, active is fully loaded and updated, standby is loaded but not updated, frozen is unloaded and unupdated
	std::vector<GameObject*> contents_;
	GLFWwindow* window_;
	LoadStatus load_state_;//how "loaded" the level is
	std::string fname;
	std::vector<Level*> neighbors_; //neighbors enter standby when this is active
	std::vector<const Level*> const_neighbors_;
	Surface<3>* collision_surface_;
	Region<3>* level_region_;
	const int level_number_;

	static Level* current_level_;
	static Level* prev_level_;
	static std::vector<Level*> all_levels_;

	Sound theme_;

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

	void enterStandby() {
		if (load_state_ == active) {
			deactivate();
		}
		load_state_ = standby;
		theme_.load();

	}

	void activate() {
		if (load_state_ == frozen) {
			enterStandby(); // cant go from frozen straight to active
		}
		load_state_ = active;
		for (auto& obj : contents_) {
			obj->onRoomActivation();
		}
		theme_.play();
	}

	void freeze() {
		if (load_state_ == active) {
			deactivate();
		}
		load_state_ = frozen;
		theme_.unload();
	}

	void deactivate() {
		load_state_ = standby;
		for (auto& obj : contents_) {
			obj->onRoomActivation();
		}
		theme_.stop();
	}

public:

	static Level* getCurrentLevel() {
		return current_level_;
	}

	static void UpdateCurrentLevel(GLFWwindow* window) {
		current_level_->update(window);
	}
	
	void goToNeighbor(int neighbor_ind) {
		Level::goToLevel(neighbors_[neighbor_ind]);
	}

	void goToNeighbor(const Level* neighbor) {
		auto new_level = std::find(neighbors_.begin(), neighbors_.end(), neighbor);
		if (new_level == neighbors_.end()) {
			std::cerr << "not a neighbor of this level!";
			return;
		}
		Level::goToLevel(*new_level);
	}

	static void goToLevel(Level* new_level) {
		prev_level_ = current_level_;
		current_level_ = new_level;
		new_level->activate();
		for (auto& neig : new_level->neighbors_) {
			neig->enterStandby();
		}
		if (prev_level_ != nullptr) {
			const std::vector<Level*>& neighbors = prev_level_->neighbors_;
			for (auto& neig : neighbors) {
				if (std::find(neighbors.begin(), neighbors.end(), neig) == neighbors.end()) {
					neig->freeze();
				}
			}
		}
	}

	static void goToLevel(int level_num) {
		goToLevel(all_levels_[level_num]);
	}

	static void incrementLevel() {
		if (Level::getCurrentLevel() == nullptr) {
			Level::goToLevel(0);
		} else {
			int next_level = Level::getCurrentLevel()->getLevelNum() + 1;
			if (next_level < all_levels_.size()) {
				goToLevel(all_levels_[next_level]);
			} else {
				Level::goToLevel(0);
			}
		}
	}

	static void decrementLevel() {
		if (Level::getCurrentLevel() == nullptr) {
			Level::goToLevel(0);
		}
		else {
			int next_level = Level::getCurrentLevel()->getLevelNum() - 1;
			if (next_level >= 0) {
				goToLevel(all_levels_[next_level]);
			}
			else {
				Level::goToLevel(all_levels_.size()-1);
			}
		}
	}

	static void goToPrevLevel() {
		Level::goToLevel(prev_level_);
	}

	void onStep() override {
		for (auto& c : contents_) {
			c->update(window_);
		}
	}

	Level(std::vector<GameObject*> layout, GLFWwindow* window, Model* model, Texture* texture, std::string room_name) :
		GameObject(room_name),
		load_state_(frozen),
		window_(window),
		fname(""),
		collision_surface_(nullptr),
		level_number_(all_levels_.size())
		//for now this uses current window size as resolution since thats what ZMapper will output as
	{
		all_levels_.push_back(this);
		setModel(model);
		setTexture(texture);
		for (auto& obj : layout) {
			contents_.push_back(obj);
		}
		moveTo(model->getBoxCenter());
		model->centerVerts();
		for (auto& neig : neighbors_) {
			const_neighbors_.push_back(neig);
		}
	}

	//Level(std::string fname, GLFWwindow* window):fname(fname),window_(window) {}

	bool save() const {
		if (fname == "") {/*error*/ }
	}

	const Surface<3>* getCollisionSurface() const {
		return collision_surface_;
	}
	
	void createZmapCollisionSurface(unsigned int n_steps, ZMapper* zmapper) {
		std::vector<const GameObject*> neighbors;
		for (const auto& neig : neighbors_) {
			neighbors.push_back(neig);
		}
		Zmap* collision_surface = new Zmap(Level::getWindowSize(window_)[1], Level::getWindowSize(window_)[0], getModel());
		collision_surface->createData(*this, n_steps, neighbors, static_cast<void*>(zmapper));
		collision_surface_ = collision_surface;
		level_region_ = collision_surface;
	}
	/*
	const Model& getModel() const override {
		return this->model_;
	}
	const Texture& getTexture() const override {
		return this->texture_;
	}*/

	const std::vector<const Level*>& getNeighbors() const {
		return const_neighbors_;
	}

	bool withinLevel(Eigen::Vector3f pos) const {
		return level_region_->insideRegion(pos-getPosition()(seq(0,2),3));
	}

	int neighborAt(Eigen::Vector3f pos) const {
		for (int i = 0; i < neighbors_.size(); i++) {//could be improved
			if (neighbors_[i]->withinLevel(pos)) {
				return i;
			}
		}
		return -1;
	}

	void add(GameObject& obj) {
		contents_.push_back(&obj);
	}

	void addNeighbor(Level* neighbor) {
		neighbors_.push_back(neighbor);
		const_neighbors_.push_back(neighbor);
	}

	/*
	void activateNeighbor(Level* neighbor) {
		deactivate();
		neighbor->activate();
	}*///not sure which version I like more 
	/*
	void activateNeighbor(int neighbor_index) {
		deactivate();
		neighbors_[neighbor_index]->activate();
	}*/

	const std::vector<GameObject*>& getContents() const {
		return contents_;
	}

	int getLevelNum() const {
		return level_number_;
	}

	void setTheme(Sound theme) {
		theme_ = theme;
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