#include "globals.cpp"

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
	movingBox = NONE;
	pushingBoxIndex = {};
	pullingBoxIndex = {};
		
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

Cube::State Cube::calculateMovement(int pressedKey) {
	
	setMoveStep(pressedKey);

	int xi = (int)moveStep.x;
	int zi = (int)moveStep.z;

	if (pIndex.x + xi < 1 || pIndex.x+xi > ground.width - 2 ||
		pIndex.z + zi < 1 || pIndex.z+zi > ground.height - 2) {
		LOGW("Limit pIndex: (%i, %i)", pIndex.x, pIndex.z);
		state = QUIET;
		playSound();
		return state;
	}
	
	BoxType boxInPushDir = boxInPushDirection(xi, zi);
	if (boxInPushDir != NONE) {
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
	pIndex.x += xi;
	pIndex.z += zi;

	LOGD("Updated pIndex: (%i, %i)", pIndex.x, pIndex.z);

	if (boxInPushDir == NONE) {
		BoxType boxInPullDir = boxInPullDirection();
		// LOGD("boxInPullDir: %s", getBoxType(boxInPullDir));
		if (boxInPullDir == PULLBOX || boxInPullDir == PUSHPULLBOX) {
			LOGD("Pulling!");
			state = PULLING;
			playSound();
			movingBox = boxInPullDir;
			return state;
		}
	
		rotationAngle = 0.0f;
		animationProgress = 0.0f;
		state = MOVING;
		return state;
	}

	if (boxInPushDir == PUSHBOX || boxInPushDir == PUSHPULLBOX) {
		movingBox = boxInPushDir;
		// Check there is not a PULLBOX in the oppositeMoveStep move direction
		BoxType boxInPullDir = boxInPullDirection();
		if (boxInPullDir == NONE) {
			// fine, the PUSHBOX can be pushed
			LOGD("Pushing!");
			state = PUSHING;
			playSound();
			return state;
		} else {
			state = FAILPUSH;
			playSound();
			// the PUSHBOX can not be pushed if there is a PULLBOX close 
			// to the player in the opposite moveStep direction, so undo all these stuff
			int id = ground.cells[pushingBoxIndex.x][pushingBoxIndex.z].entityId;
			Entity& ePush = entityPool.getEntity(id);
			ePush.hidden = false;
			id = ground.cells[pullingBoxIndex.x][pullingBoxIndex.z].entityId;
			Entity& ePull = entityPool.getEntity(id);
			ePull.hidden = false;
			movingBox = NONE;
            // Undo previous pIndex increment
			pIndex.x -= xi;
			pIndex.z -= zi;
			LOGD("Cannot push a PUSHBOX if there is PULLBOX behind me!");
			nextPosition = position;
			state = QUIET;
			return state;
		}
	}
	
	LOGW("Should never arrive here...");
	return QUIET;
}

BoxType Cube::boxBeyondPushDirection(int xi, int zi) {
	int xii = 0, zii = 0;
	if (xi != 0) xii = xi;
	else if (zi != 0) zii = zi;
	bool isEmpty =  ground.cells[cube.pIndex.x + xi + xii][cube.pIndex.z + zi + zii].isEmpty;
	if (!isEmpty) { // check if pushbox has another entity in the move direction
		LOGD("OBSTACLE");
		return OBSTACLE;
	}

	return NONE;
}

BoxType Cube::boxInPushDirection(int xi, int zi) {
	
	// LOGD("cube.pIndex: (%i, %i)", cube.pIndex.x, cube.pIndex.z);
	PositionIndex boxIndex = { pIndex.x + xi, pIndex.z+zi };
	// LOGD("boxIndex: (%i, %i)", boxIndex.x, boxIndex.z);
	
	bool isEmpty =  ground.cells[boxIndex.x][boxIndex.z].isEmpty;
	if (isEmpty) {
		return NONE;
	} 

	int id = ground.cells[boxIndex.x][boxIndex.z].entityId;
	Entity& e = entityPool.getEntity(id);

	if (e.type == OBSTACLE || e.type == PULLBOX) {
		LOGD("OBSTACLE");
		return OBSTACLE;
	}

	if (boxBeyondPushDirection(xi, zi) == OBSTACLE) {
		return OBSTACLE;
	}
	e.hidden = true;
	pushingBoxIndex = boxIndex;
	return e.type;
}

BoxType Cube::boxInPullDirection() {
	// we already incremented cube.pIndex, so need to get 2 steps back
	PositionIndex increment = { (int) -moveStep.x, (int) -moveStep.z };
	PositionIndex boxIndex = pIndex + increment * 2;
	if (ground.cells[boxIndex.x][boxIndex.z].isEmpty) {
		return NONE;
	}
	
	int id = ground.cells[boxIndex.x][boxIndex.z].entityId;
	Entity& e = entityPool.getEntity(id);
	
	if (e.type == PULLBOX || e.type == PUSHPULLBOX) {
		e.hidden = true;
		pullingBoxIndex = boxIndex;
		return e.type;
	}
	
	return NONE; // we don't really care what is there is it is not a pullable box
}

void Cube::animationEnded() {
		
	cube.position = cube.nextPosition;
	cube.animationProgress = 0.0f;
		
	if (cube.state == Cube::MOVING) {
		cube.pitchChange = KeyDelay::lerpPitch(kb.pressReleaseTime, 0.03f, 0.3f);
		SetSoundPitch(cube.rollWav, cube.pitchChange);
		cube.playSound();

		cube.rotationAngle = 0.0f;
		
	} else if (cube.state == Cube::PUSHING) {
		LOGD("Cube::PUSHING ended!");
		int id = ground.cells[cube.pIndex.x][cube.pIndex.z].entityId;
		LOGD("id: %i", id);
		Entity& e = entityPool.getEntity(id);
		LOGD("cube.pIndex: (%i, %i)", cube.pIndex.x, cube.pIndex.z);
		LOGD("e.pIndex: (%i, %i)", e.pIndex.x, e.pIndex.z);
			
		PositionIndex increment = { (int)cube.moveStep.x, (int)cube.moveStep.z };
		e.pIndex = e.pIndex + increment;
		LOGD("e.pIndex: (%i, %i)", e.pIndex.x, e.pIndex.z);
		e.hidden = false;

		// Check it is updated
		Entity& e2 = entityPool.getEntity(id);
		LOGD("e2.pIndex: (%i, %i)", e2.pIndex.x, e2.pIndex.z);
			
		// update ground
		ground.cells[cube.pIndex.x][cube.pIndex.z].entityId = -1;
		ground.cells[cube.pIndex.x][cube.pIndex.z].isEmpty = true;
		ground.cells[e.pIndex.x][e.pIndex.z].entityId = id;
		ground.cells[e.pIndex.x][e.pIndex.z].isEmpty = false;
			
		cube.movingBox = NONE;
			
	} else if (cube.state == Cube::PULLING) {
		LOGD("Cube::PULLING ended!");
		PositionIndex increment = { (int) cube.moveStep.x, (int) cube.moveStep.z };
		PositionIndex pullboxIndex = cube.pIndex + increment * -2;
        // PULLBOX is 2 steps behind of updated cube posIndex, so multiply by -2
			
		LOGD("pullboxIndex: (%i, %i)", pullboxIndex.x, pullboxIndex.z);
		LOGD("cube.pIndex: (%i, %i)", cube.pIndex.x, cube.pIndex.z);
		
		int id = ground.cells[pullboxIndex.x][pullboxIndex.z].entityId;
		Entity& e = entityPool.getEntity(id);
		LOGD("e.pIndex: (%i, %i)", e.pIndex.x, e.pIndex.z);
		e.pIndex = e.pIndex + increment;
		LOGD("e.pIndex: (%i, %i)", e.pIndex.x, e.pIndex.z);
		e.hidden = false;
			
		// update ground
		ground.cells[pullboxIndex.x][pullboxIndex.z].entityId = -1;
		ground.cells[pullboxIndex.x][pullboxIndex.z].isEmpty = true;
		ground.cells[e.pIndex.x][e.pIndex.z].entityId = id;
		ground.cells[e.pIndex.x][e.pIndex.z].isEmpty = false;
			
		cube.movingBox = NONE;
	}
		
	cube.state = Cube::QUIET;
}

void Cube::update() {

	cube.animationProgress += delta * cube.animationSpeed;

	// Use smooth easing for animation
	float t = cube.animationProgress;
	float smoothT = t * t * (3.0f - 2.0f * t); // Smoothstep formula

	
	if (cube.state == Cube::MOVING) { // rotates
		// rotate 90 degrees
		cube.rotationAngle = 90.0f * smoothT;
                    
		Matrix translateToOrigin = MatrixTranslate(-cube.rotationOrigin.x, 
												   -cube.rotationOrigin.y, 
												   -cube.rotationOrigin.z);
		Matrix rotation = MatrixRotate(cube.rotationAxis, cube.rotationAngle * DEG2RAD);
	
		Matrix translateBack = MatrixTranslate(cube.rotationOrigin.x, 
											   cube.rotationOrigin.y, 
											   cube.rotationOrigin.z);
		// Combine matrices: first translate to rotation origin, then rotate, then translate back
		cube.transform = MatrixMultiply(translateToOrigin, rotation);
		cube.transform = MatrixMultiply(cube.transform, translateBack);

	} else if (cube.state == Cube::PUSHING || cube.state == Cube::PULLING) {
		// slides from  cube.position to cube.nextPosition
		cube.position.x = cube.position.x + (cube.nextPosition.x - cube.position.x) * smoothT;
		cube.position.y = cube.position.y + (cube.nextPosition.y - cube.position.y) * smoothT;
		cube.position.z = cube.position.z + (cube.nextPosition.z - cube.position.z) * smoothT;

		cube.transform = MatrixTranslate(cube.position.x, 
										 cube.position.y, 
										 cube.position.z);
	}
	
	if (cube.animationProgress >= 1.0f) {
		animationEnded();
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
		DrawModel(model, position, 1.0f, facesColor);
		
		Vector3 otherCubePos;
		if (state == PUSHING) {
			otherCubePos = Vector3Add(position, moveStep);
		}
		else if (state == PULLING) {
			Vector3 oppositeMoveStep = Vector3Scale(moveStep, -1.0);
			otherCubePos = Vector3Add(position, oppositeMoveStep);
		}
		
		Color color = 
			movingBox == PUSHBOX ? BLUE :
			movingBox == PULLBOX ? GREEN :
			movingBox == PUSHPULLBOX ? YELLOW:
			MAGENTA;

		DrawModel(model, otherCubePos, 1.0f, color);
	}
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
