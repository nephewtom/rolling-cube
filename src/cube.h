#ifndef CUBE_H
#define CUBE_H

#include "raylib.h"
#include "entity.h"

struct Cube {
	
	Model model;
	PIndex pIndex;
	Vector3 position;
	Vector3 nextPosition;
	Vector3 direction; // Only uses x,z coordinates
	Vector3 moveStep;
	
	Vector3 rotationAxis;
	Vector3 rotationPivot;
	float rotationAngle;
	Matrix transform;
	Matrix accumRotations;
    
	enum State {
		QUIET, MOVING, PUSHING, PULLING, FAILPUSH
	};
	State state;
	BoxType pullingBox;
	int pushBoxesCount;
	
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
	void updateDirection();
	
	void setMoveStep(int key);
	State checkMovement(int key);
	bool isOutOfLimits(PIndex idx);
	void moveNegativeX();
	void movePositiveX();
	void moveNegativeZ();
	void movePositiveZ();
	BoxType boxInPushDirection(PIndex idx);
	BoxType boxBeyondPushDirection(PIndex idx);
	BoxType boxInPullDirection();

	void update();
	void draw ();
	void moveEnded();

	void playSound();
	Sound& pickSound();
	bool firstTimeCollisionWithShift;
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
