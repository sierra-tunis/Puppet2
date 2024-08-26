//see: https://stackoverflow.com/questions/22253074/how-to-play-or-open-mp3-or-wav-sound-file-in-c-program

#include <Windows.h>
#include <Mmsystem.h>
#include <mciapi.h>
#include <tchar.h>
//these two headers are already included in the <Windows.h> header
#pragma comment(lib, "Winmm.lib")

#include "sound.hpp"
//#include <xaudio2.h>
#include <iostream>
#include <vector>


#ifdef _XBOX 
#define fourccRIFF 'RIFF'
#define fourccDATA 'data'
#define fourccFMT 'fmt '
#define fourccWAVE 'WAVE'
#define fourccXWMA 'XWMA'
#define fourccDPDS 'dpds'
#endif

#ifndef _XBOX
#define fourccRIFF 'FFIR'
#define fourccDATA 'atad'
#define fourccFMT ' tmf'
#define fourccWAVE 'EVAW'
#define fourccXWMA 'AMWX'
#define fourccDPDS 'sdpd'
#endif

//#define PATH_TO_AUDIO "C:\\Users\\Sierra\\source\\repos\\Puppet2\\Puppet2\\assets\\audio\\"

std::string Sound::default_path = Sound::debug_path;

 IXAudio2* Sound::pXAudio2;
 IXAudio2MasteringVoice* Sound::pMasterVoice;


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

void Sound::initialize() {
	HRESULT hr;
	hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (FAILED(hr)) {
		std::cout << std::hex << hr;
	}

	pXAudio2 = nullptr;
	if (FAILED(hr = XAudio2Create(&pXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR))) {
		std::cout << std::hex << hr;
	}

	pMasterVoice = nullptr;
	if (FAILED(hr = pXAudio2->CreateMasteringVoice(&pMasterVoice))) {
		std::cout << std::hex << hr;
	}
}

HRESULT FindChunk(HANDLE hFile, DWORD fourcc, DWORD& dwChunkSize, DWORD& dwChunkDataPosition)
{
	HRESULT hr = S_OK;
	if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
		return HRESULT_FROM_WIN32(GetLastError());
	DWORD dwChunkType;
	DWORD dwChunkDataSize;
	DWORD dwRIFFDataSize = 0;
	DWORD dwFileType;
	DWORD bytesRead = 0;
	DWORD dwOffset = 0;
	while (hr == S_OK)
	{
		DWORD dwRead;
		if (0 == ReadFile(hFile, &dwChunkType, sizeof(DWORD), &dwRead, NULL))
			hr = HRESULT_FROM_WIN32(GetLastError());
		if (0 == ReadFile(hFile, &dwChunkDataSize, sizeof(DWORD), &dwRead, NULL))
			hr = HRESULT_FROM_WIN32(GetLastError());
		switch (dwChunkType)
		{
		case fourccRIFF:
			dwRIFFDataSize = dwChunkDataSize;
			dwChunkDataSize = 4;
			if (0 == ReadFile(hFile, &dwFileType, sizeof(DWORD), &dwRead, NULL))
				hr = HRESULT_FROM_WIN32(GetLastError());
			break;
		default:
			if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, dwChunkDataSize, NULL, FILE_CURRENT))
				return HRESULT_FROM_WIN32(GetLastError());
		}
		dwOffset += sizeof(DWORD) * 2;
		if (dwChunkType == fourcc)
		{
			dwChunkSize = dwChunkDataSize;
			dwChunkDataPosition = dwOffset;
			return S_OK;
		}
		dwOffset += dwChunkDataSize;
		if (bytesRead >= dwRIFFDataSize) return S_FALSE;
	}
	return S_OK;
}

