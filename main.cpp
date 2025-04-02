#include "raylib.h"
#include <cstddef>
#define SUPPORT_TRACELOG
#define SUPPORT_TRACELOG_DEBUG
#include "utils.h"

#include "raymath.h" // Needed for matrix and quaternion operations
#include "rlgl.h"

#include "imgui.h"
#include "rlImGui.h"
bool imgui_demo_window = false;

#include <math.h>
#include <stdarg.h>
#include <stdio.h>

#include "rlights.c"
#define GLSL_VERSION 330

//********** Shaders and Textures

Shader shader;
Vector4 ambient = { 0.1f, 0.1f, 0.1f, 1.0f };
int ambientLoc;
void loadShader() {
	shader = LoadShader(TextFormat("shaders/lighting_with_instancing.vs", GLSL_VERSION),
						TextFormat("shaders/lighting_with_instancing.fs", GLSL_VERSION));
	shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");

	ambientLoc = GetShaderLocation(shader, "ambient");
	SetShaderValue(shader, ambientLoc, &ambient, SHADER_UNIFORM_VEC4);
}

Texture logo;
Texture logoGround;
void loadTextures() {
	logo = LoadTexture("assets/logo.png");
	logoGround = LoadTexture("assets/logo-ground.png");
	
	// Image image = LoadImage("shaders/cube_ao.png"); // Load image
	// ImageFlipVertical(&image); // Flip the image vertically
	// Texture texture = LoadTextureFromImage(image); // Convert to texture
}

#include "cubemap.c"
struct Skybox {
	Mesh cube;
	Model model;
	Shader cubemap;
};
void loadSkybox(Skybox& skybox) {
	skybox.cube = GenMeshCube(1.0f, 1.0f, 1.0f);
	skybox.model = LoadModelFromMesh(skybox.cube);
	skybox.model.materials[0].shader = LoadShader(TextFormat("shaders/skybox.vs", GLSL_VERSION),
												  TextFormat("shaders/skybox.fs", GLSL_VERSION));
	
	int envMapLoc = GetShaderLocation(skybox.model.materials[0].shader, "environmentMap");
	int mmc[1] = {MATERIAL_MAP_CUBEMAP};
	int valueOne[1] = { 1 }; 
	int valueZero[1] = { 0 }; 
	SetShaderValue(skybox.model.materials[0].shader, envMapLoc, mmc, SHADER_UNIFORM_INT);
	int doGammaLoc = GetShaderLocation(skybox.model.materials[0].shader, "doGamma");
	SetShaderValue(skybox.model.materials[0].shader, doGammaLoc, valueOne, SHADER_UNIFORM_INT);
	int vflippedLoc = GetShaderLocation(skybox.model.materials[0].shader, "vflipped");
	SetShaderValue(skybox.model.materials[0].shader, vflippedLoc, valueOne, SHADER_UNIFORM_INT);
	
	skybox.cubemap = LoadShader(TextFormat("shaders/cubemap.vs", GLSL_VERSION),
								TextFormat("shaders/cubemap.fs", GLSL_VERSION));
	int equiMapLoc = GetShaderLocation(skybox.cubemap, "equirectangularMap");
	SetShaderValue(skybox.cubemap, equiMapLoc, valueZero, SHADER_UNIFORM_INT);

	Texture2D panorama = LoadTexture("assets/hdr/krita_oilpaint-sunflowers_puresky_4k.hdr");

	skybox.model.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture = 
		GenTextureCubemap(skybox.cubemap, panorama, 1024, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
	UnloadTexture(panorama);
}

void updateSkybox(Skybox& skybox) {
	FilePathList droppedFiles = LoadDroppedFiles();
	if (droppedFiles.count != 1) return;
	if (!IsFileExtension(droppedFiles.paths[0], ".hdr")) return;
	UnloadTexture(skybox.model.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture);
	Texture2D panorama = LoadTexture(droppedFiles.paths[0]);
	skybox.model.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture = 
		GenTextureCubemap(skybox.cubemap, panorama, 1024, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
	UnloadTexture(panorama);
	UnloadDroppedFiles(droppedFiles);
}

//********** Ground
const int X_CELLS = 101;
const int Z_CELLS = 101;
const int BEGIN_CELL_POS = -50;
const int END_CELL_POS = 50;

struct Cell {
	bool isEmpty;
	int entityId;
	Color color;
};
struct Ground {
	Mesh plane;
	Model model;
	Material material;

	Matrix *transforms;
	
	int maxCellIndex;
	Cell cells[X_CELLS][Z_CELLS];
};
Ground ground;

Color getRandomColor()
{
	return (Color){
		(unsigned char) GetRandomValue(0, 255), // Red
		(unsigned char) GetRandomValue(0, 255), // Green
		(unsigned char) GetRandomValue(0, 255), // Blue
		255 // Alpha (fully opaque)
	};
}

void initGround() {
	
	ground.plane = GenMeshPlane(1.0f,1.0f,1,1);
	ground.model = LoadModelFromMesh(ground.plane);
	ground.model.materials[0].shader = shader;
    ground.model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = logoGround;

	ground.material = LoadMaterialDefault();
	ground.material.shader = shader;
	ground.material.maps[MATERIAL_MAP_DIFFUSE].texture = logo;

	ground.transforms = (Matrix *)RL_CALLOC(X_CELLS*Z_CELLS, sizeof(Matrix));
    int i=0;
	for (int ix = 0, xCoord = BEGIN_CELL_POS; ix < X_CELLS; ix++, xCoord++) {
		for (int iz = 0, zCoord = BEGIN_CELL_POS; iz < Z_CELLS; iz++, zCoord++) {
			ground.transforms[i] = MatrixTranslate(xCoord + 0.5f, 0.0f, zCoord + 0.5f);
			
			ground.cells[ix][iz].isEmpty = true;
			ground.cells[ix][iz].entityId = -1;
			ground.cells[ix][iz].color = getRandomColor();
			i++;
		}
	}	
}

#include "entity.cpp"
EntityPool entityPool;

void getIndexesFromPosition(PositionIndex& pIndex, Vector3 pos) {
	pIndex.x = pos.x - 0.5 - BEGIN_CELL_POS;
	pIndex.z = pos.z - 0.5 - BEGIN_CELL_POS;
}

Vector3 getPositionFromIndexes(PositionIndex& pIndex) {
	return { BEGIN_CELL_POS + pIndex.x + 0.5f, 0.5f, BEGIN_CELL_POS + pIndex.z + 0.5f };
}


//********** Cube
struct Cube {
	Model model;
	PositionIndex pIndex;
	Vector3 position;
	Vector3 nextPosition;
	Vector3 direction;
	Vector3 moveStep;
	
	Vector3 rotationAxis;
	Vector3 rotationOrigin;
	float rotationAngle;
	Matrix transform;
    
	bool isMoving;
	float animationProgress;
	float animationSpeed;

	Color facesColor;
	Color wiresColor;

	Sound rollWav;
	float pitchChange;
	Sound collision;
};
Cube cube;

Vector3 cubeInitPos = {0.5f, 0.51f, 0.5f};

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

		.isMoving = false,
		.animationProgress = 0.0f,
		.animationSpeed = 2.5f,

		.facesColor = WHITE,
		.wiresColor = GREEN,

		.rollWav = LoadSound("assets/sounds/roll.wav"),
		.pitchChange = 1.0f,
		.collision = LoadSound("assets/sounds/collision.wav"),
	};
		
	getIndexesFromPosition(cube.pIndex, cubeInitPos);
	cube.model.materials[0].shader = shader;
    cube.model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = logo;
}

