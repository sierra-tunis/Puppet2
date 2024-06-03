#pragma once
#ifndef PUPPET_GRAPHICSMODEL
#define PUPPET_GRAPHICSMODEL

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

#include<Eigen/Dense>

//this is conceptually the same as "mesh" may want to rename since a model can also be nurbs, but a mesh is always a mesh
struct vertex;
struct edge;
struct face;

struct vertex{
	Eigen::Vector3f position;
	Eigen::Vector3f normal;
	Eigen::Vector2f tex_coord;
	std::vector<face*> faces;
	std::vector<edge*> edges;
};

struct edge {
	std::tuple<vertex&, vertex&> vertices;
};

struct face {
	std::tuple<vertex&, vertex&, vertex&> vertices;
	std::array<Eigen::Vector2f, 3> tex_coords;
	std::tuple<edge&, edge&, edge&> edges;
	Eigen::Vector3f normal;
};

class Model {

private:
	//these are the arrays filled with the exact obj data
	std::vector<float> OBJ_verts_;
	std::vector<float> OBJ_norms_;
	std::vector<float> OBJ_tex_coords_;
	std::vector<unsigned int> OBJ_face_verts_;
	std::vector<unsigned int> OBJ_face_norms_;
	std::vector<unsigned int> OBJ_face_tex_coords_;
	std::vector<unsigned int> OBJ_lines_;

	//since verbatim obj data is stored until the model is finalized, this keeps track of open files
	static std::unordered_map<std::string, Model*> open_files_;

protected:


	//std::unordered_map<std::vector<float>,std::string> vertex_data;
	//Draw data:
	std::vector<float> vert_data_;
	std::vector<float> norm_data_;
	std::vector<float> tex_coord_data_;
	std::vector<unsigned int> face_data_;
	std::vector<unsigned int> face_norm_data_;
	std::vector<unsigned int> face_tex_data_;
	std::vector<unsigned int> edge_data_;

	//Mesh data:
	//std::vector<vertex> vertices_;
	//std::vector<face> faces_;
	//std::vector<edge> edges_;

	Eigen::Vector3f bounding_box_;
	Eigen::Vector3f box_center_;

	size_t n_verts_;
	size_t n_faces_;
	const std::string fname_;

	bool loaded;
	bool shade_smooth_;

private:
	//deprecated
	void shadeByVertex() {
		face_data_ = OBJ_face_verts_;
		face_norm_data_ = std::vector<unsigned int>(n_faces_ * 3);
		face_tex_data_ = std::vector<unsigned int>(n_faces_ * 3);
		vert_data_ = OBJ_verts_;
		tex_coord_data_ = std::vector<float>(n_verts_*2);
		norm_data_ = std::vector<float>(n_verts_ * 3);
		for (size_t i = 0; i < OBJ_face_verts_.size(); i++) {
			norm_data_[3 * face_data_[i]] = OBJ_norms_[3 * OBJ_face_norms_[i]];
			norm_data_[3 * face_data_[i] + 1] = OBJ_norms_[3 * OBJ_face_norms_[i] + 1];
			norm_data_[3 * face_data_[i] + 2] = OBJ_norms_[3 * OBJ_face_norms_[i] + 2];
			face_norm_data_[i] = i;

			tex_coord_data_[2*face_data_[i]] = OBJ_tex_coords_[2 * OBJ_face_tex_coords_[i]];
			tex_coord_data_[2*face_data_[i]+1] = OBJ_tex_coords_[2 * OBJ_face_tex_coords_[i]+1];
			face_tex_data_[i] = i;

		}
		edge_data_ = OBJ_lines_;
		return;
	}


	void calculateBoundingBox() {
		std::array<float, 3> min = { INFINITY,INFINITY,INFINITY };
		std::array<float, 3> max = { -INFINITY,-INFINITY,-INFINITY };
		for (int i = 0; i < 3*this->vlen(); i += 3) {
			for (int j = 0; j < 3; j++) {
				float v = vert_data_[i + j];
				if (vert_data_[i + j] < min[j]) {
					min[j] = vert_data_[i + j];
				}
				if (vert_data_[i + j] > max[j]) {
					max[j] = vert_data_[i + j];
				}
			}
		}
		bounding_box_ << max[0] - min[0], max[1] - min[1], max[2] - min[2];
		box_center_ << (max[0] + min[0])/2, (max[1] + min[1])/2, (max[2] + min[2])/2;


	}
protected:

public:

