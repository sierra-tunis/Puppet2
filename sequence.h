#pragma once

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

	StateSequence<n_dofs+1> addColumn(std::string col_name, Eigen::Vector<float, -1> data) {
		if (data.size() != sequence_.size()/(n_dofs+1)) {
			std::cerr << "new column size doesnt match current data"
		}
		//...
	}

	void addRow(float time, Eigen::Vector<float, n_dofs> state) {

	}

	void addRow(Eigen::Vector<float, n_dofs + 1> data) {

	}

	void setRow(int time_step, float time, Eigen::Vector<float, n_dofs> state) {

	}

	void setRow(int time_step,Eigen::Vector<float, n_dofs+1> data) {

	}
	Eigen::Vector<float, n_dofs> getState(float time) const {
		if (interp_method_ == nearest) {
			int step = static_cast<int>(time + .5);
			if (step < sequence_.rows()) {
				if (looping_) {
					step %= sequence_.rows();
				} else {
					step = sequence_.rows() - 1;
				}
			} else if(step < 0){
				step = 0;
			}
			return sequence_.row(step)(seq(1,n_dofs));
		} else if (interp_method_ == linear){
			int step_below = static_cast<int>(time);
			int step_above = step_below + 1;
			float r, time_rounded;
			float r = std::modf(time, &time_rounded);
			if (step_below < sequence_.rows()) {
				if (looping_) {
					step_below %= sequence_.rows();
					step_above %= sequence_.rows();
				}
				else {
					//last row
					return sequence_.row(sequence_.rows()-1)(seq(1, n_dofs));
				}
			}
			else if (step < 0) {
				return sequence_.row(0)(seq(1, n_dofs));

			}
			return (sequence_.row(step_below) - sequence_.row(step_above)) * r + sequence_.row(step_below);
		}
	}

	bool saveToFile(std::string fname) const {
		//...
	}

	bool readFromFile(std::string fname) const {

	}

};