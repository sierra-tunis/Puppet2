#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

#define MODEL_PATH "C:\\Users\\Justin\\source\\repos\\Puppet2\\Puppet2\\assets\\"
//this is conceptually the same as "mesh" may want to rename since a model can also be nurbs, but a mesh is always a mesh
class Model {
private:
	//std::unordered_map<std::vector<float>,std::string> vertex_data;
	std::vector<float> verts;
	std::vector<float> norms;
	std::vector<float> tex_coords;
	std::vector<unsigned int> faces;
	std::vector<unsigned int> face_norms;
	std::vector<unsigned int> face_tex;

	Eigen::Vector3f bounding_box_;
	Eigen::Vector3f box_center_;

	size_t n_verts;
	size_t n_faces;
	const std::string fname;

	bool loaded;

	void reassign_vtx() {
		std::vector<float> tex_temp(tex_coords);
		tex_coords = std::vector<float>(n_verts*2);
		for (size_t i = 0; i < faces.size(); i++) {
			tex_coords[2*faces[i]] = tex_temp[2*face_norms[i]];
			//if crash here then the blender model is not in smooth vertex mode
			tex_coords[2*faces[i]+1] = tex_temp[2 * face_norms[i]+1];

		}
		return;
	}

	void calculateBoundingBox() {
		std::array<float, 3> min = { INFINITY,INFINITY,INFINITY };
		std::array<float, 3> max = { -INFINITY,-INFINITY,-INFINITY };
		for (int i = 0; i < 3*this->vlen(); i += 3) {
			for (int j = 0; j < 3; j++) {
				float v = verts[i + j];
				if (verts[i + j] < min[j]) {
					min[j] = verts[i + j];
				}
				if (verts[i + j] > max[j]) {
					max[j] = verts[i + j];
				}
			}
		}
		bounding_box_ << max[0] - min[0], max[1] - min[1], max[2] - min[2];
		box_center_ << (max[0] + min[0])/2, (max[1] + min[1])/2, (max[2] + min[2])/2;


	}

public:
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
					dest = &verts;
				} else if (type == "vn") {
					dest = &norms;
				} else if (type == "vt") {
					dest = &tex_coords;
				} else return;
				while (std::getline(ss, value, ' ')) {
					dest->push_back(std::stof(value));
				}

			}else if(line[0] == 'f') {
				while (std::getline(ss, entries, ' ')) {
					std::stringstream sub_ss(entries);
					std::getline(sub_ss, value, '\/');
					faces.push_back(std::stof(value)-1);
					std::getline(sub_ss, value, '\/');
					face_tex.push_back(std::stof(value)-1);
					std::getline(sub_ss, value, '\/');
					face_norms.push_back(std::stof(value)-1);
				}
			}
		}
		objFile.close();
		n_verts = verts.size()/3;
		n_faces = faces.size()/3;

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
		return verts;
	}

	const std::vector<float> getNorms() const {
		return norms;
	}

	const std::vector<unsigned int> getFaces() const {
		return faces;
	}

	const std::vector<float> getTexCoords() const {
		return tex_coords;
	}

	constexpr size_t vlen() const {
		return n_verts;
	}

	constexpr size_t flen() const {
		return n_faces;
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
				verts[i + j] -= box_center_(j);
			}
		}
		box_center_ << 0, 0, 0;
	}


};