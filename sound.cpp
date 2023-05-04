//see: https://stackoverflow.com/questions/22253074/how-to-play-or-open-mp3-or-wav-sound-file-in-c-program

#include <Windows.h>
#include <Mmsystem.h>
#include <mciapi.h>
#include <tchar.h>
//these two headers are already included in the <Windows.h> header
#pragma comment(lib, "Winmm.lib")

#include "sound.hpp"

#define PATH_TO_AUDIO "C:\\Users\\Justin\\source\\repos\\Puppet2\\Puppet2\\assets\\audio\\"

void msiSendString(std::string string) {
	//code courtesy of https://stackoverflow.com/questions/6814646/convert-string-to-t-in-cpp

	const char* msg = string.c_str(); //use const char*, instead of char*
	wchar_t* wmsg = new wchar_t[strlen(msg) + 1]; //memory allocation
	size_t n_converted;
	mbstowcs_s(&n_converted, wmsg, strlen(msg) + 1, msg, strlen(msg) + 1);

	MCIERROR error = mciSendString(wmsg, NULL, 0, NULL);
	TCHAR lpszErrorText[250];
	UINT cchErrorText;

	mciGetErrorString(error,lpszErrorText,250);
	//memory deallocation - must do to avoid memory leak!
	delete[]wmsg;
}


void Sound::load(){
	std::string full_command = std::string("open \"") + std::string(PATH_TO_AUDIO) + fname_ + std::string("\" type mpegvideo alias ") + name_;
	msiSendString(full_command);
}

void Sound::unload() {

	std::string full_command = std::string("close ") + name_;
	msiSendString(full_command);


}

void Sound::play() {
	std::string full_command = std::string("play ") + name_ + std::string("");
	msiSendString(full_command);


}