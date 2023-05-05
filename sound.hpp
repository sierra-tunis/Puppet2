#pragma once

#include<string>
#ifndef PUPPET_SOUND
#define PUPPET_SOUND


class Sound {
	bool looping_;

	std::string fname_;
	std::string name_;

	bool is_loaded_;
	bool is_playing_;


public:

	Sound() :name_(""), fname_("default_constructor_noise.wav"),is_loaded_(false),is_playing_(false) {};

	Sound(std::string name, std::string fname) :name_(name), fname_(fname), is_loaded_(false), is_playing_(false)  {}


	bool play();

	bool stop();

	bool load();

	bool unload();

};

#endif