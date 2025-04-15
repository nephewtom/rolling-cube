#include "cube.h"

//********** Cube & Camera

Cube cube;
Vector3 cubeInitPos = {50.5f, 0.51f, 50.5f};
void initCube() {

	cube = {
		.model = LoadModelFromMesh(GenMeshCube(1,1,1)),
		.pIndex = {},
		.position = cubeInitPos,
		.nextPosition = cubeInitPos,
		.direction = {-1.0f, 0.0f, 0.0f},
		.moveStep = {0.0f, 0.0f, 0.0f},

		.rotationAxis = {0.0f, 0.0f, 1.0f},
		.rotationOrigin = {0.0f, 0.0f, 1.0f},
		.rotationAngle = 0.0f,
		.transform = MatrixIdentity(),

		.state = Cube::QUIET,
		.movingBox = NONE,
		.pushingBoxIndex = {},
		.pullingBoxIndex = {},
		
		.animationProgress = 0.0f,
		.animationSpeed = 2.5f,

		.facesColor = WHITE,
		.wiresColor = GREEN,

		.rollWav = LoadSound("assets/sounds/roll.wav"),
		.pitchChange = 1.0f,
		.collisionWav = LoadSound("assets/sounds/collision.wav"),
		.pushBoxWav =  LoadSound("assets/sounds/push.wav"),
		.pullBoxWav =  LoadSound("assets/sounds/pull.wav"),
		.pushFailWav =  LoadSound("assets/sounds/push-fail.wav"),
	};
		
	getIndexesFromPosition(cube.pIndex, cubeInitPos);
	cube.model.materials[0].shader = shader;
	cube.model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = logo;
}

CubeCamera camera;
void initCamera() {
	camera.c3d = {
		.position = Vector3Add(cubeInitPos, Vector3({9.5f, 2.5f, 0.5f})),
		.target = cubeInitPos,
		.up = (Vector3){0.0f, 1.0f, 0.0f},
		.fovy = 45.0f,
		.projection = CAMERA_PERSPECTIVE,
	};

	// Camera orbit parameters
	Vector3 cameraOffset = { 
		camera.c3d.position.x - camera.c3d.target.x,
		camera.c3d.position.y - camera.c3d.target.y,
		camera.c3d.position.z - camera.c3d.target.z
	};
	camera.distance = sqrtf(cameraOffset.x * cameraOffset.x +
							cameraOffset.y * cameraOffset.y +
							cameraOffset.z * cameraOffset.z);

	camera.angleX = atan2f(cameraOffset.x, cameraOffset.z);
	camera.angleY = asinf(cameraOffset.y / camera.distance);
}

void updateCubeCamera() {

	// Update camera position based cube position on angles from mouse
	camera.c3d.target = Vector3Lerp(camera.c3d.target, cube.nextPosition, cube.animationSpeed * delta);

	camera.c3d.position.x = camera.c3d.target.x + camera.distance * cosf(camera.angleY) * sinf(camera.angleX);
	camera.c3d.position.y = camera.c3d.target.y + camera.distance * sinf(camera.angleY);
	camera.c3d.position.z = camera.c3d.target.z + camera.distance * cosf(camera.angleY) * cosf(camera.angleX);

// 	if (!camera.freeLight) {
// 		camera.light.target = Vector3Lerp(camera.light.target, cube.nextPosition, cube.animationSpeed * delta);
// 		camera.light.position = Vector3Lerp(camera.light.position, 
// 											Vector3Add(cube.nextPosition, camera.lightPosRelative), 
// 											cube.animationSpeed * delta);
// 	}

}

void mouseUpdateCubeDirection() {
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
	cube.direction = { 0.0f, 0.0f, 0.0f };
	if (dotX > dotZ) {
		// Camera is more aligned with X axis
		cube.direction.x = (camera.direction.x > 0) ? 1.0f : -1.0f;
	} else {
		// Camera is more aligned with Z axis
		cube.direction.z = (camera.direction.z > 0) ? 1.0f : -1.0f;
	}
}

