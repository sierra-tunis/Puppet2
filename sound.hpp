#pragma once

#ifndef PUPPET_SOUND
#define PUPPET_SOUND
#include<string>
#define NOMINMAX
#include<xaudio2.h>

class Sound {
	bool looping_;

	std::string fname_;
	std::string path_;
	std::string name_;

	bool is_loaded_;
	bool is_playing_;

	DWORD filetype_;
	
	WAVEFORMATEXTENSIBLE wfx; 
	XAUDIO2_BUFFER buffer;
	
	IXAudio2SourceVoice* pSourceVoice;

	static IXAudio2* pXAudio2;
	static IXAudio2MasteringVoice* pMasterVoice;


public:

	static constexpr char debug_path[] = "C:\\Users\\Sierra\\source\\repos\\Puppet2\\Puppet2\\assets\\";
	static std::string default_path;

	static void initialize();

	Sound() :name_(""), fname_("default_constructor_noise.wav"),is_loaded_(false),is_playing_(false) {};

	Sound(std::string name, std::string fname, std::string path=default_path) :
		name_(name),
		path_(path),
		fname_(fname),
		is_loaded_(false),
		is_playing_(false),
		wfx{ 0 },
		buffer{ 0 } {}

	bool play();

	bool playOnLoop();

	bool stop();

	bool load();

	bool unload();
};

#endif