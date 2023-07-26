//see: https://stackoverflow.com/questions/22253074/how-to-play-or-open-mp3-or-wav-sound-file-in-c-program

#include <Windows.h>
#include <Mmsystem.h>
#include <mciapi.h>
#include <tchar.h>
//these two headers are already included in the <Windows.h> header
#pragma comment(lib, "Winmm.lib")

#include "sound.hpp"

//#define PATH_TO_AUDIO "C:\\Users\\Justin\\source\\repos\\Puppet2\\Puppet2\\assets\\audio\\"

std::string Sound::default_path = Sound::debug_path;

bool msiSendString(std::string string) {
	//code courtesy of https://stackoverflow.com/questions/6814646/convert-string-to-t-in-cpp

	const char* msg = string.c_str(); //use const char*, instead of char*
	wchar_t* wmsg = new wchar_t[strlen(msg) + 1]; //memory allocation
	size_t n_converted;
	mbstowcs_s(&n_converted, wmsg, strlen(msg) + 1, msg, strlen(msg) + 1);

	MCIERROR error = mciSendString(wmsg, NULL, 0, NULL);
	return error;
/*	TCHAR lpszErrorText[250];
	UINT cchErrorText;

	mciGetErrorString(error,lpszErrorText,250);
	//memory deallocation - must do to avoid memory leak!
	delete[]wmsg;
	return error;*/
}


bool Sound::load(){
	if (!is_loaded_) {
		std::string full_command = std::string("open \"") + std::string(path_) + fname_ + std::string("\" type mpegvideo alias ") + name_;
		if (msiSendString(full_command)) {
			is_loaded_ = true;
			return true;
		} 
	}
	return false;
}

bool Sound::unload() {
	if (is_loaded_) {
		std::string full_command = std::string("close ") + name_;
		if (msiSendString(full_command)) {
			is_loaded_ = false;
			return true;
		}
		return false;
	}
	return false;

}

bool Sound::play() {
	std::string full_command = std::string("play ") + name_ + std::string("");
	return msiSendString(full_command);
}

bool Sound::stop() {
	std::string full_command = std::string("stop ") + name_ + std::string("");
	return msiSendString(full_command);
}