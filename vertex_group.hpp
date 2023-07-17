#pragma once

#ifndef PUPPET_VERTEXGROUP
#define PUPPET_VERTEXGROUP

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

public:

	VertexGroup(std::string name) : name_(name) {

	}

	void setTform(const Eigen::Matrix4f* tform) {
		tform_ = tform;
	}
	const Eigen::Matrix4f* getTform() {
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

};

#endif