	std::string getFilename() const {
		return fname_;
	}

	void setVert(int index, Eigen::Vector3f data) {
		vert_data_[3 * index] = data(0);
		vert_data_[3 * index + 1] = data(1);
		vert_data_[3 * index + 2] = data(2);
	}


	void setNorm(int index, Eigen::Vector3f data) {
		norm_data_[3 * index] = data(0);
		norm_data_[3 * index + 1] = data(1);
		norm_data_[3 * index + 2] = data(2);
	}


	template<class data_T, unsigned int data_vec_length>
	bool objVertData2gl(const std::vector<data_T>& OBJ_data, std::vector<data_T>& gl_data) {
		//reformats data. obj style vertex data (vert data+face data, i.e. EBO) gets written in gl style data(faces are 123,456,...)
		gl_data = std::vector<data_T>(n_faces_ * data_vec_length * 3);
		for (size_t i = 0; i < 3 * n_faces_; i++) {
			for (int j = 0; j < data_vec_length; j++) {
				gl_data[data_vec_length * i + j] = OBJ_data[data_vec_length * OBJ_face_verts_[i] + j];
			}
		}
		return true;
	}

	void obj2gl() {//reformats data to be compatible with opengl
		face_data_ = std::vector<unsigned int>(n_faces_ * 3);
		face_norm_data_ = std::vector<unsigned int>(n_faces_ * 3);
		face_tex_data_ = std::vector<unsigned int>(n_faces_ * 3);
		n_verts_ = n_faces_ * 3;
		vert_data_ = std::vector<float>(n_verts_ * 3);
		norm_data_ = std::vector<float>(n_verts_ * 3);
		tex_coord_data_ = std::vector<float>(n_verts_ * 2);
		for (size_t i = 0; i < 3 * n_faces_; i++) {
			vert_data_[3 * i] = OBJ_verts_[3 * OBJ_face_verts_[i]];
			vert_data_[3 * i + 1] = OBJ_verts_[3 * OBJ_face_verts_[i] + 1];
			vert_data_[3 * i + 2] = OBJ_verts_[3 * OBJ_face_verts_[i] + 2];
			face_data_[i] = i;

			norm_data_[3 * i] = OBJ_norms_[3 * OBJ_face_norms_[i]];
			norm_data_[3 * i + 1] = OBJ_norms_[3 * OBJ_face_norms_[i] + 1];
			norm_data_[3 * i + 2] = OBJ_norms_[3 * OBJ_face_norms_[i] + 2];
			face_norm_data_[i] = i;

			tex_coord_data_[2 * i] = OBJ_tex_coords_[2 * OBJ_face_tex_coords_[i]];
			tex_coord_data_[2 * i + 1] = OBJ_tex_coords_[2 * OBJ_face_tex_coords_[i] + 1];
			face_tex_data_[i] = i;
		}
		edge_data_ = OBJ_lines_;
		return;
	}
	static constexpr char debug_path[] = "C:\\Users\\Sierra\\source\\repos\\Puppet2\\Puppet2\\assets\\";
	static std::string default_path;

	Model(){}

	Model(std::vector<float> verts, std::vector<float> norms, std::vector<float> tex_coords, std::vector<unsigned int> faces, std::vector<unsigned int> face_norms, std::vector<unsigned int> face_tex) :
		vert_data_(verts),
		norm_data_(norms),
		tex_coord_data_(tex_coords),
		face_data_(faces),
		face_norm_data_(face_norms),
		face_tex_data_(face_tex),
		fname_("") {
		//reassign_vtx();
		calculateBoundingBox();
	}

	Model(std::string fname, bool force_shade_hard=true) : Model(fname, default_path, force_shade_hard) {
	}

