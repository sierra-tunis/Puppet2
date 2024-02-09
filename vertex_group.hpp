#pragma once

#ifndef PUPPET_VERTEXGROUP
#define PUPPET_VERTEXGROUP

#include<vector>
#include<string>
#include<Eigen\Dense>

class VertexGroup {


public:

	struct vertex {
		int index;

		vertex() :index(-1) {}
		vertex(int ind) :index(ind) {}

	};
private:
	std::vector<vertex> vertices_;
	std::string name_;
	const Eigen::Matrix4f* tform_;

	void (*physics_func_)(VertexGroup*, float, void*);
	void* physics_input_;

	void setPhysicsFunction(void (*physics_func)(VertexGroup*, float, void*), void* physics_input) {
		physics_func_ = physics_func;
		physics_input_ = physics_input;
	}

	struct FabricPhysics_info;
	static void FabricPhysics(VertexGroup* vert_group, float dt, void* must_be_FabricPhysics_ptr);

public:

	VertexGroup(std::string name) : name_(name) {

	}

	void setTform(const Eigen::Matrix4f* tform) {
		tform_ = tform;
	}
	const Eigen::Matrix4f* getTform() const {
		return tform_;
	}

	void addVert(int vert_index) {
		vertices_.emplace_back(vert_index);
	}

	const std::vector<vertex>& getVerts() const {
		return vertices_;
	}

	std::string getName() const {
		return name_;
	}

	void setAsFabric(float density, float thickness);

};

#endif