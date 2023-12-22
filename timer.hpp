#pragma once

#ifndef PUPPET_TIMER
#define PUPPET_TIMER

#include <chrono>

class Timer {
	bool is_running_;

	std::chrono::duration<float> time_left_;
	
protected:
	void set(std::chrono::duration<float> timer_length) {
		time_left_ = timer_length;
	}


public:

	virtual bool advance(float dt) {
		if (isRunning()) {
			time_left_ -= std::chrono::duration<float>{dt};
			if (time_left_ <= std::chrono::milliseconds{ 0 }) {
				stop();
				return true;
			}
		}
		return false;
	}

	void set(float timer_length) {
		time_left_ = std::chrono::duration<float>{timer_length};
		is_running_ = true;
	}

	void stop(){
		time_left_ = std::chrono::duration<float>{ 0 };
		is_running_ = false;
	}

	bool isRunning() const {
		return is_running_;
	}

};

class Alarm : public Timer{

	std::chrono::duration<float> alarm_length_;

public:

	bool advance(float dt) final override {
		bool timer_done = Timer::advance(dt);
		if (timer_done) {
			set(alarm_length_);
		}
		return timer_done;
	}

	Alarm(float alarm_length):alarm_length_(alarm_length_){
		set(alarm_length_);
	}


};

#endif