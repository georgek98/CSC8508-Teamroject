#include "AudioManager.h"
int AudioManager::volume = 0;
inline void sleepSomeTime() { Sleep(100); }

AudioManager::AudioManager()
{
	volume = 25.0f;
	engine = createIrrKlangDevice();
	engine->setDefault3DSoundMinDistance(20);
	music = nullptr;
}

void AudioManager::PlayAudio(std::string dir, bool loop)
{
	engine->play2D(dir.c_str(), loop);
}

//! Loads a sound source (if not loaded already) from a file and plays it as 3D sound.
/** There is some example code on how to work with 3D sound at @ref sound3d.
\param sourceFileName Filename of sound, like "sounds/test.wav" or "foobar.ogg".
\param pos Position of the 3D sound.
\param playLooped plays the sound in loop mode. If set to 'false', the sound is played once, then stopped and deleted from the internal playing list. Calls to
ISound have no effect after such a non looped sound has been stopped automaticly.*/
void AudioManager::Play3DAudio(std::string dir, const PxTransform& t, bool loop)
{
	vec3df objPos = vec3df(t.p.x, t.p.y, t.p.z);
	music = engine->play3D(dir.c_str(), objPos, loop);
}

void AudioManager::UpdateAudio(Vector3 cameraPos)
{
	engine->setSoundVolume((float)volume / 100);
	vec3df p = vec3df(cameraPos.x, cameraPos.y, cameraPos.z);
	ListenerPos = p;
	engine->setListenerPosition(p, vec3df(p.X, p.Y, p.Z + 1));
}

void AudioManager::StopAllSound()
{
	engine->stopAllSounds();
}


