#ifndef GLOBALS_CPP
#define GLOBALS_CPP

#include "raylib.h"

// Set to GetFrameTime() in game loop
float delta = 0.0f;

// ****** Entities
#include "entity.h"
EntityPool entityPool;

// ****** Shaders and Textures
#include "shader_lights.cpp"
ShaderLightsData sld;
Vector4 ambient = { 0.1f, 0.1f, 0.1f, 1.0f };
int ambientLoc;

// ****** Ground 
#include "ground.h"
Ground ground;


// ****** Input
#include "input.h"
Keyboard kb = {
    .pressedKey = 0,
    .keyPressTime = 0.0f,
    .keyReleaseTime = 0.0f,
    .pressReleaseTime = 0.0f,
	.lastPressedKeyTime = 0.0f,

    .hasQueuedKey = false,
    .queuedKey = 0,
	.shiftPressed = false,
	.shiftTimer = 0.0f,
	
	.instancingEnabled = true,
};

Mouse mouse = {
	.position = {0.0f, 0.0f},
	.prevPosition = {0.0f, 0.0f},
	.deltaPosition = {0.0f, 0.0f},
	
	.cursorHidden = false,
	
	.zoomEnabled = true,
	.zoomSpeed = 1.0f,
	.minZoomDistance = 5.0f,
	.maxZoomDistance = 100.0f,
};

// ****** Options
struct ImguiOptions {
	bool drawAxis;
	bool coloredGround;
	bool inputWindow;
	bool entitiesWindow;
	int entityType;
	
	bool cubeWindow;
	bool lightsWindow;
	bool demoWindow;
	bool editEnabled;
	bool soundEnabled;
};
ImguiOptions ops = {
	.drawAxis = false,
	.coloredGround = false,
	
	.inputWindow = false,
	.entitiesWindow = true,
	.entityType = OBSTACLE,
	.cubeWindow = true,
	.lightsWindow = false,
	.demoWindow = false,
	
	.editEnabled = false,
	.soundEnabled = false,
};


// ****** Global functions
void getIndexesFromPosition(PositionIndex& pIndex, Vector3 pos) {
	pIndex.x = pos.x - 0.5;
	pIndex.z = pos.z - 0.5;
}

Vector3 getPositionFromIndexes(PositionIndex& pIndex) {
	return { pIndex.x + 0.5f, 0.5f, pIndex.z + 0.5f };
}

bool isValidPositionIndex(PositionIndex& pIndex) {
	
	return !(pIndex.x < 0 || pIndex.x > Ground::X_CELLS-1 || pIndex.z < 0 || pIndex.z > Ground::Z_CELLS - 1);
}	

Vector3 getMouseXZPosition(Camera& camera) {
    Ray mouseRay = GetMouseRay(GetMousePosition(), camera);

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

	// return a negative Y, meaning no intersection
    return (Vector3) { 0.0f, -1.0f, 0.0f };
}

void getMouseXZindexes(Camera& camera, PositionIndex& pIndex) {
	Vector3 xzPos = getMouseXZPosition(camera);
// LOGD("xzPos: (%.2f, %.2f, %.2f)", xzPos.x, xzPos.y, xzPos.z);
	if (xzPos.y == -1.0f) {
		pIndex = { -1, -1 }; // Invalid indexes
		return;
	}
	getIndexesFromPosition(pIndex, Vector3Add({ 0.5f, 0.0f, 0.5f}, xzPos));
}

#endif
