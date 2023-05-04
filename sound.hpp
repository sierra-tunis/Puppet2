#pragma once

#include<string>
#ifndef PUPPET_SOUND
#define PUPPET_SOUND

class Sound {
	bool looping_;

	std::string fname_;

public:

	Sound(std::string fname);


	void play() {

	}

	void stop() {

	}

	void load() {

	}

	void unload() {

	}

};

#endif