	Model(std::string fname, std::string path, bool force_shade_hard=true):
	fname_(fname){
		std::string line;
		std::string type;
		std::string value;
		std::vector<float>* dest;
		std::string entries;
		std::ifstream objFile(path + fname);
		// Use a while loop together with the getline() function to read the file line by line
		while (std::getline(objFile, line)) {
			std::stringstream ss(line);
			std::getline(ss, type, ' ');
			if (line[0] == 'v') {
				if (type == "v") {
					dest = &OBJ_verts_;
				} else if (type == "vn") {
					dest = &OBJ_norms_;
				} else if (type == "vt") {
					dest = &OBJ_tex_coords_;
				} else return;
				while (std::getline(ss, value, ' ')) {
					dest->push_back(std::stof(value));
				}

			}else if(line[0] == 'f') {
				while (std::getline(ss, entries, ' ')) {
					std::stringstream sub_ss(entries);
					std::getline(sub_ss, value, '\/');
					OBJ_face_verts_.push_back(std::stof(value)-1);
					std::getline(sub_ss, value, '\/');
					OBJ_face_tex_coords_.push_back(std::stof(value) - 1);
					std::getline(sub_ss, value, '\/');
					OBJ_face_norms_.push_back(std::stof(value) - 1);
				}
			}
			else if (line[0] == 'l') {
				while (std::getline(ss, value, ' ')) {
					OBJ_lines_.push_back(std::stof(value)-1);
					//force_shade_hard = false;
				}
			}
		}
		objFile.close();
		n_verts_ = OBJ_verts_.size()/3;
		n_faces_ = OBJ_face_verts_.size()/3;

		if (force_shade_hard) {
			obj2gl();
			shade_smooth_ = false;
		} else {
			shadeByVertex();
			shade_smooth_ = true;
			//reformatShadedSmooth();
			//reassign_vtx();//this visually doesnt work if textures are broken into floating segments
		}
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

	Eigen::Vector3f getVert(int index) const {
		return Eigen::Vector3f(vert_data_[3 * index], vert_data_[3 * index + 1], vert_data_[3 * index + 2]);
	}

	const std::vector<float>& getVerts() const {
		return vert_data_;
	}

	Eigen::Vector3f getNorm(int index) const {
		return Eigen::Vector3f(norm_data_[3 * index], norm_data_[3 * index + 1], norm_data_[3 * index + 2]);
	}
	const std::vector<float>& getNorms() const {
		return norm_data_;
	}
	
	const std::vector<unsigned int>& getFaces() const {
		return face_data_;
	}

	Eigen::Vector2f getTexCoord(int index) const {
		return Eigen::Vector2f(tex_coord_data_[2 * index], tex_coord_data_[2 * index + 1]);
	}
	const std::vector<float>& getTexCoords() const {
		return tex_coord_data_;
	}


	const std::vector<unsigned int>& getLines() const {
		return edge_data_;
	}


	constexpr size_t vlen() const {
		return n_verts_;
	}

	constexpr size_t flen() const {
		return n_faces_;
	}

	void addVert(Eigen::Vector3f pos) {
		vert_data_.push_back(pos(0));
		vert_data_.push_back(pos(1));
		vert_data_.push_back(pos(2));
		n_verts_++;
	}

	void addNorm(Eigen::Vector3f dir) {
		norm_data_.push_back(dir(0));
		norm_data_.push_back(dir(1));
		norm_data_.push_back(dir(2));
	}
	void addTexCoord(Eigen::Vector2f coord) {
		tex_coord_data_.push_back(coord(0));
		tex_coord_data_.push_back(coord(1));
	}

	void addTexCoord(float x, float y) {
		tex_coord_data_.push_back(x);
		tex_coord_data_.push_back(y);
	}

	void addFace(int vert1, int vert2, int vert3) {
		face_data_.push_back(vert1);
		face_data_.push_back(vert2);
		face_data_.push_back(vert3);
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
				vert_data_[i + j] -= box_center_(j);
			}
		}
		box_center_ << 0, 0, 0;
	}

	int getID() const {
		std::cerr << "no ID available for Model class";
		return 0;
	}

	virtual void updateData(){
		
	}

	bool shadedSmooth() const {
		return shadedSmooth();
	}

	void finalize() {
		OBJ_verts_.clear();
		OBJ_norms_.clear();
		OBJ_tex_coords_.clear();
		OBJ_face_verts_.clear();
		OBJ_face_norms_.clear();
		OBJ_face_tex_coords_.clear();
		OBJ_lines_.clear();
	}
	
	void rescale(float x_scale, float y_scale, float z_scale) {
		if (OBJ_verts_.size() != vert_data_.size()) {
			OBJ_verts_ = vert_data_;
		}
		for (int i = 0; i < OBJ_verts_.size()/3; i++) {
			vert_data_[3*i] = OBJ_verts_[3*i] * x_scale;
			vert_data_[3*i + 1] = OBJ_verts_[3*i + 1] * y_scale;
			vert_data_[3*i + 2] = OBJ_verts_[3*i + 2] * z_scale;
		}
	}
};
#endif