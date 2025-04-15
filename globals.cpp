#ifndef GLOBALS_CPP
#define GLOBALS_CPP

#include "raylib.h"

#include "entity.h"
EntityPool entityPool;

// ****** Shaders and Textures
Shader shader;
Vector4 ambient = { 0.1f, 0.1f, 0.1f, 1.0f };
int ambientLoc;

Texture logo;

#include "lights.h"
Light lights[MAX_LIGHTS];
int lightsCount = 0;    // Current amount of created lights

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

float delta = 0.0f;


// ****** Global functions

void loadShader() {
	shader = LoadShader(TextFormat("shaders/lighting_with_instancing.vs", GLSL_VERSION),
						TextFormat("shaders/lighting_with_instancing.fs", GLSL_VERSION));
	shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");

	ambientLoc = GetShaderLocation(shader, "ambient");
	SetShaderValue(shader, ambientLoc, &ambient, SHADER_UNIFORM_VEC4);
}

void loadLogo() {
	logo = LoadTexture("assets/logo.png");
	
}

void getIndexesFromPosition(PositionIndex& pIndex, Vector3 pos) {
	pIndex.x = pos.x - 0.5 - BEGIN_CELL_POS;
	pIndex.z = pos.z - 0.5 - BEGIN_CELL_POS;
}

Vector3 getPositionFromIndexes(PositionIndex& pIndex) {
	return { BEGIN_CELL_POS + pIndex.x + 0.5f, 0.5f, BEGIN_CELL_POS + pIndex.z + 0.5f };
}

bool isValidPositionIndex(PositionIndex& pIndex) {
	
	return !(pIndex.x < 0 || pIndex.x > X_CELLS-1 || pIndex.z < 0 || pIndex.z > Z_CELLS - 1);
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


// Send light properties to shader
// NOTE: Light shader locations should be available 
void UpdateLightValues(Shader shader, Light light)
{
    // Send to shader light enabled state and type
    SetShaderValue(shader, light.enabledLoc, &light.enabled, SHADER_UNIFORM_INT);
    SetShaderValue(shader, light.typeLoc, &light.type, SHADER_UNIFORM_INT);

    // Send to shader light position values
    float position[3] = { light.position.x, light.position.y, light.position.z };
    SetShaderValue(shader, light.positionLoc, position, SHADER_UNIFORM_VEC3);

    // Send to shader light target position values
    float target[3] = { light.target.x, light.target.y, light.target.z };
    SetShaderValue(shader, light.targetLoc, target, SHADER_UNIFORM_VEC3);

    // Send to shader light color values
    float color[4] = { (float)light.color.r/(float)255, (float)light.color.g/(float)255, 
		(float)light.color.b/(float)255, (float)light.color.a/(float)255 };
    SetShaderValue(shader, light.colorLoc, color, SHADER_UNIFORM_VEC4);
}


// Create a light and get shader locations
Light CreateLight(int type, Vector3 position, Vector3 target, Color color, Shader shader)
{
    Light light = { 0 };

    if (lightsCount < MAX_LIGHTS)
    {
        light.enabled = true;
        light.type = type;
        light.position = position;
        light.target = target;
        light.color = color;
		light.hidden = true;

        // NOTE: Lighting shader naming must be the provided ones
        light.enabledLoc = GetShaderLocation(shader, TextFormat("lights[%i].enabled", lightsCount));
        light.typeLoc = GetShaderLocation(shader, TextFormat("lights[%i].type", lightsCount));
        light.positionLoc = GetShaderLocation(shader, TextFormat("lights[%i].position", lightsCount));
        light.targetLoc = GetShaderLocation(shader, TextFormat("lights[%i].target", lightsCount));
        light.colorLoc = GetShaderLocation(shader, TextFormat("lights[%i].color", lightsCount));

        UpdateLightValues(shader, light);
        
        lightsCount++;
		TRACELOGD("Light created: number=%d | enabled=%s", lightsCount, light.enabled ? "true" : "false");
    } else {
		TRACELOGD("MAX_LIGHTS exceeded!");
	}

	return light;
}

#endif
