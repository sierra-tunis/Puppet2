#pragma once
#ifndef PUPPET_MATH_SURFACE
#define PUPPET_MATH_SURFACE

#include <Eigen/dense>
#include <iostream>

using Eigen::seq;
//boundaryConstraint -> cant cross specified boundary, motion is adjusted to stay within bounds
//coupleConstraint -> motion is tied to parent motion except for set degrees of freedom

template<int n_dims>
class Surface {
public:
	virtual bool crossesSurface(Eigen::Vector<float, n_dims> first_state, Eigen::Vector<float, n_dims> second_state) const = 0;
};


template<int n_dims>
class Region : public Surface<n_dims> { //Region==bounded surface
public:
	virtual bool insideRegion(Eigen::Vector<float, n_dims> state) const = 0;

	virtual bool crossesSurface(Eigen::Vector<float, n_dims> first_state, Eigen::Vector<float, n_dims> second_state) const override {
		return insideRegion(first_state) != insideRegion(second_state);
	};

};

//hitbox is then a child of region
class MeshSurface : public Surface<3> {
	std::vector<Eigen::Vector3f> verts_;
	std::vector<std::pair<int, int>> edges_;
	std::vector<std::tuple<int, int, int>> faces_;

	virtual bool crossesSurface(Eigen::Vector<float, 3> first_state, Eigen::Vector<float, 3> second_state) const override {
		std::cout << "not implemented to function as primary surface";
		return true;
	}

	struct edgeHasher {
		size_t operator()(const std::pair<int, int>& p) const {
			auto hash1 = std::hash<int>{}(p.first);
			auto hash2 = std::hash<int>{}(p.second);

			if (hash1 != hash2) {
				return hash1 ^ hash2;
			}

			// If hash1 == hash2, their XOR is zero.
			return hash1;
		}
	};

public:
	const std::vector<Eigen::Vector3f>& getVerts() const {
		return verts_;
	}
	const std::vector<std::pair<int, int>>& getEdges() const {
		return edges_;
	}
	const std::vector<std::tuple<int, int, int>>& getFaces() const {
		return faces_;
	}

	explicit MeshSurface(std::string fname);


};

class Ellipse : public Region<3> {//technially generalizable into N dimensions but not useful for making games lol
	const Eigen::Vector3f E_;
	const Eigen::Vector3f E_inv_sq_;
public:
	bool insideRegion(Eigen::Vector<float, 3> state) const {
		return state.array().pow<int>(2).matrix().transpose() * E_inv_sq_;
	};

};

/*
crazy chatgpt moment: it recommended changing "surface" to "boundary " without knowing about BoundaryConstraint, namely "const Surface<3>* boundary_;"
*/

#endif