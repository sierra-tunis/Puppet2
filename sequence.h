#pragma once

#include<Eigen/Dense>

template<int n_dofs>
struct StateSequence {//building block upon which animations will be based
	//using Eigen::matrix instead of say vector so that i can easily take derivatives and thus do jacobians
	//second dim isnt a template because it would be annoying to look up in a file after its created how many frames the animation is, but you probably know how many dofs the model is
	Eigen::Matrix<float, n_dofs + 1, -1> sequence;//[x | t]

	bool looping_;

public:

	bool saveToFile(std::string fname) const {
		//...
	}

	bool readFromFile(std::string fname) const {

	}

};