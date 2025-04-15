#ifndef CUBE_H
#define CUBE_H

#include "raylib.h"
#include "entity.h"

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

	void init(Vector3 v);
	void playSound();
	Sound& pickSound();
};

struct CubeCamera {
	Camera c3d;
	float distance; // distance to target
	Vector3 direction;
	float angleX;
	float angleY;
	
	void init(Vector3 v);
	void update();
};

#endif
