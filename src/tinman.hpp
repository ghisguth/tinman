/* tinman.hpp
 * This file is a part of tinman project
 * Copyright (c) tinman authors (see file `COPYRIGHT` for the license)
 */

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