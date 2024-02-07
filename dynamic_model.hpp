#pragma once

#ifndef PUPPET_DYNAMICMODEL
#define PUPPET_DYNAMICMODEL

#include <fstream>
#include <sstream>
#include <iostream>

#include "Model.h"
#include "vertex_group.hpp"

using Eigen::seq;

class DynamicModel : public Model {
	Eigen::Matrix<float, 3, -1> vert_mat_;
	Eigen::Matrix<float, 3, -1> norm_mat_;

	std::vector<VertexGroup*> vert_groups_;
	const Eigen::Matrix4f* root_tform_;

	std::unordered_map<std::string, int> group_index_by_name_;
	std::unordered_map<std::string, VertexGroup*> group_by_name_;
	std::vector<VertexGroup*> primary_group_by_vert_;

	int n_groups_;

	void loadMatrices() {
		vert_mat_.resize(3, vlen());
		norm_mat_.resize(3, vlen());
		for (int i = 0; i < vlen(); i++) {
			vert_mat_(0, i) = getVerts()[3 * i];
			vert_mat_(1, i) = getVerts()[3 * i + 1];
			vert_mat_(2, i) = getVerts()[3 * i + 2];

			norm_mat_(0, i) = getNorms()[3 * i];
			norm_mat_(1, i) = getNorms()[3 * i + 1];
			norm_mat_(2, i) = getNorms()[3 * i + 2];
		}
	}

public:

	DynamicModel():
		Model(){
	}

	DynamicModel(std::string model_fname, std::string vertex_groups_fname, bool force_shade_hard=true) :
		DynamicModel(model_fname, vertex_groups_fname, Model::default_path, force_shade_hard) {}


	DynamicModel(std::string model_fname, std::string vertex_groups_fname, std::string path, bool force_shade_hard=true) :
	Model(model_fname, path, force_shade_hard){
		
		if (!shade_smooth_) {
			//std::cerr << "hard dynamic models not implemented yet!\n";
		}
		loadMatrices();

		std::vector<unsigned int> OBJ_groups;
		std::vector<unsigned int> OBJ_indices;

		std::string line;
		std::string type;
		std::string value;
		std::string entries;
		std::ifstream objFile(Model::default_path + vertex_groups_fname);
		while (std::getline(objFile, line)) {
			std::stringstream ss(line);
			std::getline(ss, type, ' ');
			if (line[0] == 'g') {
				std::getline(ss, value, ' ');
				int index = std::stoi(value);
				std::getline(ss, value, ' ');
				std::string name = value;
				//group_index_by_name_.insert({ name,index });
				VertexGroup* vgroup = new VertexGroup(name);
				group_index_by_name_.insert({ name,index });
				group_by_name_.insert({ name,vgroup });
				vert_groups_.push_back(vgroup);
			}
			else{
				//std::getline(ss, value, ' ');//index is ordered so not needed for now
				int index = std::stoi(type);
				std::getline(ss, value, ' ');
				int group = std::stoi(value);
				OBJ_indices.push_back(index);
				OBJ_groups.push_back(group);
			}

		}
		objFile.close();

		std::vector<unsigned int> gl_groups;
		std::vector<unsigned int> gl_indices;

		objVertData2gl<unsigned int,1>(OBJ_indices, gl_indices);
		objVertData2gl<unsigned int,1>(OBJ_groups, gl_groups);

		for (int i = 0; i < gl_groups.size(); i++) {
			vert_groups_[gl_groups[i]]->addVert(i); //only get first group
			primary_group_by_vert_.push_back(vert_groups_[gl_groups[i]]);//works because verts are ordered in file
		}
		
		n_groups_ = vert_groups_.size();

	}

	VertexGroup* getGroup(std::string group_name) const {
		return group_by_name_.at(group_name);
	}

	VertexGroup* getGroup(int vertex_index) const {
		return primary_group_by_vert_[vertex_index];
	}

	const std::vector<VertexGroup*>& getVertexGroups() const {
		return vert_groups_;
	}

	/*
	void setVertTforms(std::vector<const Eigen::Matrix4f*> vert_tforms) {
		vert_tforms_ = vert_tforms;
	}*/

	void updateData() override {
		for (const auto& vg : vert_groups_) {
			if (vg->getTform() != nullptr) {
				const Eigen::Matrix4f& tform = *vg->getTform();
				const Eigen::Matrix4f rel_tform = root_tform_ == nullptr ? tform : root_tform_->inverse() * tform;
				const Eigen::Matrix3f& rot = rel_tform(seq(0, 2), seq(0, 2));
				for (const VertexGroup::vertex& vert : vg->getVerts()) {
					Eigen::Vector3f pos = vert_mat_(seq(0, 2), vert.index);
					pos = rot * pos + rel_tform(seq(0, 2), 3);
					setVert(vert.index, pos);
					Eigen::Vector3f norm = norm_mat_(seq(0, 2), vert.index);
					norm = rot * norm;
					setNorm(vert.index, norm);
				}
			}
		}
	}

	/*void offsetVerts(std::vector<const Eigen::Matrix4f*> initial_positions) {
		std::vector<const Eigen::Matrix4f*> tmp;
		std::vector<Eigen::Matrix4f> inverse_positions_data;
		std::vector<const Eigen::Matrix4f*> inverse_positions;
		for (auto& vg : vert_groups_) {
			inverse_positions_data.emplace_back(initial_positions[i]->inverse());
			tmp.emplace_back(vg->getTform());
			vg->setTform(vg->getTform()->inverse());
		}
		for (int i = 0; i < glen(); i++) {
			inverse_positions.push_back(&inverse_positions_data[i]);
		}
		vert_tforms_ = inverse_positions;
		updateData();
		loadMatrices();
		vert_tforms_ = tmp;
		//offsets each vert such that when the vert tforms are equal to initial_positions then the
		//model will look as it is in the .obj file

	}*/

	void offsetVerts() {
		std::vector<const Eigen::Matrix4f*> tmp;//saves current vg tforms
		std::vector<Eigen::Matrix4f> inverse_positions_data;
		std::vector<const Eigen::Matrix4f*> inverse_positions;
		for (auto& vg : vert_groups_) {
			if (vg->getTform() != nullptr) {
				inverse_positions_data.emplace_back(vg->getTform()->inverse());
			} else {
				inverse_positions_data.emplace_back(Eigen::Matrix4f::Identity());

			}
			tmp.emplace_back(vg->getTform());
		}
		for (int i = 0; i < vert_groups_.size(); i++) {
			vert_groups_[i]->setTform(&(inverse_positions_data[i]));
		}
		updateData();
		loadMatrices();
		for (int i = 0; i < vert_groups_.size(); i++) {
			vert_groups_[i]->setTform(tmp[i]);
		}
	}


	int glen() const {
		return n_groups_;
	}

	int getInd(std::string group_name) const {
		return group_index_by_name_.at(group_name);
	}
	
	void setRootTransform(const Eigen::Matrix4f* root_transform) {
		root_tform_ = root_transform;
	}

};

#endif