#include "player_camera.h"

void PlayerCamera::onStep() {

	tether_.setState(Eigen::Vector3f(tether_.getState()(0), tether_.getState()(1), 0.0));
	Eigen::Vector<float, 3> equilibrium_state(tether_.getState()(0), tether_.getState()(1), equilibrium_length_);
	if (getParent() != nullptr) {
		tether_.boundedMove<10>(equilibrium_state, getParent()->getMotionConstraints());
		Eigen::Vector3f new_state = tether_.getState();

		float velocity_cushion_factor = 0.2;
		float max_velocity_cushion = .4; //max_velocity_cushion * velocity_cushion_factor must be less than max_velocity_cusion
		float cushion_kd = .75f;
		float new_velocity_cushion = std::min(max_velocity_cushion, getdG()(seq(0, 2), 3).norm() * velocity_cushion_factor);
		new_velocity_cushion = std::min(max_velocity_cushion, (velocity_cushion_ - new_velocity_cushion) * cushion_kd + new_velocity_cushion);
		velocity_cushion_ = new_velocity_cushion;

		if (new_state(2) != equilibrium_length_) {
			tether_.setState(Eigen::Vector3f(tether_.getState()(0), tether_.getState()(1), 0.0));
			tether_.boundedMove<5>(new_state - Eigen::Vector3f(0, 0, .1 + new_velocity_cushion), getParent()->getMotionConstraints());
		}
	}
	tether_.setState(tether_.getState());
}