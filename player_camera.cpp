#include "player_camera.h"

void PlayerCamera::onStep() {
	Eigen::Vector3f current_state = tether_.getState();
	tether_.setState(Eigen::Vector3f(tether_.getState()(0), tether_.getState()(1), 0.0));
	Eigen::Vector<float, 3> equilibrium_state(tether_.getState()(0), tether_.getState()(1), equilibrium_length_);
	if (getParent() != nullptr) {
		tether_.boundedMove<10>(equilibrium_state, getParent()->getMotionConstraints());

		bool constraints_match = true;
		for (int i = 0; i < getParent()->getMotionConstraints().size(); i++) {
			if (i >= getMotionConstraints().size() || getParent()->getMotionConstraints()[i] != getMotionConstraints()[i]) {
				constraints_match = false;
				break;
			}
		}
		if (!constraints_match) {
			float constraint_change_snap_prevention_factor = .5;
			if (tether_.getState()(2) < current_state(2) * constraint_change_snap_prevention_factor && getMotionConstraints().size()!=0) {
				tether_.setState(Eigen::Vector3f(tether_.getState()(0), tether_.getState()(1), 0.0));
				tether_.boundedMove<10>(equilibrium_state, getMotionConstraints());
			} else {
				for (auto& m_c : getMotionConstraints()) {
					removeMotionConstraint(m_c);
				}
				for (auto& m_c : getParent()->getMotionConstraints()) {
					addMotionConstraint(m_c);
				}
			}
		}

		Eigen::Vector3f new_state = tether_.getState();
		float velocity_cushion_factor = 0.4;
		float max_velocity_cushion = .6; //max_velocity_cushion * velocity_cushion_factor must be less than max_velocity_cusion
		float cushion_kd = .9f;
		float new_velocity_cushion = std::min(max_velocity_cushion, (getdG()(seq(0, 2), 3).norm()+abs(new_state(2)-current_state(2))) * velocity_cushion_factor);
		new_velocity_cushion = std::min(max_velocity_cushion, (velocity_cushion_ - new_velocity_cushion) * cushion_kd + new_velocity_cushion);
		velocity_cushion_ = new_velocity_cushion;

		if (new_state(2) != equilibrium_length_) {
			tether_.setState(Eigen::Vector3f(tether_.getState()(0), tether_.getState()(1), 0.0));
			tether_.boundedMove<5>(new_state - Eigen::Vector3f(0, 0, .1 + new_velocity_cushion), getMotionConstraints());
		}
	}
	tether_.setState(tether_.getState());
}