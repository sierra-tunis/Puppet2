#pragma once

#ifndef PUPPET_SEQUENCE
#define PUPPET_SEQUENCE

#include<Eigen/Dense>

using Eigen::seq;

template<int n_dofs>
struct StateSequence {//building block upon which animations will be based
	//using Eigen::matrix instead of say vector so that i can easily take derivatives and thus do jacobians
	//second dim isnt a template because it would be annoying to look up in a file after its created how many frames the animation is, but you probably know how many dofs the model is
	Eigen::Matrix<float, n_dofs + 1, -1> sequence_;//[x | t]
	std::unordered_map<std::string, int> columns_by_name_;

	bool looping_; //if not looping, then any time after the end will return the last frame

	enum interpolation_method{nearest,linear,spline,fourier};
	interpolation_method interp_method_;

public:
	StateSequence():interp_method_(linear),looping_(true) {

	}

	StateSequence<n_dofs+1> addRow(std::string row_name, Eigen::Vector<float, -1> data) {
		if (data.size() != sequence_.size()/(n_dofs+1)) {
			std::cerr << "new row size doesnt match current data";
		}
		//...
	}

	void addCol(float time, Eigen::Vector<float, n_dofs> state) {
		sequence_.conservativeResize(sequence_.rows(), sequence_.cols() + 1);
		sequence_(seq(1, n_dofs), sequence_.cols() - 1) = state;
		sequence_(0, sequence_.cols() - 1) = time;
	}

	void addCol(Eigen::Vector<float, n_dofs + 1> data) {
		sequence_.conservativeResize(sequence_.rows(), sequence_.cols() + 1);
		sequence_(seq(0, n_dofs), sequence_.cols() - 1) = data;
	}

	void setCol(int col, float time, Eigen::Vector<float, n_dofs> state) {
		sequence_(seq(1, n_dofs), col) = state;
		sequence_(0, col) = time;
	}

	void setCol(int col, Eigen::Vector<float, n_dofs+1> data) {
		sequence_(seq(0, n_dofs), col) = data;
	}

	Eigen::Vector<float, n_dofs> getState(float time) const {
		if (interp_method_ == nearest) {
			int step = static_cast<int>(time + .5);
			if (step < sequence_.cols()) {
				if (looping_) {
					step %= sequence_.cols();
				} else {
					step = sequence_.cols() - 1;
				}
			} else if(step < 0){
				step = 0;
			}
			return sequence_.col(step)(seq(1,n_dofs));
		} else if (interp_method_ == linear){//this assumes 1 frame is 1 second
			int step_below = static_cast<int>(time);
			int step_above = step_below + 1;
			float r, time_rounded;
			r = std::modf(time, &time_rounded);
			if (step_above >= sequence_.cols()) {
				if (looping_) {
					step_below %= sequence_.cols();
					step_above %= sequence_.cols();
				}
				else {
					//last row
					return sequence_.col(sequence_.cols()-1)(seq(1, n_dofs));
				}
			}
			else if (step_below < 0) {
				return sequence_.col(0)(seq(1, n_dofs));

			}
			return ((sequence_.col(step_above) - sequence_.col(step_below)) * r + sequence_.col(step_below))(seq(1,n_dofs));
		}
	}

	Eigen::Vector<float, n_dofs+1> getData(int col) const {
		return sequence_(seq(0, n_dofs), col);
	}

	bool saveToFile(std::string fname, std::string path) const {
		std::ofstream file(path + fname);
		if(file.is_open()){
			for (int i = 0; i < size(); i++) {
				for (int j = 0; j < n_dofs+1; j++) {
					file << sequence_(j, i) << " ";
				}
				file << "\n";
			}
			file.close();
			return true;
		}else {
			return false;
		}
	}

	bool readFromFile(std::string fname, std::string path) {
		std::string line;
		std::string headers;
		std::string index_time;
		std::string dof_state;
		std::ifstream animFile(path + fname);
		if (animFile.is_open()) {
			//std::getline(animFile, headers);
			sequence_.resize(n_dofs+1,0);
			while (std::getline(animFile, line)) {
				Eigen::Vector<float, n_dofs> new_col;
				std::stringstream ss(line);
				std::getline(ss, index_time, ' ');
				float t = std::stof(index_time);
				for (int i = 0; i < n_dofs; i++) {
					std::getline(ss, dof_state, ' ');
					new_col(i) = std::stof(dof_state);
				}
				addCol(t, new_col);
			}
			animFile.close();
			return true;
		} else {
			return false;
		}
	}

	int size() const {
		return sequence_.cols();
	}
	float getColTime(int col) {
		return sequence_(0, col);
	}

	float getLastTime() {
		return sequence_(0, sequence_.cols() - 1);
	}


};

#undef ANIMATION_PATH
#endif