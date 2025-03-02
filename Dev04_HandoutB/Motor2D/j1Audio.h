#ifndef __j1AUDIO_H__
#define __j1AUDIO_H__

#include "j1Module.h"
#include "SDL_mixer\include\SDL_mixer.h"
#include "p2List.h"

#define DEFAULT_MUSIC_FADE_TIME 2.0f

struct _Mix_Music;
struct Mix_Chunk;

class j1Audio : public j1Module
{
public:

	j1Audio();

	// Destructor
	virtual ~j1Audio();

	// Called before render is available
	bool Awake(pugi::xml_node&);

	// Called before quitting
	bool CleanUp();

	// Play a music file
	bool PlayMusic(const char* path, float fade_time = DEFAULT_MUSIC_FADE_TIME);

	// Load a WAV in memory
	unsigned int LoadFx(const char* path);

	// Play a previously loaded WAV
	bool PlayFx(unsigned int fx, int repeat=0);

	void Change_Volume(float change_volume, bool up);
	
	void Change_Volume_Music(float value);
	void Change_Volume_FX(float value);

	float Music_Volume;
	float Get_Music_Volume();
	float Get_FX_Volume();

private:

	float				music_volume;
	float				fx_volume;

	_Mix_Music*			music = nullptr;

	p2List<Mix_Chunk*>	fx;
	
};

#endif // __j1AUDIO_H__