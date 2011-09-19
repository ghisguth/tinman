/* tinman.cpp
 * This file is a part of tinman project
 * Copyright (c) tinman authors (see file `COPYRIGHT` for the license)
 */

#include "tinman.hpp"

#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <fmodex/fmod_errors.h>

struct FModSystemDeleter
{
		void operator()(FMOD::System * system) const
		{
			FMOD_RESULT result = system->close();
			if(result != FMOD_OK)
			{
				std::cerr<<"FMOD close error! ("<<result<<") "<<FMOD_ErrorString(result)<<"\n";
			}
			result = system->release();
			if(result != FMOD_OK)
			{
				std::cerr<<"FMOD release error! ("<<result<<") "<<FMOD_ErrorString(result)<<"\n";
			}
		}
};

TinMan::TinMan()
{
	srand(time(0));
	result = FMOD::System_Create(&system);
	ERRCHECK();

 	unsigned int version;
	result = system->getVersion(&version);
	ERRCHECK();

	if (version < FMOD_VERSION)
	{
		std::cerr<<"Error!  You are using an old version of FMOD "<<version<<". This program requires "<<FMOD_VERSION<<"\n";
		throw "fmod version mismatch";
	}

	result = system->setOutput(FMOD_OUTPUTTYPE_ALSA);
	ERRCHECK();

	result = system->init(32, FMOD_INIT_NORMAL, 0);
	ERRCHECK();

	LoadSounds("./data/sound/", sounds);
	LoadSounds("./data/voice/", voices);
}

TinMan::~TinMan()
{
	if(system)
	{
		std::vector<FMOD::Sound*>::iterator end = sounds.end();
		for(std::vector<FMOD::Sound*>::iterator it = sounds.begin(); it != end; ++it)
		{
			(*it)->release();
		}

		end = voices.end();
		for(std::vector<FMOD::Sound*>::iterator it = voices.begin(); it != end; ++it)
		{
			(*it)->release();
		}

		result = system->close();
		if(result != FMOD_OK)
		{
			std::cerr<<"FMOD close error! ("<<result<<") "<<FMOD_ErrorString(result)<<"\n";
		}
		result = system->release();
		if(result != FMOD_OK)
		{
			std::cerr<<"FMOD release error! ("<<result<<") "<<FMOD_ErrorString(result)<<"\n";
		}
	}
}

