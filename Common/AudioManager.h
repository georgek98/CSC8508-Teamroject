#pragma once
#include <windows.h>
#include <irrKlang.h>
#include <stdio.h>
#include <conio.h>
#include <string>
#include <vector>
#include <iostream>
#include "Vector3.h"
using namespace irrklang;
using namespace NCL;
using namespace Maths;
class AudioManager
{
public:
	AudioManager(); 
	void PlayAudio(std::string dir, bool loop = false);
	void Play3DAudio(std::string dir, const PxTransform& t, bool loop);
	void UpdateAudio(Vector3 cameraPos);
	void StopAllSound();

	ISoundEngine* GetAudioEngine() const {
		return engine;
	}

	static int& GetVolume() {
		return volume;
	}

	void SetVolume(int val) {
		volume = val;
	}
private:
	ISoundEngine* engine;
	ISound* music;
	vec3df ListenerPos;
	static int volume;
};

