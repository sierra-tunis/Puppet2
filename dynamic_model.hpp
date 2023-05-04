#pragma once

#ifndef PUPPET_DYNAMICMODEL
#define PUPPET_DYNAMICMODEL

#include "Model.h"

using Eigen::seq;

class DynamicModel : public Model {
	Eigen::Matrix<float, 3, -1> vert_mat_;
	Eigen::Matrix<float, 3, -1> norm_mat_;
	std::vector<const Eigen::Matrix4f*> vert_tforms_;

public:

	DynamicModel(std::string fname):
	Model(fname){
		vert_mat_.resize(3,vlen());
		norm_mat_.resize(3, vlen());
		for (int i = 0; i < vlen(); i++) {
			vert_mat_(0, i) = getVerts()[3 * i];
			vert_mat_(1, i) = getVerts()[3 * i + 1];
			vert_mat_(2, i) = getVerts()[3 * i + 2];

			norm_mat_(0, i) = getNorms()[3 * i];
			norm_mat_(1, i) = getNorms()[3 * i+1];
			norm_mat_(2, i) = getNorms()[3 * i+2];
		}

	}

	void setVertTforms(std::vector<const Eigen::Matrix4f*> vert_tforms) {
		vert_tforms_ = vert_tforms;
	}

	const float* getVertData() const override {
		std::vector<float>* tformed_verts = new std::vector<float>;
		tformed_verts->reserve(vlen() * 3);
		for (int i = 0; i < vlen(); i++) {
			Eigen::Vector3f pos = vert_mat_(seq(0, 2), i);
			pos = (*vert_tforms_[i])(seq(0, 2), seq(0, 2)) * pos + (*vert_tforms_[i])(seq(0, 2), 3);
			tformed_verts->push_back(pos(0));
			tformed_verts->push_back(pos(1));
			tformed_verts->push_back(pos(2));
		}
		return tformed_verts->data();
	}

	const float* getNormData() const override {
		std::vector<float>* tformed_norms = new std::vector<float>;
		tformed_norms->reserve(vlen() * 3);
		for (int i = 0; i < vlen(); i++) {
			Eigen::Vector3f norm = norm_mat_(seq(0, 2), i);
			norm = (*vert_tforms_[i])(seq(0, 2), seq(0, 2)) * norm;
			tformed_norms->push_back(norm(0));
			tformed_norms->push_back(norm(1));
			tformed_norms->push_back(norm(2));
		}
		return tformed_norms->data();
	}

};

#endif