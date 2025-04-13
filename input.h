#ifndef INPUT_H
#define INPUT_H

#include "raylib.h"
#include "raymath.h"

struct Keyboard {
	int pressedKey;
	double keyPressTime;
	double keyReleaseTime;
	float pressReleaseTime; // Between press and release
	float lastPressedKeyTime;
		
	bool hasQueuedKey;
	int queuedKey;
	bool shiftPressed;
	float shiftTimer;
	
	bool instancingEnabled;
};


class KeyDelay {
public:
    static constexpr float MinSpeed = 1.0f;
    static constexpr float MaxSpeed = 10.0f;

	// return interpolated speed taking Press/Release time as input
    static float lerpSpeed(float time, float minTime, float maxTime) {
        time = Clamp(time, minTime, maxTime);
        return MinSpeed + (MaxSpeed - MinSpeed) * ((maxTime - time) / (maxTime - minTime));
    }

    static constexpr float MinPitch = 0.75f;
    static constexpr float MaxPitch = 2.0f;    
    static float lerpPitch(float time, float minTime, float maxTime) {
        time = Clamp(time, minTime, maxTime);
        return MinPitch + (MaxPitch - MinPitch) * ((maxTime - time) / (maxTime - minTime));
    }
    
};

struct Mouse {
	Vector2 position;
	Vector2 prevPosition;
	Vector2 deltaPosition;
	
	bool cursorHidden;
	
	bool zoomEnabled;
	float zoomSpeed;
	float minZoomDistance;
	float maxZoomDistance;
};


#endif
