#include "entity.h"
#include "globals.cpp"
#include "raymath.h"

//********** Cube & Camera
void Cube::init(Vector3 initPos) {

	model = LoadModelFromMesh(GenMeshCube(1,1,1));
	pIndex = {};
	position = initPos;
	nextPosition = initPos;
	direction = {-1.0f, 0.0f, 0.0f};
	moveStep = {0.0f, 0.0f, 0.0f};

	rotationAxis = {0.0f, 0.0f, 1.0f};
	rotationOrigin = {0.0f, 0.0f, 1.0f};
	rotationAngle = 0.0f;
	transform = MatrixIdentity();

	state = Cube::QUIET;
	pullingBox = NONE;
	pushBoxesCount = 0;
		
	animationProgress = 0.0f;
	animationSpeed = 2.5f;

	facesColor = WHITE;
	wiresColor = GREEN;

	rollWav = LoadSound("assets/sounds/roll.wav");
	pitchChange = 1.0f;
	collisionWav = LoadSound("assets/sounds/collision.wav");
	pushBoxWav =  LoadSound("assets/sounds/push.wav");
	pullBoxWav =  LoadSound("assets/sounds/pull.wav");
	pushFailWav =	LoadSound("assets/sounds/push-fail.wav");
		
	getIndexesFromPosition(pIndex, initPos);
	model.materials[0].shader = sld.shader;
	model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = sld.logo;
}

