#pragma once

#ifndef PUPPET_DYNAMICMODEL
#define PUPPET_DYNAMICMODEL

#include <fstream>
#include <sstream>
#include <iostream>

#include "Model.h"

using Eigen::seq;

class DynamicModel : public Model {
	Eigen::Matrix<float, 3, -1> vert_mat_;
	Eigen::Matrix<float, 3, -1> norm_mat_;
	std::vector<int> vert_groups_;
	std::vector<const Eigen::Matrix4f*> vert_tforms_;
	std::unordered_map<std::string, int> group_index_by_name_;
	int n_groups_;

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
		std::string line;
		std::string type;
		std::string value;
		std::string entries;
		std::ifstream objFile(MODEL_PATH + vertex_groups_fname);
		while (std::getline(objFile, line)) {
			std::stringstream ss(line);
			std::getline(ss, type, ' ');
			if (line[0] == 'g') {
				std::getline(ss, value, ' ');
				int index = std::stoi(value);
				std::getline(ss, value, ' ');
				group_index_by_name_.insert({ value,index });
			}
			else{
				std::getline(ss, value, ' ');//index is ordered so not needed for now
				std::getline(ss, value, ' ');
				vert_groups_.push_back(std::stoi(value)); //only get first group
			}
		}
		objFile.close();
		n_groups_ = group_index_by_name_.size();
	}

	void setVertTforms(std::vector<const Eigen::Matrix4f*> vert_tforms) {
		vert_tforms_ = vert_tforms;
	}

	void updateData() {
		for (int i = 0; i < vlen(); i++) {
			const Eigen::Matrix4f& tform = (*vert_tforms_[vert_groups_[i]]);
			const Eigen::Matrix3f& rot = tform(seq(0, 2), seq(0, 2));
			Eigen::Vector3f pos = vert_mat_(seq(0, 2), i);
			pos = rot * pos + tform(seq(0, 2), 3);
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

	int glen() const {
		return n_groups_;
	}

	int getInd(std::string group_name) const {
		return group_index_by_name_.at(group_name);
	}

};

#endif