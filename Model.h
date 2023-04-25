#pragma once
#ifndef PUPPET_GRAPHICSMODEL
#define PUPPET_GRAPHICSMODEL

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

#include<Eigen/Dense>

#define MODEL_PATH "C:\\Users\\Justin\\source\\repos\\Puppet2\\Puppet2\\assets\\"
//this is conceptually the same as "mesh" may want to rename since a model can also be nurbs, but a mesh is always a mesh
class Model {
private:
	//std::unordered_map<std::vector<float>,std::string> vertex_data;
	std::vector<float> verts_;
	std::vector<float> norms_;
	std::vector<float> tex_coords_;
	std::vector<unsigned int> faces_;
	std::vector<unsigned int> face_norms_;
	std::vector<unsigned int> face_tex_;

	Eigen::Vector3f bounding_box_;
	Eigen::Vector3f box_center_;

	size_t n_verts_;
	size_t n_faces_;
	const std::string fname_;

	bool loaded;

	void reassign_vtx() {
		std::vector<float> tex_temp(tex_coords_);
		tex_coords_ = std::vector<float>(n_verts_*2);
		for (size_t i = 0; i < faces_.size(); i++) {
			tex_coords_[2*faces_[i]] = tex_temp[2*face_norms_[i]];
			//if crash here then the blender model is not in smooth vertex mode
			tex_coords_[2*faces_[i]+1] = tex_temp[2 * face_norms_[i]+1];

		}
		return;
	}

	void calculateBoundingBox() {
		std::array<float, 3> min = { INFINITY,INFINITY,INFINITY };
		std::array<float, 3> max = { -INFINITY,-INFINITY,-INFINITY };
		for (int i = 0; i < 3*this->vlen(); i += 3) {
			for (int j = 0; j < 3; j++) {
				float v = verts_[i + j];
				if (verts_[i + j] < min[j]) {
					min[j] = verts_[i + j];
				}
				if (verts_[i + j] > max[j]) {
					max[j] = verts_[i + j];
				}
			}
		}
		bounding_box_ << max[0] - min[0], max[1] - min[1], max[2] - min[2];
		box_center_ << (max[0] + min[0])/2, (max[1] + min[1])/2, (max[2] + min[2])/2;


	}

public:
	Model(std::vector<float> verts, std::vector<float> norms, std::vector<float> tex_coords, std::vector<unsigned int> faces, std::vector<unsigned int> face_norms, std::vector<unsigned int> face_tex) :
		verts_(verts),
		norms_(norms),
		tex_coords_(tex_coords),
		faces_(faces),
		face_norms_(face_norms),
		face_tex_(face_tex),
		fname_("") {
		//reassign_vtx();
		calculateBoundingBox();
	}

	Model(std::string fname) {
		std::string line;
		std::string type;
		std::string value;
		std::vector<float>* dest;
		std::string entries;
		std::ifstream objFile(MODEL_PATH + fname);
		// Use a while loop together with the getline() function to read the file line by line
		while (std::getline(objFile, line)) {
			std::stringstream ss(line);
			std::getline(ss, type, ' ');
			if (line[0] == 'v') {
				if (type == "v") {
					dest = &verts_;
				} else if (type == "vn") {
					dest = &norms_;
				} else if (type == "vt") {
					dest = &tex_coords_;
				} else return;
				while (std::getline(ss, value, ' ')) {
					dest->push_back(std::stof(value));
				}

			}else if(line[0] == 'f') {
				while (std::getline(ss, entries, ' ')) {
					std::stringstream sub_ss(entries);
					std::getline(sub_ss, value, '\/');
					faces_.push_back(std::stof(value)-1);
					std::getline(sub_ss, value, '\/');
					face_tex_.push_back(std::stof(value)-1);
					std::getline(sub_ss, value, '\/');
					face_norms_.push_back(std::stof(value)-1);
				}
			}
		}
		objFile.close();
		n_verts_ = verts_.size()/3;
		n_faces_ = faces_.size()/3;

		reassign_vtx();
		calculateBoundingBox();
	}


	/*Model(std::string fname) :verts({
		-0.5f, 0.f, 0.0f, // left  
		 0.5f, 0.f, 0.0f, // right 
		 0.0f,  0.5f, 0.0f,  // top   
		 0.0f,  -0.5f, 0.0f  // bottom
		}),faces({0,3,1,0,2,1}), n_faces(2), n_verts(4) {

	}*/

	~Model() {

	}	

	const std::vector<float> getVerts() const {
		return verts_;
	}

	const std::vector<float> getNorms() const {
		return norms_;
	}

	const std::vector<unsigned int> getFaces() const {
		return faces_;
	}

	const std::vector<float> getTexCoords() const {
		return tex_coords_;
	}

	constexpr size_t vlen() const {
		return n_verts_;
	}

	constexpr size_t flen() const {
		return n_faces_;
	}

	void addVert(Eigen::Vector3f pos) {
		verts_.push_back(pos(0));
		verts_.push_back(pos(1));
		verts_.push_back(pos(2));
		n_verts_++;
	}

	void addNorm(Eigen::Vector3f dir) {
		norms_.push_back(dir(0));
		norms_.push_back(dir(1));
		norms_.push_back(dir(2));
	}

	void addTexCoord(float x, float y) {
		tex_coords_.push_back(x);
		tex_coords_.push_back(y);
	}

	void addFace(int vert1, int vert2, int vert3) {
		faces_.push_back(vert1);
		faces_.push_back(vert2);
		faces_.push_back(vert3);
		n_faces_ ++;
	}

	const Eigen::Vector3f& getBoundingBox() const {
		return bounding_box_;
	}

	const Eigen::Vector3f& getBoxCenter() const {
		return box_center_;
	}

	void centerVerts() {
		for (int i = 0; i < 3*vlen(); i+=3) {
			for (int j = 0; j < 3; j++) {
				verts_[i + j] -= box_center_(j);
			}
		}
		box_center_ << 0, 0, 0;
	}

	int getID() const {
		std::cerr << "no ID available for Model class";
		return 0;
	}
	/*
	void rescale(float scale_factor) {
		
	}*/

};

#endif