void Cube::playSound() {

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
	
Sound& Cube::pickSound() {
	
	switch (state) {
	case MOVING: return rollWav;
	case PUSHING: return pushBoxWav;
	case PULLING: return pullBoxWav;
	case FAILPUSH: return pushFailWav;
	case QUIET: return collisionWav;
	default: return collisionWav;
	}
}

void Cube::updateDirection() {
	// Calculate camera direction vector (normalized)
	camera.direction = {
		camera.c3d.target.x - camera.c3d.position.x,
		0.0f,
		camera.c3d.target.z - camera.c3d.position.z
	};
	float length = sqrtf(camera.direction.x * camera.direction.x + camera.direction.z * camera.direction.z);
	camera.direction.x /= length;
	camera.direction.z /= length;

	// Calculate dot products with world axes
	float dotX = fabsf(camera.direction.x);  // Dot product with (1,0,0)
	float dotZ = fabsf(camera.direction.z);  // Dot product with (0,0,1)

	// Determine movement direction based on camera orientation
	direction = { 0.0f, 0.0f, 0.0f };
	if (dotX > dotZ) {
		// Camera is more aligned with X axis
		direction.x = (camera.direction.x > 0) ? 1.0f : -1.0f;
	} else {
		// Camera is more aligned with Z axis
		direction.z = (camera.direction.z > 0) ? 1.0f : -1.0f;
	}
}

//********** Cube movement
void Cube::moveNegativeX() {
	moveStep = { -1.0f, 0.0f, 0.0f };
	rotationAxis = (Vector3){0.0f, 0.0f, 1.0f};
	rotationOrigin.x = position.x - 0.5f; // Left edge
	rotationOrigin.y = position.y - 0.5f; // Bottom edge
	rotationOrigin.z = position.z;
}
void Cube::movePositiveX() {
	moveStep = { 1.0f, 0.0f, 0.0f };
	rotationAxis = (Vector3){0.0f, 0.0f, -1.0f};
	rotationOrigin.x = position.x + 0.5f; // Right edge
	rotationOrigin.y = position.y - 0.5f; // Bottom edge
	rotationOrigin.z = position.z;
}
void Cube::moveNegativeZ() {
	moveStep = { 0.0f, 0.0f, -1.0f };
	rotationAxis = (Vector3){-1.0f, 0.0f, 0.0f};
	rotationOrigin.x = position.x;
	rotationOrigin.y = position.y - 0.5f; // Bottom edge
	rotationOrigin.z = position.z - 0.5f; // Front edge
}
void Cube::movePositiveZ() {
	moveStep = { 0.0f, 0.0f, 1.0f };
	rotationAxis = (Vector3){1.0f, 0.0f, 0.0f};
	rotationOrigin.x = position.x;
	rotationOrigin.y = position.y - 0.5f; // Bottom edge
	rotationOrigin.z = position.z + 0.5f; // Back edge
}

void Cube::setMoveStep(int pressedKey) {
	if (direction.x == -1.0f) {
		if (pressedKey == KEY_W) {
			moveNegativeX();
		} else if (pressedKey == KEY_S) {
			movePositiveX();
		} else if (pressedKey == KEY_A) {
			movePositiveZ();
		} else if (pressedKey == KEY_D) {
			moveNegativeZ();
		}
	} else if (direction.x == 1.0f) {
		if (pressedKey == KEY_W) {
			movePositiveX();
		} else if (pressedKey == KEY_S) {
			moveNegativeX();
		} else if (pressedKey == KEY_A) {
			moveNegativeZ();
		} else if (pressedKey == KEY_D) {
			movePositiveZ();
		}
		
	} else if (direction.z == 1.0f) {
		if (pressedKey == KEY_W) {
			movePositiveZ();
		} else if (pressedKey == KEY_S) {
			moveNegativeZ();
		} else if (pressedKey == KEY_A) {
			movePositiveX();
		} else if (pressedKey == KEY_D) {
			moveNegativeX();
		}
	} else if (direction.z == -1.0f) {
		if (pressedKey == KEY_W) {
			moveNegativeZ();
		} else if (pressedKey == KEY_S) {
			movePositiveZ();
		} else if (pressedKey == KEY_A) {
			moveNegativeX();
		} else if (pressedKey == KEY_D) {
			movePositiveX();
		}
	}
}

bool Cube::isOutOfLimits(PIndex pCheck) {
	if (pIndex.x + pCheck.x < 1 || pIndex.x + pCheck.x > ground.width - 2 ||
		pIndex.z + pCheck.z < 1 || pIndex.z + pCheck.z > ground.height - 2) {
		// Trying to move outside of limits
		LOGW("Limit reached pIndex: (%i, %i)", pIndex.x, pIndex.z);
		return true;
	}
	return false;
}


Cube::State Cube::checkMovement(int pressedKey) {
	
	setMoveStep(pressedKey);
	PIndex pStep = { (int)moveStep.x, (int)moveStep.z };

	state = QUIET;
	if (isOutOfLimits(pStep)) {
		playSound();
		return state;
	}
	
	BoxType boxInPushDir = boxInPushDirection(pStep);
	if (boxInPushDir != NONE) { // Just for debugging
		LOGD("boxInPushDir: %s", getBoxType(boxInPushDir));
	}
	
	if (boxInPushDir == OBSTACLE) {
		state = QUIET;
		playSound();
		return state;
	}

	// Movement advance and pIndex increment
	nextPosition.x = position.x + moveStep.x;
	nextPosition.y = position.y + moveStep.y;
	nextPosition.z = position.z + moveStep.z;
	pIndex.x += pStep.x;
	pIndex.z += pStep.z;
	LOGD("Updated pIndex: (%i, %i)", pIndex.x, pIndex.z);

	if (boxInPushDir == NONE) {
		BoxType boxInPullDir = boxInPullDirection();
		LOGD("boxInPullDir: %s", getBoxType(boxInPullDir));
		if (boxInPullDir == PULLBOX || boxInPullDir == PUSHPULLBOX) {
			LOGD("Pulling!");
			state = PULLING;
			playSound();
			pullingBox = boxInPullDir;
			return state;
		}
	
		LOGD("Moving!");
		rotationAngle = 0.0f;
		animationProgress = 0.0f;
		state = MOVING;
		return state;
	}

	if (boxInPushDir == PUSHBOX || boxInPushDir == PUSHPULLBOX) {

		state = PUSHING;
		playSound();
		LOGD("Cooking pushing +1 boxes, count: %i", pushBoxesCount);
		
		// Check there is not a PULLBOX in the oppositeMoveStep move direction
		BoxType boxInPullDir = boxInPullDirection();
		if (boxInPullDir == NONE) {
			// fine, the PUSHBOX can be pushed
			LOGD("Pushing!");
			state = PUSHING;
			playSound();
			LOGD("pStep: (%i, %i)", pStep.x, pStep.z);
			PIndex boxIndex = pIndex;
			for (int i=0; i<pushBoxesCount; i++) {
				int id = ground.getEntityId(boxIndex);
				Entity& ePush = entityPool.getEntity(id);
				ePush.hidden = true;
				boxIndex = boxIndex + pStep;
			}
			return state;
			
		} else {
			state = FAILPUSH;
			playSound();
			// the PUSHBOX can not be pushed if there is a PULLBOX close 
			// to the player in the opposite moveStep direction, so undo all these stuff
			PositionIndex increment = { (int) -moveStep.x, (int) -moveStep.z };
			PositionIndex boxIndex = pIndex + increment * 2;
			int id = ground.getEntityId(boxIndex);
			Entity& ePull = entityPool.getEntity(id);
			ePull.hidden = false;

			// Undo previous pIndex increment
			pIndex.x -= pStep.x;
			pIndex.z -= pStep.z;
			LOGD("Cannot push PUSH boxes if there is PULLBOX behind me!");
			nextPosition = position;
			state = QUIET;
			pushBoxesCount = 0;
			return state;
		}
	}
	
	LOGW("Should never arrive here...");
	return QUIET;
}

BoxType Cube::boxInPushDirection(PIndex pStep) {
	
	LOGD("step: (%i, %i)", pStep.x, pStep.z);
	PIndex boxIndex = { pIndex.x + pStep.x, pIndex.z + pStep.z };
	// LOGD("boxIndex: (%i, %i)", boxIndex.x, boxIndex.z);
	
	if (ground.isEmptyCell(boxIndex)) {
		if (pushBoxesCount == 0) {
			return NONE;
		} else {
			return PUSHBOX;
		}
	} 

	int id = ground.getEntityId(boxIndex);
	Entity& e = entityPool.getEntity(id);

	if (e.type == OBSTACLE || e.type == PULLBOX) {
		LOGD("OBSTACLE");
		pushBoxesCount = 0;
		return OBSTACLE;
	}
	pushBoxesCount++;
	if (pStep.x != 0) { pStep.x += sign(pStep.x); }
	else { pStep.z += sign(pStep.z); }
	
	return boxInPushDirection(pStep);
}

BoxType Cube::boxInPullDirection() {
	// we already incremented pIndex, so need to get 2 steps back
	PositionIndex increment = { (int) -moveStep.x, (int) -moveStep.z };
	PositionIndex boxIndex = pIndex + increment * 2;
	if (ground.isEmptyCell(boxIndex)) {
		return NONE;
	}
	
	int id = ground.getEntityId(boxIndex);
	Entity& e = entityPool.getEntity(id);
	if (e.type == PULLBOX || e.type == PUSHPULLBOX) {
		e.hidden = true;
		return e.type;
	}
	
	return NONE; // we don't really care what is there is it is not a pullable box
}

void Cube::update() {

	animationProgress += delta * animationSpeed;

	// Use smooth easing for animation
	float t = animationProgress;
	float smoothT = t * t * (3.0f - 2.0f * t); // Smoothstep formula

	
	if (state == Cube::MOVING) { // rotates
		// rotate 90 degrees
		rotationAngle = 90.0f * smoothT;
                    
		Matrix translateToOrigin = MatrixTranslate(-rotationOrigin.x, 
												   -rotationOrigin.y, 
												   -rotationOrigin.z);
		Matrix rotation = MatrixRotate(rotationAxis, rotationAngle * DEG2RAD);
	
		Matrix translateBack = MatrixTranslate(rotationOrigin.x, 
											   rotationOrigin.y, 
											   rotationOrigin.z);
		// Combine matrices: first translate to rotation origin, then rotate, then translate back
		transform = MatrixMultiply(translateToOrigin, rotation);
		transform = MatrixMultiply(transform, translateBack);

	} else if (state == Cube::PUSHING || state == Cube::PULLING) {
		// slides from  position to nextPosition
		position.x = position.x + (nextPosition.x - position.x) * smoothT;
		position.y = position.y + (nextPosition.y - position.y) * smoothT;
		position.z = position.z + (nextPosition.z - position.z) * smoothT;

		transform = MatrixTranslate(position.x, position.y, position.z);
	}
	
	if (animationProgress >= 1.0f) {
		moveEnded();
	}
}

//********** Drawing
void Cube::draw() {

	if (state == QUIET) {
		DrawModel(model, position, 1.0f, facesColor);
		
	} else if (state == MOVING) {
		
		rlPushMatrix();
		{
			rlMultMatrixf(MatrixToFloat(transform));
			DrawModel(model, position, 1.0f, facesColor);
		}
		rlPopMatrix();

		if (state == MOVING) { // Only show cilinder when rotating
			Vector3 vOffset = Vector3Scale(rotationAxis, 0.2);
			DrawCylinderEx(Vector3Subtract(rotationOrigin, vOffset),
						   Vector3Add(rotationOrigin, vOffset),
						   0.05f, 0.05f, 20, ORANGE);
		}

	} else if (state == PUSHING || state == PULLING) {
		// Draw THE CUBE
		DrawModel(model, position, 1.0f, facesColor);
		
		if (state == PUSHING) {
			// Draw pushing cubes
			PIndex boxIndex = pIndex;
			PIndex pStep = { (int)moveStep.x, (int)moveStep.z };
			Vector3 increment = moveStep;
			for (int i=0; i<pushBoxesCount; i++) {
				int id = ground.getEntityId(boxIndex);
				Entity& e = entityPool.getEntity(id);
				Color color = e.type == PUSHBOX ? BLUE : YELLOW;
				Vector3 pushCubePos = Vector3Add(position, increment);				
				DrawModel(model, pushCubePos, 1.0f, color);
				boxIndex = boxIndex + pStep;
				increment = Vector3Add(increment, moveStep);
			}
		}
		else if (state == PULLING) {
			Vector3 pullCubePos;
			Vector3 oppositeMoveStep = Vector3Scale(moveStep, -1.0);
			pullCubePos = Vector3Add(position, oppositeMoveStep);
			Color color = pullingBox == PULLBOX ? GREEN : YELLOW;
			DrawModel(model, pullCubePos, 1.0f, color);
		}
	}
}


void Cube::moveEnded() {
		
	position = nextPosition;
	animationProgress = 0.0f;
		
	if (state == MOVING) {
		pitchChange = KeyDelay::lerpPitch(kb.pressReleaseTime, 0.03f, 0.3f);
		SetSoundPitch(rollWav, pitchChange);
		playSound();

		rotationAngle = 0.0f;
		
	} else if (state == PUSHING) {
		
		LOGD("Cube::PUSHING ended!");
		
		PIndex increment = { (int)moveStep.x, (int)moveStep.z };
		
		for (int i = pushBoxesCount - 1; i >= 0; i--) {

			PIndex pushboxIndex = { pIndex.x + i*increment.x, pIndex.z + i*increment.z };
			int id = ground.getEntityId(pushboxIndex); LOGD("id: %i", id);
			Entity& e = entityPool.getEntity(id);
			LOGD("pushboxIndex: (%i, %i)", pushboxIndex.x, pushboxIndex.z);
			assert(pushboxIndex == e.pIndex);

			e.pIndex = e.pIndex + increment;
			e.hidden = false;
			LOGD("e.pIndex: (%i, %i)", e.pIndex.x, e.pIndex.z);
			
			ground.markEmptyCell(pushboxIndex);
			ground.markEntityInCell(e.pIndex, id);
		}
		pushBoxesCount = 0;
			
	} else if (state == PULLING) {
		LOGD("Cube::PULLING ended!");
		PIndex increment = { (int) moveStep.x, (int) moveStep.z };
		PIndex pullboxIndex = pIndex + increment * -2;
        // PULLBOX is 2 steps behind of updated posIndex, so multiply by -2
			
		LOGD("pullboxIndex: (%i, %i)", pullboxIndex.x, pullboxIndex.z);
		LOGD("pIndex: (%i, %i)", pIndex.x, pIndex.z);
		
		int id = ground.getEntityId(pullboxIndex);
		Entity& e = entityPool.getEntity(id);
		LOGD("e.pIndex: (%i, %i)", e.pIndex.x, e.pIndex.z);
		e.pIndex = e.pIndex + increment;
		LOGD("e.pIndex: (%i, %i)", e.pIndex.x, e.pIndex.z);
		e.hidden = false;
			
		ground.markEmptyCell(pullboxIndex);
		ground.markEntityInCell(e.pIndex, id);
	}
		
	state = QUIET;
}

//********** Camera
void CubeCamera::init(Vector3 initPos) {
	c3d = {
		.position = Vector3Add(initPos, Vector3({9.5f, 2.5f, 0.5f})),
		.target = initPos,
		.up = (Vector3){0.0f, 1.0f, 0.0f},
		.fovy = 45.0f,
		.projection = CAMERA_PERSPECTIVE,
	};

	// Camera orbit parameters
	Vector3 cameraOffset = { 
		c3d.position.x - c3d.target.x,
		c3d.position.y - c3d.target.y,
		c3d.position.z - c3d.target.z
	};
	distance = sqrtf(cameraOffset.x * cameraOffset.x +
					 cameraOffset.y * cameraOffset.y +
					 cameraOffset.z * cameraOffset.z);

	angleX = atan2f(cameraOffset.x, cameraOffset.z);
	angleY = asinf(cameraOffset.y / distance);
}

void CubeCamera::update() {

	// Update camera position based cube position on angles from mouse
	c3d.target = Vector3Lerp(c3d.target, cube.nextPosition, cube.animationSpeed * delta);

	c3d.position.x = c3d.target.x + distance * cosf(angleY) * sinf(angleX);
	c3d.position.y = c3d.target.y + distance * sinf(angleY);
	c3d.position.z = c3d.target.z + distance * cosf(angleY) * cosf(angleX);

// 	if (!camera.freeLight) {
// 		camera.light.target = Vector3Lerp(camera.light.target, cube.nextPosition, cube.animationSpeed * delta);
// 		camera.light.position = Vector3Lerp(camera.light.position, 
// 											Vector3Add(cube.nextPosition, camera.lightPosRelative), 
// 											cube.animationSpeed * delta);
// 	}

}
