#pragma once
#ifndef PUPPET_MATH_SURFACE
#define PUPPET_MATH_SURFACE

#include <Eigen/dense>
#include <iostream>
#include <tuple>

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
	std::vector<Eigen::Vector3f> face_norms_;

	Eigen::Vector3f box_center_;
	Eigen::Vector3f bounding_box_;

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

	void calculateBoundingBox() {
		std::array<float, 3> min = { INFINITY,INFINITY,INFINITY };
		std::array<float, 3> max = { -INFINITY,-INFINITY,-INFINITY };
		for (int i = 0; i < verts_.size(); i ++) {
			for (int j = 0; j < 3; j++) {
				float v = verts_[i](j);
				if (v < min[j]) {
					min[j] = v;
				}
				if (v > max[j]) {
					max[j] = v;
				}
			}
		}
		bounding_box_ << max[0] - min[0], max[1] - min[1], max[2] - min[2];
		box_center_ << (max[0] + min[0]) / 2, (max[1] + min[1]) / 2, (max[2] + min[2]) / 2;


	}
	//returns true if the line segment e12 intersects the triangle t123
	static bool crossesTriangle(Eigen::Vector3f e1, Eigen::Vector3f e2, Eigen::Vector3f t1, Eigen::Vector3f t2, Eigen::Vector3f t3) {

		// v(k) = k*(e2-e1) + e1
		// (v(K)-t1).n = 0
		// (K*(e2-e1)+e1-t1).n = 0
		// (K*(e2-e1).n + e1.n - t1.n) = 0
		// K = (t1-e1).n/(e2-e1).n
		// if (K > 1 or K < 0) no collision
		// v(K) = ...
		//                                              _______
		//(eq.1) a*(t2-t1) + b*(t3-t1) = v(K)-t1        \*****/\    .  = v(K)
		//if(a+b > 1 or a<0 or b<0)						 \***/. \   \_ = a*(t2-t1) + b*(t3-t1)
		//												  \*/__\_\  *  = no collision
		
		//since eq.1 must have a solution, [a|b] = [t21(1:2),t31(1:2)].inv()*(v(K)-t1)
		
		//R = [(t2-t1)/||t2-t1||, ((t2-t1)/||t2-t1||)xn, n]

		//Eigen::Vector3f n = (t2 - t1).cross(t3 - t1);
		//Eigen::Vector3f e21 = e2 - e1;
		Eigen::Vector3f t21 = t2 - t1;
		Eigen::Vector3f t31 = t3 - t1;
		/*
		float K = ((t1 - e1).transpose()*n)(0) / (e21.transpose()*n)(0);
		if (K > 1. || K < 0.) { 
			return false; 
		}
		Eigen::Vector3f v = e21 * K + e1;
		*/
		Eigen::Matrix3f tmp;
		tmp(seq(0, 2), 0) = t21;
		tmp(seq(0, 2), 1) = t31;
		tmp(seq(0, 2), 2) = (e1-e2);
		//a*(t21) + b*t31 + t1 = k*(e2-e1)+e1
		//[t21 | t31 | (e1-e2)]*[a;b;k;] = e1-t1
		if (tmp.determinant() == 0) {
			return false; 
		}
		Eigen::Vector3f abk = tmp.inverse() * (e1-t1);
		if (abk(2) > 1. || abk(2) < 0 || abk(0) < 0 || abk(1) < 0 || abk(0) + abk(1) > 1.) {
			return false;
		} else {
			return true;
		}
	}

	static bool crossesTriangle(Eigen::Vector3f e1, Eigen::Vector3f e2, Eigen::Vector3f t1, Eigen::Vector3f t2, Eigen::Vector3f t3, float* k) {
		Eigen::Vector3f t21 = t2 - t1;
		Eigen::Vector3f t31 = t3 - t1;
		
		Eigen::Matrix3f tmp;
		tmp(seq(0, 2), 0) = t21;
		tmp(seq(0, 2), 1) = t31;
		tmp(seq(0, 2), 2) = (e1 - e2);
		//a*(t21) + b*t31 + t1 = k*(e2-e1)+e1
		//[t21 | t31 | (e1-e2)]*[a;b;k;] = e1-t1
		if (tmp.determinant() == 0) {
			return false;
		}
		Eigen::Vector3f abk = tmp.inverse() * (e1 - t1);
		if (abk(2) > 1. || abk(2) < 0 || abk(0) < 0 || abk(1) < 0 || abk(0) + abk(1) > 1.) {
			return false;
		}
		else {
			*k = abk(2);
			return true;
		}
	}

public:

	//this (primary surface) is the "shield" and other(secondary surface) is the "sword"
	//i.e. if secondary is a single edge then it will work but not vice versa
	bool crossesSurface(Eigen::Vector<float, 3> first_state, Eigen::Vector<float, 3> second_state) const override {
		for (const std::tuple<int, int, int>& f : faces_) {
			if (crossesTriangle(first_state, second_state, verts_[std::get<0>(f)], verts_[std::get<1>(f)], verts_[std::get<2>(f)])) {
				return true;
			}
		} 
		return false;
	}

	bool crossesSurface(Eigen::Vector<float, 3> first_state, Eigen::Vector<float, 3> second_state, float* loc) const {
		for (const std::tuple<int, int, int>& f : faces_) {
			if (crossesTriangle(first_state, second_state, verts_[std::get<0>(f)], verts_[std::get<1>(f)], verts_[std::get<2>(f)],loc)) {
				return true;
			}
		}
		return false;
	}

	const std::vector<Eigen::Vector3f>& getVerts() const {
		return verts_;
	}
	void addVert(float x, float y, float z) {
		verts_.emplace_back(x, y, z);
	}
	void addVert(Eigen::Vector3f vert) {
		verts_.emplace_back(vert);
	}

	const std::vector<std::pair<int, int>>& getEdges() const {
		return edges_;
	}
	void addEdge(int first_ind, int second_ind) {
		edges_.emplace_back( first_ind,second_ind );
	}

	const std::vector<std::tuple<int, int, int>>& getFaces() const {
		return faces_;
	}
	void addFace(int first_ind, int second_ind, int third_ind) {
		faces_.emplace_back(first_ind, second_ind, third_ind);
	}

	void centerVerts() {
		for (int i = 0; i < verts_.size(); i ++) {
			for (int j = 0; j < 3; j++) {
				verts_[i](j) -= box_center_(j);
			}
		}
		box_center_ << 0, 0, 0;
	}

	explicit MeshSurface(std::string fname);
	MeshSurface(std::string fname, std::string path);

	MeshSurface(){}

};

class Ellipse : public Region<3> {//technially generalizable into N dimensions but not useful for making games lol
	const Eigen::Vector3f E_;
	const Eigen::Vector3f E_inv_sq_;
public:
	bool insideRegion(Eigen::Vector<float, 3> state) const {
		return state.array().pow<int>(2).matrix().transpose() * E_inv_sq_;
	};

};
#endif