//********** Cube movement
void moveNegativeX() {
	cube.moveStep = { -1.0f, 0.0f, 0.0f };
	cube.rotationAxis = (Vector3){0.0f, 0.0f, 1.0f};
	cube.rotationOrigin.x = cube.position.x - 0.5f; // Left edge
	cube.rotationOrigin.y = cube.position.y - 0.5f; // Bottom edge
	cube.rotationOrigin.z = cube.position.z;
}
void movePositiveX() {
	cube.moveStep = { 1.0f, 0.0f, 0.0f };
	cube.rotationAxis = (Vector3){0.0f, 0.0f, -1.0f};
	cube.rotationOrigin.x = cube.position.x + 0.5f; // Right edge
	cube.rotationOrigin.y = cube.position.y - 0.5f; // Bottom edge
	cube.rotationOrigin.z = cube.position.z;
}
void moveNegativeZ() {
	cube.moveStep = { 0.0f, 0.0f, -1.0f };
	cube.rotationAxis = (Vector3){-1.0f, 0.0f, 0.0f};
	cube.rotationOrigin.x = cube.position.x;
	cube.rotationOrigin.y = cube.position.y - 0.5f; // Bottom edge
	cube.rotationOrigin.z = cube.position.z - 0.5f; // Front edge
}
void movePositiveZ() {
	cube.moveStep = { 0.0f, 0.0f, 1.0f };
	cube.rotationAxis = (Vector3){1.0f, 0.0f, 0.0f};
	cube.rotationOrigin.x = cube.position.x;
	cube.rotationOrigin.y = cube.position.y - 0.5f; // Bottom edge
	cube.rotationOrigin.z = cube.position.z + 0.5f; // Back edge
}

