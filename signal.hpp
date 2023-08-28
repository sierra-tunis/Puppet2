#pragma once

#ifndef PUPPET_SIGNAL
#define PUPPET_SIGNAL

#include "Eigen/dense"

class Signal_base {

public:
	virtual void advance(float dt) = 0;

};

template<int n_dofs=1>
class Signal : public Signal_base {

	Eigen::Vector<float,n_dofs> x_,dx_;
	Eigen::Matrix<float, n_dofs,n_dofs> k_, m_;
	float f;

	const Eigen::Vector<float, n_dofs>& getx() const {
		return x_;
	}
	operator Eigen::Vector<float, n_dofs>() const {
		return x_;
	}

	const Eigen::Vector<float, n_dofs>& getdx() const {
		return dx_;
	}
	virtual Eigen::Matrix<float, n_dofs, n_dofs> getMassMatrix() const {
		return m_;
	}
	virtual Eigen::Matrix<float, n_dofs, n_dofs> getKMatrix() const {//should maybe be a reference for speed
		return k_;
	}

	void advance(float dt) override {
		Eigen::Vector<float, n_dofs> new_delta_x = getdx();
		x_ = getMassMatrix().invert() * getKMatrix() * getdx() / dt;
		dx_ = x_ - new_delta_x;
	}


};


template<>
class Signal<1> : public Signal_base {
	/*
	* See mass spring system below: a carriage fixed at position y is
	*								connected to x via a spring with 
									spring constant = k, and a damper with damping coefficient = c
	  ||	    ||
      ||	    ||   y
	  ||        ||   |   
	  |<========>|---'
	  ||  Z  |  ||
	  ||  Z [T]---> k,c
	  ||  Z  |  ||  
	  ||  [==]------,
	  ||	    ||  |
	  ||	    || x,m
	
	|
	*/


	float x_, dx_, d2x_;
	float k_,c_, m_;
	float y_, dy_;

	const float getx() const {
		return x_;
	}

	operator float() const{
		return x_;
	}




	const float getdx() const {
		return dx_;
	}

	void advance(float dt) override {
		// m*d2x/dt^2 = f
		// f = k*(y-x) + c*diff(y-x,t)
		// f = -k*x + k*y - c*dx/dt + c*dy/dt
		// m*d2x/dt^2 + c*dx/dt +k*x = k * y + c * dydt 
		float d2x_dt2 = (1/ m_) * (k_ * (y_ - x_) + c_ * (dy_ / dt - dx_ / dt));
		// dxdt|t=t0+dt = int(d2xdt2*dt,t,t+dt)
		//float dx_dt = (.5*(d2x_prev/dt/dt) + .5*(d2x_dt2))*dt;
		float dx_dt = .5 * (d2x_ / dt) + .5 * (d2x_dt2 * dt) + dx_ * dt;
		x_ = .5 * (dx_ / dt) + .5 * (dx_dt * dt) + x_;
		dx_ = dx_dt * dt;
		d2x_ = d2x_dt2 * dt * dt;

		//turns all y into step fn, not ideal implementation
		dy_ = 0;
	}

	void sety(float y) {
		dy_ = y - y_;
		y_ = y;
	}
public:
	Signal(float x0,float y0, float k, float c, float m):
	y_(y0),
	dy_(0),
	x_(x0),
	dx_(0),
	d2x_(0),
	m_(m),
	c_(c){


	}

};

#endif // PUPPET_SIGNAL
