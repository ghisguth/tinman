#pragma once

#include <vector>
#include <string>
#include <fmodex/fmod.hpp>

class TinMan
{
public:

	TinMan();
	~TinMan();

	void Run();

private:
	void ERRCHECK();

	void LoadSounds(const std::string & path, std::vector<FMOD::Sound*> & container);

	FMOD::Sound* GetRandomSound();
	FMOD::Sound* GetRandomVoice();

	void PlayVoice();

private:
	FMOD::System* system;
	std::vector<FMOD::Sound*> sounds;
	std::vector<FMOD::Sound*> voices;

	FMOD_RESULT result;
};