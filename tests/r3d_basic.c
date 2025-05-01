#include "raylib.h"

#include <r3d.h>
#include <raylib.h>
#include <raymath.h>

#include <stddef.h>

#define RESOURCES_PATH "./resources/"

static Model		plane = { 0 };
static Model		sphere = { 0 };
static Model		cube = { 0 };
static Camera3D		camera = { 0 };

R3D_Light spotLight;
static Vector3 spotLightPos = { 0, 7.5, 5 };
static Vector3 spotLightTarget = { 0, 0, 0 };
bool spotLightEnabled = true;
Color spotLightColor = BLUE;

R3D_Light light;
static Vector3 lightPos = { 0, 2.5, -5 };
static Vector3 lightTarget = { 0, 0, 0 };
bool lightEnabled = true;
Color lightColor = RED;

float angle = 0.0f;
float radius = 5.0f;
float yPos = 5.0f;

const char* Init(void)
{
    R3D_Init(GetScreenWidth(), GetScreenHeight(), 0);
    SetTargetFPS(60);

    plane = LoadModelFromMesh(GenMeshPlane(1000, 1000, 1, 1));
    plane.materials[0].maps[MATERIAL_MAP_OCCLUSION].value = 1;
    plane.materials[0].maps[MATERIAL_MAP_ROUGHNESS].value = 1;
    plane.materials[0].maps[MATERIAL_MAP_METALNESS].value = 0;

    sphere = LoadModelFromMesh(GenMeshSphere(0.5f, 64, 64));
    sphere.materials[0].maps[MATERIAL_MAP_OCCLUSION].value = 1;
    sphere.materials[0].maps[MATERIAL_MAP_ROUGHNESS].value = 0.25;
    sphere.materials[0].maps[MATERIAL_MAP_METALNESS].value = 0.75;

	cube = LoadModelFromMesh(GenMeshCube(1.0, 1.0, 1.0));
	cube.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = LoadTexture("./assets/cube_color.jpg");
	cube.materials[0].maps[MATERIAL_MAP_ALBEDO].value = 1.0;
	cube.materials[0].maps[MATERIAL_MAP_ROUGHNESS].texture = LoadTexture("./assets/cube_roughness.jpg");
	cube.materials[0].maps[MATERIAL_MAP_ROUGHNESS].value = 1.0;
	cube.materials[0].maps[MATERIAL_MAP_OCCLUSION].texture = LoadTexture("./assets/cube_ao.jpg");
	cube.materials[0].maps[MATERIAL_MAP_OCCLUSION].value = 1.0;
	cube.materials[0].maps[MATERIAL_MAP_NORMAL].texture = LoadTexture("./assets/cube_normal.png");
	cube.materials[0].maps[MATERIAL_MAP_NORMAL].value = 1.0;
	SetTextureFilter(cube.materials[0].maps[MATERIAL_MAP_ALBEDO].texture, TEXTURE_FILTER_BILINEAR);
	SetTextureFilter(cube.materials[0].maps[MATERIAL_MAP_ROUGHNESS].texture, TEXTURE_FILTER_BILINEAR);
	SetTextureFilter(cube.materials[0].maps[MATERIAL_MAP_OCCLUSION].texture, TEXTURE_FILTER_BILINEAR);
	SetTextureFilter(cube.materials[0].maps[MATERIAL_MAP_NORMAL].texture, TEXTURE_FILTER_BILINEAR);
	
	
	camera = (Camera3D) {
		.position = (Vector3) { 0, 5, 5 },
		.target = (Vector3) { 0, 0, 0 },
		.up = (Vector3) { 0, 1, 0 },
		.fovy = 60,
	};

	spotLight = R3D_CreateLight(R3D_LIGHT_SPOT);
	{
		R3D_SetLightPosition(spotLight, spotLightPos );
		R3D_SetLightTarget(spotLight, spotLightTarget);
		R3D_SetLightColor(spotLight, spotLightColor);
		R3D_SetLightActive(spotLight, true);
		R3D_EnableShadow(spotLight, 4096);
	}

	light = R3D_CreateLight(R3D_LIGHT_DIR);
	{
		R3D_SetLightDirection(light, (Vector3) { 0, -1, -1 });
		R3D_SetShadowUpdateMode(light, R3D_SHADOW_UPDATE_MANUAL);
		R3D_SetLightColor(light, lightColor);
		R3D_SetShadowBias(light, 0.005f);
		R3D_EnableShadow(light, 4096);

		R3D_SetLightActive(light, true);
	}
	
	return "[r3d] - basic example";
}

void Update(float delta)
{
	if (IsKeyPressed(KEY_F1)) {
		lightEnabled = !lightEnabled;
		R3D_SetLightActive(light, lightEnabled);
	}
	if (IsKeyPressed(KEY_F2)) {
		spotLightEnabled = !spotLightEnabled;
		R3D_SetLightActive(spotLight, spotLightEnabled);
	}
	
	angle += 0.01f; // Control speed here
	if (angle > 2*PI) angle -= 2*PI;

	// Compute position on circle
	Vector3 pos = {
		cosf(angle) * radius,
		yPos,
		sinf(angle) * radius
	};
	spotLightPos = pos;
	R3D_SetLightPosition(spotLight, spotLightPos );
	
	UpdateCamera(&camera, CAMERA_FREE);
}

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

void drawLights() {
	DrawSphereWires(spotLightPos, 0.5f, 10, 10, spotLightColor);
	DrawSphereWires(spotLightTarget, 0.2f, 5, 5, spotLightColor);
	DrawLine3D(spotLightPos, spotLightTarget, spotLightColor);
	
	DrawSphereWires(lightPos, 0.5f, 10, 10, lightColor);
	DrawSphereWires(lightTarget, 0.2f, 5, 5, lightColor);
	DrawLine3D(lightPos, spotLightTarget, lightColor);
}

void Draw(void)
{
    R3D_Begin(camera);
	R3D_DrawModel(plane, (Vector3) { 0, 0.0f, 0 }, 1.0f);
	R3D_DrawModel(sphere, (Vector3) { 0, 0.5, 0 }, 1.0f);
	R3D_DrawModel(cube, (Vector3) { 2, 0.5, 2 }, 1.0f);
	R3D_DrawModel(cube, (Vector3) { -2, 0.5, -2 }, 0.5f);
	R3D_End();
	
	BeginMode3D(camera);
	drawAxis();
	drawLights();
	EndMode3D();
	
}

void Close(void)
{
    UnloadModel(plane);
    UnloadModel(sphere);
    R3D_Close();
}


int main(void)
{
	InitWindow(1920, 1080, "");

	const char* title = Init();
	SetWindowTitle(title);

	while (!WindowShouldClose()) {
		Update(GetFrameTime());
		BeginDrawing();
		Draw();
		EndDrawing();
	}

	Close();
	CloseWindow();

	return 0;
}