//********** Cube Camera
struct CubeCamera {
	Camera c3d;
	float distance; // distance to target
	Vector3 direction;
	float angleX;
	float angleY;
};
CubeCamera camera;

void initCamera() {
	camera.c3d = {
		.position = (Vector3){10.0f, 3.0f, 0.5f},
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

void updateCamera(float delta) {

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


//********** Keyboard definitions
struct Keyboard {
	int pressedKey;
	double keyPressTime;
	double keyReleaseTime;
	float pressReleaseTime; // Between press and release
	float lastPressedKeyTime;
		
	bool hasQueuedKey;
	int queuedKey;
	bool shiftPressed;

	bool cursorHidden;
	bool drawAxis;
	bool coloredGround;
	bool instancingEnabled;
};
Keyboard kb = {
    .pressedKey = 0,
    .keyPressTime = 0.0f,
    .keyReleaseTime = 0.0f,
    .pressReleaseTime = 0.0f,
	.lastPressedKeyTime = 0.0f,

    .hasQueuedKey = false,
    .queuedKey = 0,
	.shiftPressed = false,
	
    .cursorHidden = false,
	.drawAxis = false,
	.coloredGround = false,
	.instancingEnabled = true,
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

//********** Mouse definitions & functions
struct Mouse {
	Vector2 position;
	Vector2 prevPosition;
	Vector2 deltaPosition;
};
Mouse mouse = {
	.position = {0.0f, 0.0f},
	.prevPosition = {0.0f, 0.0f},
	.deltaPosition = {0.0f, 0.0f},
};

void mouseUpdateCameraAngles() {
	mouse.deltaPosition = { 0.0f, 0.0f };
	mouse.position = GetMousePosition();
            
	if (mouse.prevPosition.x != 0.0f || mouse.prevPosition.y != 0.0f) {
		mouse.deltaPosition.x = mouse.position.x - mouse.prevPosition.x;
		mouse.deltaPosition.y = mouse.position.y - mouse.prevPosition.y;
	}
	mouse.prevPosition = mouse.position;
            
	// Update camera angles based on mouse movement
	camera.angleX -= mouse.deltaPosition.x * 0.003f;
	camera.angleY = camera.angleY + mouse.deltaPosition.y * 0.003f;
	camera.angleY = Clamp(camera.angleY, -1.5*DEG2RAD, PI/2 - 0.3f);

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

Vector3 getMouseXZPosition() {
    Ray mouseRay = GetMouseRay(GetMousePosition(), camera.c3d);

    // Plane is at y = 0
    float planeY = 0.0f;

    // Ray formula: P = origin + t * direction
    float t = (planeY - mouseRay.position.y) / mouseRay.direction.y;

    if (t > 0) { // Ensure ray is pointing downward
        return (Vector3) {
            mouseRay.position.x + t * mouseRay.direction.x,
            planeY,  // Always on XZ plane
            mouseRay.position.z + t * mouseRay.direction.z
        };
    }

    return (Vector3) { 0.0f, 0.0f, 0.0f }; // Default if no intersection
}

void getMouseXZindexes(PositionIndex& pIndex) {
	Vector3 xzPos = getMouseXZPosition();
	getIndexesFromPosition(pIndex, Vector3Add({ 0.5f, 0.0f, 0.5f}, xzPos));
}

void handleMouseButtons() {
	if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
		mouseUpdateCameraAngles();
		mouseUpdateCubeDirection();
			
	} else {
        mouse.prevPosition = (Vector2){ 0.0f, 0.0f };
    }
	
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !kb.cursorHidden) {

		PositionIndex pIndex;
		getMouseXZindexes(pIndex);
		
		if (ground.cells[pIndex.x][pIndex.z].isEmpty) {
			int id = entityPool.addEntity(pIndex);
			ground.cells[pIndex.x][pIndex.z].entityId = id;
			ground.cells[pIndex.x][pIndex.z].isEmpty = false;
			
		} else {
			ground.cells[pIndex.x][pIndex.z].isEmpty = true;
			int id = ground.cells[pIndex.x][pIndex.z].entityId;
			Entity e = { pIndex, id };
			entityPool.removeEntity(e);
			
			// store new id in the position
			ground.cells[pIndex.x][pIndex.z].entityId = e.id;
		}
	}
}

const float MIN_CAMERA_DISTANCE = 5.0f;  // Minimum zoom distance
const float MAX_CAMERA_DISTANCE = 100.0f;  // Maximum zoom distance
const float ZOOM_SPEED = 1.0f;           // Zoom sensitivity
void handleMouseWheel() {
    // Handle mouse wheel for zoom
    float mouseWheel = GetMouseWheelMove();
    if (mouseWheel != 0) {
        camera.distance -= mouseWheel * ZOOM_SPEED;
        camera.distance = fmax(MIN_CAMERA_DISTANCE, fmin(camera.distance, MAX_CAMERA_DISTANCE));
    }
}



// Define axis parameters
const float axisLength = 2.0f;  // Length of each axis
const float coneLength = 0.3f;  // Length of the cone part
const float coneRadius = 0.1f;  // Radius of the cone base
const float lineRadius = 0.02f; // Radius for the axis lines    
void drawAxis() {
    // Draw coordinate axes with cones
    // X axis (red)
    DrawCylinderEx(Vector3Zero(), (Vector3){axisLength, 0, 0}, lineRadius, lineRadius, 8, RED);
    DrawCylinderEx((Vector3){axisLength, 0, 0}, (Vector3){axisLength + coneLength, 0, 0}, coneRadius, 0.0f, 8, RED);

    // Y axis (green)
    DrawCylinderEx(Vector3Zero(), (Vector3){0, axisLength, 0}, lineRadius, lineRadius, 8, GREEN);
    DrawCylinderEx((Vector3){0, axisLength, 0}, (Vector3){0, axisLength + coneLength, 0}, coneRadius, 0.0f, 8, GREEN);

    // Z axis (blue)
    DrawCylinderEx(Vector3Zero(), (Vector3){0, 0, axisLength}, lineRadius, lineRadius, 8, BLUE);
    DrawCylinderEx((Vector3){0, 0, axisLength}, (Vector3){0, 0, axisLength + coneLength}, coneRadius, 0.0f, 8, BLUE);
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

bool hasCollision() {
	int xi = (int)cube.moveStep.x;
	int zi = (int)cube.moveStep.z;
	bool isEmpty =  ground.cells[cube.pIndex.x + xi][cube.pIndex.z + zi].isEmpty;
	if (isEmpty) {
		cube.pIndex.x += xi;
		cube.pIndex.z += zi;
		return false;
	} 
	return true;
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

	if (hasCollision()) {
		PlaySound(cube.collision);
		return;
	}
	
	cube.nextPosition.x = cube.position.x + cube.moveStep.x;
	cube.nextPosition.y = cube.position.y + cube.moveStep.y;
	cube.nextPosition.z = cube.position.z + cube.moveStep.z;
                
	cube.rotationAngle = 0.0f;
	cube.animationProgress = 0.0f;
	cube.isMoving = true;
}

void updateCubeMovement(float delta) {

	cube.animationProgress += delta * cube.animationSpeed;

	// Use smooth easing for animation
	float t = cube.animationProgress;
	float smoothT = t * t * (3.0f - 2.0f * t); // Smoothstep formula
                    
    // Always rotate 90 degrees
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

	if (cube.animationProgress >= 1.0f) {

		cube.pitchChange = KeyDelay::lerpPitch(kb.pressReleaseTime, 0.03f, 0.3f);
		SetSoundPitch(cube.rollWav, cube.pitchChange);
		PlaySound(cube.rollWav);
		                
		cube.position = cube.nextPosition;
		cube.animationProgress = 0.0f;
		cube.rotationAngle = 0.0f;
		cube.isMoving = false;

	}
}

//********** Keyboard management
void handleKeyboard() {
	
	if (IsKeyPressed(KEY_F1)) {
		kb.cursorHidden = !kb.cursorHidden;
		if (kb.cursorHidden)
			HideCursor();
		else 
			ShowCursor();
	}
	if (IsKeyPressed(KEY_F10)) { imgui_demo_window = !imgui_demo_window; }
	if (IsKeyPressed(KEY_F5)) { kb.coloredGround = !kb.coloredGround; }
	if (IsKeyPressed(KEY_F11)) { ToggleFullscreen(); }

	
	kb.shiftPressed = IsKeyDown(kb.pressedKey) && (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT));
	
	int releasedKey =
		IsKeyReleased(KEY_W) ? KEY_W :
		IsKeyReleased(KEY_S) ? KEY_S :
		IsKeyReleased(KEY_A) ? KEY_A :
		IsKeyReleased(KEY_D) ? KEY_D : 0;

	if (kb.pressedKey !=0 && releasedKey == kb.pressedKey) {
		kb.keyReleaseTime = GetTime();
		kb.pressReleaseTime = kb.keyReleaseTime - kb.keyPressTime;
		float t = kb.pressReleaseTime;
		
		if (!kb.shiftPressed)
			cube.animationSpeed = KeyDelay::lerpSpeed(t, 0.01f, 0.5f);
	}
		
	int pressedKey =
		IsKeyPressed(KEY_W) ? KEY_W :
		IsKeyPressed(KEY_S) ? KEY_S :
		IsKeyPressed(KEY_A) ? KEY_A :
		IsKeyPressed(KEY_D) ? KEY_D : 0;
		
	// cube.animationSpeed = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)
	// 	? fastSpeed : normalSpeed;

	if (kb.shiftPressed) {
		if (pressedKey == 0)
			pressedKey = kb.pressedKey;
	} 

	if (pressedKey) {
		
		kb.pressedKey = pressedKey;
		kb.keyPressTime = GetTime();

		float t = kb.keyPressTime - kb.keyReleaseTime;
		// Different than before (pressed after released)                 
		if (!kb.shiftPressed)
			cube.animationSpeed = KeyDelay::lerpSpeed(t, 0.01f, 0.5f);

		if (!cube.isMoving) { // pressed after a stop
			calculateCubeMovement(pressedKey);
		}
		else if (!kb.shiftPressed){
			kb.hasQueuedKey = true;
			kb.queuedKey = pressedKey;
		}
	}
}


//********** Drawing
void drawRollingCube() {

	if (cube.isMoving) {
			
		rlPushMatrix();
		{
			rlMultMatrixf(MatrixToFloat(cube.transform));
			DrawModel(cube.model, cube.position, 1.0f, cube.facesColor);
		}
		rlPopMatrix();

		Vector3 vOffset = Vector3Scale(cube.rotationAxis, 0.2);
		DrawCylinderEx(Vector3Subtract(cube.rotationOrigin, vOffset),
					   Vector3Add(cube.rotationOrigin, vOffset),
					   0.05f, 0.05f, 20, ORANGE);
				
	} else {
		DrawModel(cube.model, cube.position, 1.0f, cube.facesColor);
	}
}

void drawColoredGround(Ground& ground) {
	float fx = BEGIN_CELL_POS;
	for (int x = 0; x < X_CELLS; x++, fx++) {
		float fz = BEGIN_CELL_POS;
		for (int z = 0; z < Z_CELLS; z++, fz++) {

			DrawTriangle3D({fx, 0.0f, fz}, {fx, 0.0f, fz+1}, {fx+1, 0.0f, fz+1}, ground.cells[x][z].color);
			DrawTriangle3D({fx+1, 0.0f, fz+1}, {fx+1, 0.0f, fz}, {fx, 0.0f, fz}, ground.cells[x][z].color);

		}
	}
}


//********** Sound

// Generate sine wave
#define SAMPLE_RATE 44100  // Standard sample rate
#define SAMPLES 5500    // 1 second of audio

Wave wave = {
	.frameCount = SAMPLES,
	.sampleRate = 44100, 
	.sampleSize = 16, // 16-bit samples
	.channels = 1,	  // Mono sound
	.data = 0
};

float frequency[5] = { 220, 246.94, 277.18, 329.63, 369.99 };
Sound waveSound[5] = {};
void initWave() {
	short* data = (short*)MemAlloc(SAMPLES * sizeof(short)); // Allocate buffer
    
	for (int i=0; i<5; i++) {
		for (int s = 0; s < SAMPLES; s++) {
			float t = (float)s / SAMPLE_RATE; // Time
			data[s] = (short)(sinf(2.0f * PI * frequency[i] * t) * 32000); // Sine wave
		}
		wave.data = data;
		waveSound[i] = LoadSoundFromWave(wave);
	}
}


//********** Timers
#include "timer.cpp"
Timer activationLightTimer("3secsTimer");
Timer moveTimer("MoveTimer");
int countTimer = 0;

bool spawnCube = false;
bool spawnDirUp = true;
Light lights[MAX_LIGHTS];
void testLightMovement(float delta) {

	if (activationLightTimer.isEnabled()) {
		activationLightTimer.update(delta);
		if (!activationLightTimer.isDone()) {
			return;
		}
		lights[1].position.x = -5.5f;
		lights[1].enabled = true;
		moveTimer.start(0.5f);
		PlaySound(waveSound[0]);
	}

	moveTimer.update(delta);
	if (!moveTimer.isDone()) {
		return;
	}
	countTimer++;
	if (countTimer == 5) {
		TRACELOGD("testLightMovement: countTimer = 5!");
		lights[1].enabled = false;
		countTimer = 0;
		activationLightTimer.start(3.0f);
		spawnCube = true;
		spawnDirUp = true;
		return;
	}
	PlaySound(waveSound[countTimer]);
	lights[1].position.x += 1.0f;
	moveTimer.start();
}

float EaseInOut(float t) {
    return t * t * (3.0f - 2.0f * t); // Smoothstep function
}

float elapsedTime = 0.0f;
Vector3 spawnPos = { -1.5f, -0.5f, 0.5f };
void updateSpawnedCube(float delta) {
    if (elapsedTime < 1.0f) { //secs
		// float t = elapsedTime / duration; // Normalize time [0,1]
        // cubePosition.y = startY + t * (endY - startY); // Lerp formula
		float t = elapsedTime / 1.0f;
		
		if (spawnDirUp == true)
			spawnPos.y = -1.0f + EaseInOut(t) * (3.0f + 1.0f);
		else 
			spawnPos.y = 3.0f + EaseInOut(t) * (-1.0f - 3.0f);

		elapsedTime += delta;
    } else {
		if (spawnDirUp) {
			spawnDirUp = false;
			elapsedTime = 0.0f;
		} else {
			spawnCube = false;
			elapsedTime = 0.0f;
			spawnDirUp = true;
		}
	}
}

//********** Lights
void createLights() {
	lights[0] = CreateLight(LIGHT_POINT, { -2.5f, 0.5f, -2.5f }, Vector3Zero(), YELLOW, shader);
	lights[1] = CreateLight(LIGHT_POINT, { 0.5f, 0.5f, 0.5f }, { 0.0f, 0.0f, 0.0f }, RED, shader);
	lights[2] = CreateLight(LIGHT_POINT, { -2.5f, 0.5f, 2.5f }, { 0.0f, 0.0f, 0.0f }, BLUE, shader);
	lights[3] = CreateLight(LIGHT_POINT, { 2.5f, 0.5f, -2.5f }, { 0.0f, 0.0f, 0.0f }, GREEN, shader);
	lights[4] = CreateLight(LIGHT_DIRECTIONAL, { -5.0f, 2.0f, 5.0f }, { 0.0f, 0.0f, 0.0f }, 
							{ 139,146,146,255 }, shader);
	lights[5] = CreateLight(LIGHT_DIRECTIONAL, { 5.0f, 2.0f, -3.0f }, { 0.0f, 0.0f, 0.0f }, 
							{ 144, 147, 98, 255 }, shader);
	
	TRACELOGD("sizeOf lights: %d", sizeof(lights)/sizeof(Light));
	for (int i=0; i<MAX_LIGHTS; i++){
		const char* type = lights[i].type == INACTIVE ? "INACTIVE" : lights[i].type == LIGHT_DIRECTIONAL ? "DIRECTIONAL" : "POINT";
		TRACELOGD("lights[%i].type=%s", i, type);
		Vector3 p = lights[i].position;
		const char* position = TextFormat("x: %f, y:%f, z: %f", p.x, p.y, p.z);
		TRACELOGD("lights[%i].position=%s", i, position);
		lights[i].enabled = false;
	}
	lights[4].enabled = true;
	lights[5].enabled = true;
}

void drawLights() {
	// Draw spheres to show where the lights are
	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		if (lights[i].type == INACTIVE) continue;
		if (lights[i].hidden == true) continue;

		if (lights[i].enabled)  {
			DrawSphereEx(lights[i].position, 0.2f, 8, 8, lights[i].color);
		}
		else { 
			DrawSphereWires(lights[i].position, 0.2f, 8, 8, ColorAlpha(lights[i].color, 0.3f));
		}
			
		if (lights[i].type == LIGHT_DIRECTIONAL) {
			DrawLine3D(lights[i].position, lights[i].target, lights[i].color);
			DrawSphereWires(lights[i].target, 0.021f, 4, 4, lights[i].color);
		}
	}
}



void imguiMenus();
void drawText(int margin);
Vector2 fullHD = { 1920, 1080 };
//********** Main
int main()
{
	SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    SetTraceLogLevel(LOG_ALL);
	InitWindow(fullHD.x, fullHD.y, "Cube!");
	SetWindowPosition(25, 50);

	TRACELOGD("*** Started Cube! ***");
	
	SetTargetFPS(60);
	InitAudioDevice();
	initWave();

	if (kb.cursorHidden) {
		HideCursor();
	}

	loadShader();
	loadTextures();
	createLights();
	Skybox skybox;
	loadSkybox(skybox);

	initGround();
	initCube();
	initCamera();

	int instancing = 0;
	int instancingLoc = GetShaderLocation(shader, "instancing");
	SetShaderValue(shader, instancingLoc, &instancing, SHADER_UNIFORM_INT);


	// Entities
	entityPool.init(100);
	
	PositionIndex initialObstacles[5] = { { 53, 53 }, { 48, 48 }, { 53, 48 }, { 50, 51 }, { 52, 49 } };
	for (PositionIndex idx : initialObstacles) {
		int id = entityPool.addEntity(idx);
		ground.cells[idx.x][idx.z].entityId = id;
		ground.cells[idx.x][idx.z].isEmpty = false;
	}
	
	activationLightTimer.start(3.0f);
	
	rlImGuiSetup(true);
	while (!WindowShouldClose()) // Main game loop
	{
		float delta = GetFrameTime();

		handleMouseButtons();
		handleMouseWheel();
		handleKeyboard();

		if (cube.isMoving) {
			updateCubeMovement(delta);

		} 
		else if (kb.hasQueuedKey) {
			calculateCubeMovement(kb.queuedKey);
			updateCubeMovement(delta);
			
			if (!kb.shiftPressed)
				kb.hasQueuedKey = false;
		}

		updateCamera(delta);

		// Update the shader with the camera view vector (points towards { 0.0f, 0.0f, 0.0f })
		float cameraPos[3] = { camera.c3d.position.x, camera.c3d.position.y, camera.c3d.position.z };
		SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);


		SetShaderValue(shader, ambientLoc, &ambient, SHADER_UNIFORM_VEC4);
		
		for (int i = 0; i < MAX_LIGHTS; i++) {
			UpdateLightValues(shader, lights[i]);
		}

		testLightMovement(delta);
		if (spawnCube) {
			updateSpawnedCube(delta);
		}
		
		if (IsFileDropped()) {
			updateSkybox(skybox);
		}
		
		BeginDrawing();
		{
			ClearBackground(BLACK);
			// ClearBackground(RAYWHITE);

			BeginMode3D(camera.c3d);
			{
				int timeLoc = GetShaderLocation(skybox.model.materials[0].shader, "time");
				float elapsedTime = GetTime();
				SetShaderValue(skybox.model.materials[0].shader, timeLoc, &elapsedTime, SHADER_UNIFORM_FLOAT);
				
				rlDisableBackfaceCulling();
				rlDisableDepthMask();
				DrawModel(skybox.model, Vector3Zero(), 1.0f, WHITE);
				rlEnableBackfaceCulling();
				rlEnableDepthMask();
				
				if (kb.drawAxis) {
					drawAxis();
				}
								
				if (kb.coloredGround) {
					drawColoredGround(ground);
				} else {
					// TODO
					BeginShaderMode(shader);
					instancing = 1;
					SetShaderValue(shader, instancingLoc, &instancing, SHADER_UNIFORM_INT);
					DrawMeshInstanced(ground.plane, ground.material, ground.transforms, X_CELLS*Z_CELLS);
					EndShaderMode();
				}
				
				// TODO: since shader is applied to models...
				// I think these 2 BeginShaderMode() above and below are not needed...
				
				BeginShaderMode(shader); // TODO
				instancing = 0;
				SetShaderValue(shader, instancingLoc, &instancing, SHADER_UNIFORM_INT);
				
				// Draw several planes with ground texture to check its appearance
				DrawModel(ground.model, {-2.5f,0.05,-2.5f}, 1.0f, RED);
				DrawModel(ground.model, {-3.5f,0.05,-2.5f}, 1.0f, RED);
				DrawModel(ground.model, {-4.5f,0.05,-2.5f}, 1.0f, RED);
				DrawModel(ground.model, {-4.5f,0.05,-3.5f}, 1.0f, RED);

				drawRollingCube();
				EndShaderMode();

				for (int i=0; i<entityPool.getCount(); i++) {
					PositionIndex idx = entityPool.getPositionIndex(i);
					Vector3 v = getPositionFromIndexes(idx);
					DrawModel(cube.model, v, 1.0f, RED);
				}

				if (spawnCube) {
					DrawModel(cube.model, spawnPos, 1.0f, MAGENTA);
				}
				
				drawLights();

			}
			EndMode3D();

			int tp = 10; // topMargin
			DrawRectangle(8, 8, 200, 44, RAYWHITE);
			DrawFPS(12, tp);
			DrawText("F1 - Toggle Menus", 12, tp + 20, 20, BLACK);
			// DrawText("F5 - Toggle ground colors", 12, tp + 60, 20, BLACK);
			if (!kb.cursorHidden) {
				imguiMenus();
				drawText(tp+120);	  
			}
		}
		EndDrawing();
	}
	rlImGuiShutdown();

	TRACELOGD("Ending program!");
	
	UnloadShader(shader);
	UnloadShader(skybox.model.materials[0].shader);
	UnloadTexture(skybox.model.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture);
	UnloadModel(skybox.model);
	
	UnloadSound(cube.rollWav);
	CloseAudioDevice();
	CloseWindow();
    return 0;
}



//********** ImGui & DrawText stuff
ImVec4 RaylibColorToImVec4(Color col) {
    return ImVec4(
        col.r / 255.0f, // Red (0 to 1)
        col.g / 255.0f, // Green (0 to 1)
        col.b / 255.0f, // Blue (0 to 1)
        col.a / 255.0f  // Alpha (0 to 1)
		);
}

// Convert ImGui color format back to Raylib Color (0 to 255)
Color ImVec4ToRaylibColor(ImVec4 col) {
    return Color{
        (unsigned char)(col.x * 255), // Red (0 to 255)
        (unsigned char)(col.y * 255), // Green (0 to 255)
        (unsigned char)(col.z * 255), // Blue (0 to 255)
        (unsigned char)(col.w * 255)  // Alpha (0 to 255)
    };
}
void imguiMenus() {

	rlImGuiBegin();
	ImGui::Begin("Keyboard & Mouse");

	ImGui::SeparatorText("Keyboard");
	ImGui::Checkbox("Colored Ground plane (F6)", &kb.coloredGround);
	bool isShiftDown = (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT));
	ImGui::Checkbox("IsShiftDown", &isShiftDown);
	// ImGui::Text("pressedKey: ");  ImGui::SameLine(180); 
	// ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%i", pressedKey);
	ImGui::Text("kb.pressedKey: ");  ImGui::SameLine(180); 
	ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%i", kb.pressedKey);
	ImGui::Checkbox("hasQueuedKey", &kb.hasQueuedKey);
	ImGui::Spacing();
	ImGui::Text("Press/Release time:"); ImGui::SameLine(180);
	ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%.3f", kb.pressReleaseTime);
	ImGui::Text("Cube animation speed:"); ImGui::SameLine(180);
	ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%.3f", cube.animationSpeed); ImGui::SameLine(280);
	ImGui::Text("Pitch change:"); ImGui::SameLine(380);
	ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%.3f", cube.pitchChange);
	ImGui::Spacing();
	
	ImGui::SeparatorText("Mouse");
	
	PositionIndex pIndex;
	Vector3 xzPos = getMouseXZPosition();
	ImGui::Text("xz position: (%0.2f, %0.2f)", xzPos.x, xzPos.z);
	getIndexesFromPosition(pIndex, Vector3Add({ 0.5f, 0.0f, 0.5f}, xzPos));
	ImGui::Text("xz Index: (%i, %i)", pIndex.x, pIndex.z);

	ImGui::End();

	ImGui::Begin("Cube & Camera");
	ImGui::SeparatorText("Cube");
	ImGui::Checkbox("isMoving", &cube.isMoving);
	ImGui::Text("pIndex (ix, iz):"); ImGui::SameLine(140);
	ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "(%i, %i)", cube.pIndex.x, cube.pIndex.z);
	ImGui::DragFloat3("position", (float *)&cube.position, 1.0f, -1000.0f, 1000.0f);
	ImGui::DragFloat3("next position", (float *)&cube.nextPosition, 1.0f, -1000.0f, 1000.0f);
	ImGui::DragFloat3("direction", (float *)&cube.direction, 1.0f, -1000.0f, 1000.0f);
	ImGui::DragFloat3("rotationAxis", (float *)&cube.rotationAxis, 1.0f, -1000.0f, 1000.0f);
	ImGui::DragFloat("rotationAngle", (float *)&cube.rotationAngle, 1.0f, 200.0f, 2000.0f);
	ImVec4 cubeFacesColor = RaylibColorToImVec4(cube.facesColor);
	if (ImGui::ColorEdit4("faces color", &cubeFacesColor.x)) {
		cube.facesColor = ImVec4ToRaylibColor(cubeFacesColor);
	}
	ImVec4 cubeWiresColor = RaylibColorToImVec4(cube.wiresColor);
	if (ImGui::ColorEdit4("wires color", &cubeWiresColor.x)) {
		cube.wiresColor = ImVec4ToRaylibColor(cubeWiresColor);
	}
	ImGui::Spacing();

	ImGui::SeparatorText("Camera");
	ImGui::Checkbox("drawAxis", &kb.drawAxis);	
	ImGui::DragFloat3("position ", (float *)&camera.c3d.position, 1.0f, -1000.0f, 1000.0f);
	ImGui::DragFloat3("target ", (float *)&camera.c3d.target, 1.0f, -1000.0f, 1000.0f);
	float camAngleX = camera.angleX * RAD2DEG;
	float camAngleY = camera.angleY * RAD2DEG;
	ImGui::DragFloat("angle_x", (float*)&camAngleX, 1.0f, 200.0f, 2000.0f);
	ImGui::DragFloat("angle_y", (float*)&camAngleY, 1.0f, 200.0f, 2000.0f);
	ImGui::Spacing();
	ImGui::End();

	ImGui::Begin("Lighting");
	ImGui::SeparatorText("Lights");
	ImGui::ColorEdit4("ambient", &ambient.x);
	ImGui::Text("lightsCount:"); ImGui::SameLine(180);
	ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%i", lightsCount);
	ImVec4 lightColor[6];
	for (int i=0; i<=5; i++) {
		const char* type = (lights[i].type == LIGHT_POINT ? "Point" : "Directional");
		const char* lightName = TextFormat("Light %i - %s", i, type);
		if (ImGui::CollapsingHeader( lightName)) {
			const char* posStr = TextFormat("position##%i", i);
			const char* colorStr = TextFormat("color##%i", i);
			const char* enableStr = TextFormat("enable##%i", i);
			const char* hiddenStr = TextFormat("hidden##%i", i);
			ImGui::Checkbox(enableStr, &lights[i].enabled);ImGui::SameLine(140);
			ImGui::Checkbox(hiddenStr, &lights[i].hidden);
			ImGui::DragFloat3(posStr, (float *)&lights[i].position, 0.5f, -20.0f, 20.0f);
			lightColor[i] = RaylibColorToImVec4(lights[i].color);
			if (ImGui::ColorEdit4(colorStr, &lightColor[i].x)) {
				lights[i].color = ImVec4ToRaylibColor(lightColor[i]);
			}
			if (lights[i].type == LIGHT_DIRECTIONAL) {
				const char* targetStr = TextFormat("target##%i", i);
				ImGui::DragFloat3(targetStr, (float *)&lights[i].target, 0.5f, -100.0f, 100.0f);
			}
			ImGui::Spacing();
		}
	}

// ImGui::Checkbox("isEmpty light", &camera.freeLight);        
// ImGui::DragFloat3("position  ", (float *)&camera.light.position, 0.2f, -1000.0f, 1000.0f);
// ImGui::DragFloat3("target  ", (float *)&camera.light.target, 0.2f, -1000.0f, 1000.0f);
// ImVec4 lightColor = RaylibColorToImVec4(camera.light.color);
// if (ImGui::ColorEdit4("color", &lightColor.x)) {
// 	camera.light.color = ImVec4ToRaylibColor(lightColor);
// }

	ImGui::End();

	if (imgui_demo_window) {
		ImGui::ShowDemoWindow(&imgui_demo_window);
	}
	rlImGuiEnd();
}

void drawText(int margin) {
// 	DrawText(TextFormat("cube.position: {%.2f, %.2f, %.2f}",
// 						cube.position.x, cube.position.y, cube.position.z),
// 			 10, margin, 20, BLACK);
}