void TinMan::Run()
{
	FMOD::Sound * primarySound = GetRandomSound();
	FMOD::Channel * primaryChannel;
	result = system->playSound(FMOD_CHANNEL_FREE, primarySound, true, &primaryChannel);
	ERRCHECK();
	primaryChannel->setVolume(0.f);
	primaryChannel->setPaused(false);

	FMOD::Sound * secondarySound = 0;
	FMOD::Channel * secondaryChannel = 0;

	const unsigned int crossfadeTime = 2500;

	time_t voiceStartTime = time(0) + 10 + (rand() % 20);

	while(true)
	{
		system->update();

		bool isPlaying = false;
		result = primaryChannel->isPlaying(&isPlaying);
		if ((result != FMOD_OK) && (result != FMOD_ERR_INVALID_HANDLE) && (result != FMOD_ERR_CHANNEL_STOLEN))
		{
				ERRCHECK();
		}

		if(isPlaying)
		{
			unsigned int primaryPos = 0;
			unsigned int primaryLen = 0;

			result = primaryChannel->getPosition(&primaryPos, FMOD_TIMEUNIT_MS);
			if ((result != FMOD_OK) && (result != FMOD_ERR_INVALID_HANDLE) && (result != FMOD_ERR_CHANNEL_STOLEN))
			{
					ERRCHECK();
			}

			result = primarySound->getLength(&primaryLen, FMOD_TIMEUNIT_MS);
			ERRCHECK();

			float volume = 1.f;
			if(primaryPos < crossfadeTime)
			{
				volume = std::max(0.f, std::min((float)(primaryPos)/(float)crossfadeTime, 1.f));
			}
			else if(primaryPos > primaryLen - crossfadeTime)
			{
				volume = std::max(0.f, std::min((float)(primaryLen - primaryPos)/(float)crossfadeTime, 1.f));
			}

			//std::cout<<std::time(0)<<" "<<voiceStartTime<<" "<<primaryPos<<"/"<<primaryLen<<" "<<volume<<"\n";

			result = primaryChannel->setVolume(volume);
			if ((result != FMOD_OK) && (result != FMOD_ERR_INVALID_HANDLE) && (result != FMOD_ERR_CHANNEL_STOLEN))
			{
					ERRCHECK();
			}

			if(primaryPos > primaryLen - crossfadeTime)
			{
				if(!secondaryChannel)
				{
					secondarySound = GetRandomSound();
					result = system->playSound(FMOD_CHANNEL_FREE, secondarySound, true, &secondaryChannel);
					ERRCHECK();
					result = secondaryChannel->setVolume(1.f - volume);
					ERRCHECK();
					result = secondaryChannel->setPaused(false);
					ERRCHECK();
				}
				else
				{
					secondaryChannel->setVolume(1.f - volume);
					if ((result != FMOD_OK) && (result != FMOD_ERR_INVALID_HANDLE) && (result != FMOD_ERR_CHANNEL_STOLEN))
					{
							ERRCHECK();
					}
				}
			}
		}
		else
		{
			if(secondaryChannel && secondarySound)
			{
				primaryChannel = secondaryChannel;
				primarySound = secondarySound;
				secondaryChannel = 0;
			}
			else
			{
				// for some reason all channels are stopped
				primarySound = GetRandomSound();
				result = system->playSound(FMOD_CHANNEL_FREE, primarySound, 0, &primaryChannel);
				ERRCHECK();
			}
		}

		if(time(0) >= voiceStartTime)
		{
			voiceStartTime = time(0) + 15 + (rand() % 25);
			PlayVoice();
		}

		boost::this_thread::sleep(boost::posix_time::milliseconds(10));
	}
}

void TinMan::PlayVoice()
{
	FMOD::Sound * voiceSound = GetRandomVoice();
	FMOD::Channel * channel;
	result = system->playSound(FMOD_CHANNEL_FREE, voiceSound, 0, &channel);
	ERRCHECK();
}

void TinMan::LoadSounds(const std::string & path, std::vector<FMOD::Sound*> & container)
{
	if(!boost::filesystem::exists(path))
	{
		std::cerr<<path<<" is not exists\n";
		throw "data not found";
	}

	boost::filesystem::directory_iterator end_itr;
	for ( boost::filesystem::directory_iterator itr(path); itr != end_itr; ++itr )
	{
		if ( !boost::filesystem::is_directory( *itr ) )
		{
			FMOD::Sound * sound;
			result = system->createSound(itr->path().string().c_str(), FMOD_SOFTWARE, 0, &sound);
			ERRCHECK();
			container.push_back(sound);
		}
	}
	if(container.empty())
	{
		std::cerr<<path<<" has no sound files\n";
		throw "data not found";
	}
}

void TinMan::ERRCHECK()
{
	if (result != FMOD_OK)
	{
			std::cerr<<"FMOD error! ("<<result<<") "<<FMOD_ErrorString(result)<<"\n";
			throw "fmod error";
	}
}

FMOD::Sound * TinMan::GetRandomSound()
{
	if(sounds.empty())
	{
		std::cerr<<"sound container is empty\n";
		throw "data";
	}
	return sounds[rand() % sounds.size()];
}

FMOD::Sound * TinMan::GetRandomVoice()
{
	if(voices.empty())
	{
		std::cerr<<"voice container is empty\n";
		throw "data";
	}
	return voices[rand() % voices.size()];
}