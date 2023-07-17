#pragma once

#ifndef PUPPET_PHYSICSMESH
#define PUPPET_PHYSICSMESH

#include "Model.h"

//dims=1 -> line, dims=2 -> surface, dims=3 ->volume
template<int nth_order, int n_dims>// here "n_dims" doesnt mean number of spatial dimensions, but rather number of curvilinear coord dimensions
class PhysicsModel : public Model {

	std::vector<Eigen::Vector<float, 3*(nth_order+1)>> x_; //diff(x,t,n)
	std::vector<Eigen::Matrix<float, 3, n_dims>> F_; //deformation gradient

	void (*ODE_)(const Eigen::Vector<float, 3 * (nth_order + 1)>& x, Eigen::Vector<float, 3 * (nth_order + 1)>& dx);

public:

	PhysicsModel() : Model(),x_(0){

	}

	const std::vector<Eigen::Vector<float, 3 * (nth_order + 1)>>& getx() {
		return x_;
	}

	void beginPhysics() {
		beginPhysics(std::vector<Eigen::Vector<float, 3 * (nth_order + 1)>>(0));
	}

	void beginPhysics(const std::vector<Eigen::Vector<float, 3 * (nth_order + 1)>>& x0) {
		x_ = x0;
	}

	static void LiquidFreefall(const Eigen::Vector<float, 3 * (nth_order + 1)>& x, Eigen::Vector<float, 3 * (nth_order + 1)>& dxdt) {

		dxdt(seq(0, 2)) = x(seq(3, 5));
		dxdt(seq(3, 5)) = Eigen::Vector3f(0,-9.81,0);

	}

	void setODE(void (*ODE)(const Eigen::Vector<float, 3 * (nth_order + 1)>& x, Eigen::Vector<float, 3 * (nth_order + 1)>& dxdt)) {
		ODE_ = ODE;
	}

	void update(float dt) {
		if (ODE_ != nullptr) {
			for (int i = 0; i < x_.size(); i++) {
				//do ode step
				Eigen::Vector<float, 3 * (nth_order + 1)> dxdt_i;
				ODE_(x_[i], dxdt_i);
				x_[i] += dxdt_i * dt;
				//update visual model
				setVert(i, x_[i](seq(0, 2)));
			}
		}
	}

};

#endif