BoxType getBoxBeyondPushDirection(int xi, int zi) {
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

BoxType getBoxInPushDirection(int xi, int zi) {
	
	// LOGD("cube.pIndex: (%i, %i)", cube.pIndex.x, cube.pIndex.z);
	PositionIndex boxIndex = { cube.pIndex.x + xi, cube.pIndex.z+zi };
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

	if (getBoxBeyondPushDirection(xi, zi) == OBSTACLE) {
		return OBSTACLE;
	}
	e.hidden = true;
	cube.pushingBoxIndex = boxIndex;
	return e.type;
}

BoxType getBoxInPullDirection() {
	// we already incremented cube.pIndex, so need to get 2 steps back
	PositionIndex increment = { (int) -cube.moveStep.x, (int) -cube.moveStep.z };
	PositionIndex boxIndex = cube.pIndex + increment * 2;
	if (ground.cells[boxIndex.x][boxIndex.z].isEmpty) {
		return NONE;
	}
	
	int id = ground.cells[boxIndex.x][boxIndex.z].entityId;
	Entity& e = entityPool.getEntity(id);
	
	if (e.type == PULLBOX || e.type == PUSHPULLBOX) {
		e.hidden = true;
		cube.pullingBoxIndex = boxIndex;
		return e.type;
	}
	
	return NONE; // we don't really care what is there is it is not a pullable box
}

void calculateCubeMovement(int pressedKey) {
	
	if (cube.direction.x == -1.0f) {
		if (pressedKey == KEY_W) {
			moveNegativeX();
		} else if (pressedKey == KEY_S) {
			movePositiveX();
		} else if (pressedKey == KEY_A) {
			movePositiveZ();
		} else if (pressedKey == KEY_D) {
			moveNegativeZ();
		}
	} else if (cube.direction.x == 1.0f) {
		if (pressedKey == KEY_W) {
			movePositiveX();
		} else if (pressedKey == KEY_S) {
			moveNegativeX();
		} else if (pressedKey == KEY_A) {
			moveNegativeZ();
		} else if (pressedKey == KEY_D) {
			movePositiveZ();
		}
		
	} else if (cube.direction.z == 1.0f) {
		if (pressedKey == KEY_W) {
			movePositiveZ();
		} else if (pressedKey == KEY_S) {
			moveNegativeZ();
		} else if (pressedKey == KEY_A) {
			movePositiveX();
		} else if (pressedKey == KEY_D) {
			moveNegativeX();
		}
	} else if (cube.direction.z == -1.0f) {
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

	int xi = (int)cube.moveStep.x;
	int zi = (int)cube.moveStep.z;
	
	BoxType boxInPushDir = getBoxInPushDirection(xi, zi);
	if (boxInPushDir != NONE) {
		LOGD("boxInPushDir: %s", getBoxType(boxInPushDir));
	}
	
	if (boxInPushDir == OBSTACLE) {
		cube.state = Cube::QUIET;
		cube.playSound();
		return;
	}

	// Movement advance and pIndex increment
	cube.nextPosition.x = cube.position.x + cube.moveStep.x;
	cube.nextPosition.y = cube.position.y + cube.moveStep.y;
	cube.nextPosition.z = cube.position.z + cube.moveStep.z;
	cube.pIndex.x += xi;
	cube.pIndex.z += zi;

	LOGD("Updated cube.pIndex: (%i, %i)", cube.pIndex.x, cube.pIndex.z);

	if (boxInPushDir == NONE) {
		BoxType boxInPullDir = getBoxInPullDirection();
		LOGD("boxInPullDir: %s", getBoxType(boxInPullDir));
		if (boxInPullDir == PULLBOX || boxInPullDir == PUSHPULLBOX) {
			LOGD("Pulling!");
			cube.state = Cube::PULLING;
			cube.playSound();
			cube.movingBox = boxInPullDir;
			return;
		}
	
		cube.rotationAngle = 0.0f;
		cube.animationProgress = 0.0f;
		cube.state = Cube::MOVING;
		return;
	}

	if (boxInPushDir == PUSHBOX || boxInPushDir == PUSHPULLBOX) {
		cube.movingBox = boxInPushDir;
		// Check there is not a PULLBOX in the oppositeMoveStep move direction
		BoxType boxInPullDir = getBoxInPullDirection();
		if (boxInPullDir == NONE) {
			// fine, the PUSHBOX can be pushed
			LOGD("Pushing!");
			cube.state = Cube::PUSHING;
			cube.playSound();
			return;
		} else {
			cube.state = Cube::FAILPUSH;
			cube.playSound();
			// the PUSHBOX can not be pushed if there is a PULLBOX close 
			// to the player in the opposite moveStep direction, so undo all these stuff
			int id = ground.cells[cube.pushingBoxIndex.x][cube.pushingBoxIndex.z].entityId;
			Entity& ePush = entityPool.getEntity(id);
			ePush.hidden = false;
			id = ground.cells[cube.pullingBoxIndex.x][cube.pullingBoxIndex.z].entityId;
			Entity& ePull = entityPool.getEntity(id);
			ePull.hidden = false;
			cube.movingBox = NONE;
            // Undo previous cube.pIndex increment
			cube.pIndex.x -= xi;
			cube.pIndex.z -= zi;
			LOGD("Cannot push a PUSHBOX if there is PULLBOX behind me!");
			cube.nextPosition = cube.position;
			cube.state = Cube::QUIET;
			return;
		}
	}
	

	LOGW("Should never arrive here...");
	// cube.state = Cube::QUIET;
}

void animationEnded() {
		
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

void updateCubeMovement() {

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
void drawRollingCube() {

	if (cube.state == Cube::QUIET) {
		DrawModel(cube.model, cube.position, 1.0f, cube.facesColor);
		
	} else if (cube.state == Cube::MOVING) {
		
		rlPushMatrix();
		{
			rlMultMatrixf(MatrixToFloat(cube.transform));
			DrawModel(cube.model, cube.position, 1.0f, cube.facesColor);
		}
		rlPopMatrix();

		if (cube.state == Cube::MOVING) { // Only show cilinder when rotating
			Vector3 vOffset = Vector3Scale(cube.rotationAxis, 0.2);
			DrawCylinderEx(Vector3Subtract(cube.rotationOrigin, vOffset),
						   Vector3Add(cube.rotationOrigin, vOffset),
						   0.05f, 0.05f, 20, ORANGE);
		}

	} else if (cube.state == Cube::PUSHING || cube.state == Cube::PULLING) {
		DrawModel(cube.model, cube.position, 1.0f, cube.facesColor);
		
		Vector3 otherCubePos;
		if (cube.state == Cube::PUSHING) {
			otherCubePos = Vector3Add(cube.position, cube.moveStep);
		}
		else if (cube.state == Cube::PULLING) {
			Vector3 oppositeMoveStep = Vector3Scale(cube.moveStep, -1.0);
			otherCubePos = Vector3Add(cube.position, oppositeMoveStep);
		}
		
		Color color = 
			cube.movingBox == PUSHBOX ? BLUE :
			cube.movingBox == PULLBOX ? GREEN :
			cube.movingBox == PUSHPULLBOX ? YELLOW:
			MAGENTA;

		DrawModel(cube.model, otherCubePos, 1.0f, color);
	}
}