HRESULT ReadChunkData(HANDLE hFile, void* buffer, DWORD buffersize, DWORD bufferoffset)
{
	HRESULT hr = S_OK;
	if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, bufferoffset, NULL, FILE_BEGIN))
		return HRESULT_FROM_WIN32(GetLastError());
	DWORD dwRead;
	if (0 == ReadFile(hFile, buffer, buffersize, &dwRead, NULL))
		hr = HRESULT_FROM_WIN32(GetLastError());
	return hr;
}
bool Sound::load(){
	/*
	if (!is_loaded_) {
		std::string full_command = std::string("open \"") + std::string(path_) + fname_ + std::string("\" type mpegvideo alias ") + name_;
		if (msiSendString(full_command)) {
			is_loaded_ = true;
			return true;
		} 
	}*/

#ifdef _XBOX
	char* strFileName = fname_;
#else
	//   first, if you need a const TCHAR *

	std::string full_path = std::string(path_) + fname_;

	std::basic_string<TCHAR> converted(full_path.begin(), full_path.end());

	const TCHAR* strFileName = converted.c_str();

	//   use tchar as it is in the required form (const)


#endif
	HANDLE hFile = CreateFile(
		strFileName,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);

	if (INVALID_HANDLE_VALUE == hFile) {
		std::cout << std::hex << HRESULT_FROM_WIN32(GetLastError());
		return false;
	}

	if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN)) {
		std::cout << std::hex << HRESULT_FROM_WIN32(GetLastError());
		return false;
	}

	DWORD dwChunkSize;
	DWORD dwChunkPosition;
	//check the file type, should be fourccWAVE or 'XWMA'
	FindChunk(hFile, fourccRIFF, dwChunkSize, dwChunkPosition);
	DWORD filetype;
	ReadChunkData(hFile, &filetype, sizeof(DWORD), dwChunkPosition);
	if (filetype != fourccWAVE)
		return false;

	FindChunk(hFile, fourccFMT, dwChunkSize, dwChunkPosition);
	ReadChunkData(hFile, &wfx, dwChunkSize, dwChunkPosition);

	FindChunk(hFile, fourccDATA, dwChunkSize, dwChunkPosition);
	BYTE* pDataBuffer = new BYTE[dwChunkSize];
	ReadChunkData(hFile, pDataBuffer, dwChunkSize, dwChunkPosition);

	buffer.AudioBytes = dwChunkSize;  //size of the audio buffer in bytes
	buffer.pAudioData = pDataBuffer;  //buffer containing audio data
	buffer.Flags = XAUDIO2_END_OF_STREAM; // tell the source voice not to expect any data after this buffer

	is_loaded_ = true;
	return true;
}

bool Sound::unload() {
	/*
	if (is_loaded_) {
		std::string full_command = std::string("close ") + name_;
		if (msiSendString(full_command)) {
			is_loaded_ = false;
			return true;
		}
		return false;
	}
	*/
	return false;

}

bool Sound::play() {
	/*
	is_playing_ = true;
	std::string full_command = std::string("play ") + name_ + std::string(" from 0");
	return msiSendString(full_command);
	*/
	if (is_loaded_) {
		HRESULT hr;
		
		if (FAILED(hr = pXAudio2->CreateSourceVoice(&pSourceVoice, (WAVEFORMATEX*)&wfx))) {
			//std::cout << std::hex << hr;
			return false;
		}
		if (FAILED(hr = pSourceVoice->SubmitSourceBuffer(&buffer))) {
			//std::cout << std::hex << hr;
			return false;
		}

		if (FAILED(hr = pSourceVoice->Start(0))) {
			//std::cout << std::hex << hr;
			return false;
		}
		return true;
	}
	else return false;
}

bool Sound::playOnLoop() {

	if (is_loaded_ && !is_playing_) {
		/*std::string full_command = std::string("play ") + name_ + std::string(" from 0 repeat");
		is_playing_ = true;
		return msiSendString(full_command);
		*/
		HRESULT hr;
		if (FAILED(hr = pXAudio2->CreateSourceVoice(&pSourceVoice, (WAVEFORMATEX*)&wfx))) {
			std::cout << std::hex << hr;
			return false;
		}
		buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
		if (FAILED(hr = pSourceVoice->SubmitSourceBuffer(&buffer))) {
			std::cout << std::hex << hr;
			return false;
		}

		if (FAILED(hr = pSourceVoice->Start(0))) {
			std::cout << std::hex << hr;
			return false;
		} 
		is_playing_ = true;
		return true;
	}
	else {
		return false;
	}
}

bool Sound::stop() {
	if (is_loaded_ && is_playing_) {
		/*
		std::string full_command = std::string("stop ") + name_ + std::string("");
		is_playing_ = false;
		return msiSendString(full_command);
		*/
		pSourceVoice->Stop();
		is_playing_ = false;
	}
	else return false;
}