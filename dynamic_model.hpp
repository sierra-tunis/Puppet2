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

	DynamicModel(std::string model_fname, std::string vertex_groups_fname):
	Model(model_fname){
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

	void updateData() {
		for (int i = 0; i < vlen(); i++) {
			const Eigen::Matrix3f& rot = (*vert_tforms_[i])(seq(0, 2), seq(0, 2));
			Eigen::Vector3f pos = vert_mat_(seq(0, 2), i);
			pos = rot * pos + (*vert_tforms_[i])(seq(0, 2), 3);
			verts_[3 * i] = pos(0);
			verts_[3 * i + 1] = pos(1);
			verts_[3 * i + 2] = pos(2);

			Eigen::Vector3f norm = norm_mat_(seq(0, 2), i);
			norm = rot * norm;
			norms_[3 * i] = norm(0);
			norms_[3 * i + 1] = norm(1);
			norms_[3 * i + 2] = norm(2);
		}
	}

};

#endif