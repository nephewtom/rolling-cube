#ifndef CUBE_H
#define CUBE_H

#include "raylib.h"
#include "globals.cpp"

struct Cube {
	
	Model model;
	PositionIndex pIndex;
	Vector3 position;
	Vector3 nextPosition;
	Vector3 direction; // Only uses x,z coordinates
	Vector3 moveStep;
	
	Vector3 rotationAxis;
	Vector3 rotationOrigin;
	float rotationAngle;
	Matrix transform;
    
	enum State {
		QUIET, MOVING, PUSHING, PULLING, FAILPUSH
	};
	State state;
	BoxType movingBox;
	PositionIndex pushingBoxIndex;
	PositionIndex pullingBoxIndex;
	
	float animationProgress;
	float animationSpeed;

	Color facesColor;
	Color wiresColor;

	Sound rollWav;
	float pitchChange;
	Sound collisionWav;
	Sound pushBoxWav;
	Sound pullBoxWav;
	Sound pushFailWav;

	void playSound() {
		if (!ops.soundEnabled) return;
	
		Sound& sound = pickSound();
		if (!kb.shiftPressed || (state != QUIET && state != FAILPUSH)) {
			PlaySound(sound);
			kb.shiftTimer = 0.0f;
			return;
		}
		
		kb.shiftTimer += delta;
	
		if (kb.shiftTimer >= 0.5f) {
			PlaySound(sound);
			kb.shiftTimer = 0.0f;
		}
	}
	
	Sound& pickSound() {
	
		switch (state) {
		case MOVING: return rollWav;
		case PUSHING: return pushBoxWav;
		case PULLING: return pullBoxWav;
		case FAILPUSH: return pushFailWav;
		case QUIET: return collisionWav;
		default: return collisionWav;
		}
	}
};

struct CubeCamera {
	Camera c3d;
	float distance; // distance to target
	Vector3 direction;
	float angleX;
	float angleY;
};

#endif
