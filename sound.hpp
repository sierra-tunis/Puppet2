#pragma once

#include<string>
#ifndef PUPPET_SOUND
#define PUPPET_SOUND


class Sound {
	bool looping_;

	std::string fname_;
	std::string name_;



public:

	Sound(std::string name, std::string fname) :name_(name), fname_(fname) {}


	void play();

	void stop() {

	}

	void load();

	void unload();

};

#endif