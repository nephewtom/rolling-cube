#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include <cstdlib>
#include <stdlib.h>  // for rand()
#include <time.h>    // for time()
#include <stdio.h>  // For FILE, fread, etc.
#include <stdlib.h> // For malloc

#include "imgui.h"
#include "rlImGui.h"

#define GLSL_VERSION            330

#define SHADOWMAP_RESOLUTION 1024
#define SET_FLOAT4(arr, a, b, c, d)										\
    do { arr[0] = a; arr[1] = b; arr[2] = c; arr[3] = d; } while (0)

RenderTexture2D LoadShadowmapRenderTexture(int width, int height);
void UnloadShadowmapRenderTexture(RenderTexture2D target);
void DrawScene(Model cube, Model robot);
void Draw3DSpaceGrid(Vector3 origin, int gridSize, float spacing);
void drawAxis();
void drawLight();
void drawLaser(float& time);

void loadMusic();
void loadShadowShader();
void loadLaserShader();
void handleKeys(float delta);
void handleLightKeys(float delta);
void handleMouse(float delta);
void imguiMenus();

Vector3 getDirection(Vector3 pos, Vector3 target) {
	Vector3 diff = { pos.x - target.x, pos.y - target.y, pos.z - target.z };
	return Vector3Normalize(diff);
}


struct Global {
	Music music;
	float duration;
	
	Shader shadowShader;
	Vector3 lightDir;
	int lightDirLoc;
	Vector4 lightColor;
	int lightColorLoc;
	float ambient[4];
	int ambientLoc;
	float factor = -15.0f;
	
	
	int shadowMapLoc;
	int lightVPLoc;
	
	Shader laserShader;
    int timeLoc;
	
	bool freeCamera = false;
	Camera camera;
	Camera lightCam;
	float cameraAngleX;
	float cameraAngleY;
	float cameraDistance;
	float animationSpeed = 2.5f;
	
	bool showImgui;
};	
Global g;

int main(void)
{
	srand((unsigned int)time(NULL));  // Seed the random number generator

	const int screenWidth = 1920;
	const int screenHeight = 1080;

	SetConfigFlags(FLAG_MSAA_4X_HINT);
	// Shadows are a HUGE topic, and this example shows an extremely simple implementation of
	// the shadowmapping algorithm, which is the industry standard for shadows.
	// This algorithm can be extended in a ridiculous number of ways to improve realism and also adapt it 
	// for different scenes. This is pretty much the simplest possible implementation.
	InitWindow(screenWidth, screenHeight, "raylib [shaders] example - shadowmap");
	g.showImgui = true;
	
	InitAudioDevice();
	loadMusic();
	PlayMusicStream(g.music);
	g.duration = GetMusicTimeLength(g.music);
	SetMousePosition(screenWidth/2, screenHeight/2);
	
	g.camera = {
		.position = (Vector3){ 10.0f, 10.0f, 10.0f },
		.target = Vector3Zero(),
		.up = (Vector3){ 0.0f, 1.0f, 0.0f },
		.fovy = 60.0f,
		.projection = CAMERA_PERSPECTIVE,
	};

	loadShadowShader();
	Model cube = LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));
	cube.materials[0].shader = g.shadowShader;
	
	Model robot = LoadModel("assets/robot.glb");
	for (int i = 0; i < robot.materialCount; i++) {
		robot.materials[i].shader = g.shadowShader;
	}
	
	int animCount = 0;
	ModelAnimation* robotAnimations = LoadModelAnimations("assets/robot.glb", &animCount);

	RenderTexture2D shadowMap = LoadShadowmapRenderTexture(SHADOWMAP_RESOLUTION, SHADOWMAP_RESOLUTION);
	
	// For the shadowmapping algorithm, we will be rendering everything from the light's point of view
	g.lightCam = {
		.position = Vector3Scale(g.lightDir, g.factor),
		.target = Vector3Zero(),
		.up = (Vector3){ 0.0f, 1.0f, 0.0f },
		.fovy = 20.0f,
		.projection = CAMERA_ORTHOGRAPHIC,
	};


	loadLaserShader();
	
	SetTargetFPS(60);
	int fc = 0;
	rlImGuiSetup(true);
	
	while (!WindowShouldClose()) {

		float time = GetTime();

		float dt = GetFrameTime();
		handleKeys(dt);
		handleMouse(dt);

		UpdateMusicStream(g.music);

		Vector3 cameraPos = g.camera.position;
		SetShaderValue(g.shadowShader, g.shadowShader.locs[SHADER_LOC_VECTOR_VIEW],
					   &cameraPos, SHADER_UNIFORM_VEC3);		
		fc++;
		fc %= (robotAnimations[0].frameCount);
		UpdateModelAnimation(robot, robotAnimations[0], fc);
		
		// g.lightDir = getDirection(g.lightCam.position, g.lightCam.target);


		g.lightDir = Vector3Normalize(g.lightDir);
		g.lightCam.position = Vector3Scale(g.lightDir, g.factor);
		SetShaderValue(g.shadowShader, g.lightDirLoc, &g.lightDir, SHADER_UNIFORM_VEC3);

		BeginDrawing();

		// First, render all objects into the shadowmap
		// The idea is, we record all the objects' depths
		// (as rendered from the light source's point of view) in a buffer
		// Anything that is "visible" to the light is in light, anything that isn't is in shadow
		// We can later use the depth buffer when rendering everything from the player's point of view
		// to determine whether a given point is "visible" to the light

		// Record the light matrices for future use!
		Matrix lightView;
		Matrix lightProj;
		BeginTextureMode(shadowMap);
		{
			ClearBackground(WHITE);
			BeginMode3D(g.lightCam);
			{
				lightView = rlGetMatrixModelview();
				lightProj = rlGetMatrixProjection();
				DrawScene(cube, robot);
			}
			EndMode3D();
		}
		EndTextureMode();
		Matrix lightViewProj = MatrixMultiply(lightView, lightProj);

		ClearBackground(RAYWHITE);

		SetShaderValueMatrix(g.shadowShader, g.lightVPLoc, lightViewProj);

		rlEnableShader(g.shadowShader.id);
		int slot = 10; // Can be anything 0 to 15, but 0 will probably be taken up
		rlActiveTextureSlot(10);
		rlEnableTexture(shadowMap.depth.id);
		rlSetUniform(g.shadowMapLoc, &slot, SHADER_UNIFORM_INT, 1);

		BeginMode3D(g.camera);
		{
			// DrawGrid(100, 1.0f); // Draw 100x100 grid with 1.0 spacing
			Draw3DSpaceGrid({-20.0f, 0.0f, -10.0f},10, 1.0f);  // 10x10x10 grid, 1 unit spacing
			drawAxis();
			
			// Optional: draw XYZ axes
			DrawLine3D((Vector3){-100, 0, 0}, (Vector3){100, 0, 0}, RED);
			DrawLine3D((Vector3){0, -100, 0}, (Vector3){0, 100, 0}, GREEN);
			DrawLine3D((Vector3){0, 0, -100}, (Vector3){0, 0, 100}, BLUE);
			// Draw the same exact things as we drew in the shadowmap!
			DrawScene(cube, robot);
			
			drawLight();
			
			drawLaser(time);
		}
		EndMode3D();

		DrawFPS(10, 10);
		DrawText("Shadows in raylib using the shadowmapping algorithm!", 10, 30, 20, GRAY);
		DrawText("Use the arrow keys to rotate the light!", 10, 50, 20, RED);
        
		float current = GetMusicTimePlayed(g.music);
		float total = GetMusicTimeLength(g.music);

		DrawText(TextFormat("Playing: %.1f / %.1f sec", current, total), 10, 70, 20, BLUE);

		DrawText("F1 Toggle Imgui menus", 10, 90, 20, GREEN);
		DrawText("F5 Toggle free camera", 10, 110, 20, GREEN);
		
		if (g.showImgui) {
			imguiMenus();
		}
		EndDrawing();

		if (IsKeyPressed(KEY_F))
		{
			TakeScreenshot("shaders_shadowmap.png");
		}
	}
	rlImGuiShutdown();

	UnloadShader(g.shadowShader);
	UnloadModel(cube);
	UnloadModel(robot);
	UnloadModelAnimations(robotAnimations, animCount);
	UnloadShadowmapRenderTexture(shadowMap);

	CloseWindow();        // Close window and OpenGL context

	return 0;
}

RenderTexture2D LoadShadowmapRenderTexture(int width, int height)
{
    RenderTexture2D target = { 0 };

    target.id = rlLoadFramebuffer(); // Load an empty framebuffer
    target.texture.width = width;
    target.texture.height = height;

    if (target.id > 0)
    {
        rlEnableFramebuffer(target.id);

        // Create depth texture
        // We don't need a color texture for the shadowmap
        target.depth.id = rlLoadTextureDepth(width, height, false);
        target.depth.width = width;
        target.depth.height = height;
        target.depth.format = 19;       //DEPTH_COMPONENT_24BIT?
        target.depth.mipmaps = 1;

        // Attach depth texture to FBO
        rlFramebufferAttach(target.id, target.depth.id, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_TEXTURE2D, 0);

        // Check if fbo is complete with attachments (valid)
        if (rlFramebufferComplete(target.id)) TRACELOG(LOG_INFO, "FBO: [ID %i] Framebuffer object created successfully", target.id);

        rlDisableFramebuffer();
    }
    else TRACELOG(LOG_WARNING, "FBO: Framebuffer object can not be created");

    return target;
}

// Unload shadowmap render texture from GPU memory (VRAM)
void UnloadShadowmapRenderTexture(RenderTexture2D target)
{
    if (target.id > 0)
    {
        // NOTE: Depth texture/renderbuffer is automatically
        // queried and deleted before deleting framebuffer
        rlUnloadFramebuffer(target.id);
    }
}

void DrawScene(Model cube, Model robot)
{
    // DrawModelEx(cube, Vector3Zero(), { 0.0f, 1.0f, 0.0f }, 0.0f, { 100.0f, 1.0f, 100.0f }, BLUE);
	// DrawModelEx(cube, { 1.5f, 1.0f, -1.5f }, { 0.0f, 1.0f, 0.0f }, 0.0f, Vector3One(), WHITE);
    DrawModelEx(robot, { 0.0f, 0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }, 0.0f,  { 1.0f, 1.0f, 1.0f }, RED);
}

void Draw3DSpaceGrid(Vector3 origin, int gridSize, float spacing)
{
    float totalSize = gridSize * spacing;

    for (int i = 0; i <= gridSize; i++)
    {
        float offset = i * spacing;

        // Lines parallel to X-axis
        for (int j = 0; j <= gridSize; j++)
        {
            float offsetY = j * spacing;
            DrawLine3D(
                (Vector3){ origin.x, origin.y + offsetY, origin.z + offset },
                (Vector3){ origin.x + totalSize, origin.y + offsetY, origin.z + offset },
                DARKGRAY
				);
        }

        // Lines parallel to Y-axis
        for (int j = 0; j <= gridSize; j++)
        {
            float offsetZ = j * spacing;
            DrawLine3D(
                (Vector3){ origin.x + offset, origin.y, origin.z + offsetZ },
                (Vector3){ origin.x + offset, origin.y + totalSize, origin.z + offsetZ },
                DARKGRAY
				);
        }

        // Lines parallel to Z-axis
        for (int j = 0; j <= gridSize; j++)
        {
            float offsetX = j * spacing;
            DrawLine3D(
                (Vector3){ origin.x + offsetX, origin.y + offset, origin.z },
                (Vector3){ origin.x + offsetX, origin.y + offset, origin.z + totalSize },
                DARKGRAY
				);
        }
    }
}

// Define axis parameters
const float axisLength = 10.0f;  // Length of each axis
const float coneLength = 0.3f;  // Length of the cone part
const float coneRadius = 0.1f;  // Radius of the cone base
const float lineRadius = 0.02f; // Radius for the axis lines 
void drawAxis() {
    // Draw coordinate axes with cones
    // X axis (red)
	Vector3 offset = {0.0f, 0.5f, 0.0f};
	DrawCylinderEx(offset, (Vector3){axisLength, 0.5f, 0}, lineRadius, lineRadius, 8, RED);
    DrawCylinderEx({axisLength, 0.5f, 0}, (Vector3){axisLength + coneLength, 0.5f, 0}, coneRadius, 0.0f, 8, RED);

	// Y axis (green)
    DrawCylinderEx(offset, (Vector3){0, axisLength+0.5f, 0}, lineRadius, lineRadius, 8, GREEN);
	DrawCylinderEx({0, axisLength, 0}, (Vector3){0, axisLength + coneLength + 0.5f, 0}, coneRadius, 0.0f, 8, GREEN);

	// Z axis (blue)
    DrawCylinderEx(offset, (Vector3){0, 0.5f, axisLength}, lineRadius, lineRadius, 8, BLUE);
    DrawCylinderEx((Vector3){0, 0.5f, axisLength}, (Vector3){0, 0.5f, axisLength + coneLength}, coneRadius, 0.0f, 8, BLUE);
}


void drawLight() {
	DrawSphereWires(g.lightCam.position, 0.5f, 20, 20, YELLOW);
	Vector3 lTarget = Vector3Add(g.lightCam.target, {0.0f, 0.5f, 0.0f});
	DrawSphereWires(lTarget, 0.25f, 10, 10, YELLOW);
	DrawLine3D(g.lightCam.position, lTarget, RED);
}

void drawLaser(float& time) {
	BeginShaderMode(g.laserShader);
	SetShaderValue(g.laserShader, g.timeLoc, &time, SHADER_UNIFORM_FLOAT);

	// Simulate a "laser line" with two triangles (quad)
	Vector2 p1 = {200, 300};
	Vector2 p2 = {600, 300};
	float thickness = 10.0f;
	Vector2 dir = Vector2Normalize(Vector2Subtract(p2, p1));
	Vector2 perp = { -dir.y, dir.x };
	perp = Vector2Scale(perp, thickness / 2);

	Vector2 v1 = Vector2Add(p1, perp);
	Vector2 v2 = Vector2Subtract(p1, perp);
	Vector2 v3 = Vector2Add(p2, perp);
	Vector2 v4 = Vector2Subtract(p2, perp);

	rlBegin(RL_QUADS);
	rlTexCoord2f(0.0f, 0.0f); rlVertex2f(v1.x, v1.y);
	rlTexCoord2f(0.0f, 1.0f); rlVertex2f(v2.x, v2.y);
	rlTexCoord2f(1.0f, 1.0f); rlVertex2f(v4.x, v4.y);
	rlTexCoord2f(1.0f, 0.0f); rlVertex2f(v3.x, v3.y);
	rlEnd();

	EndShaderMode();
}


void loadMusic() {
	FILE *file = fopen("satie.wav", "rb");
    if (!file) {
        TraceLog(LOG_ERROR, "Failed to open file!");
        exit(0);
    }

    fseek(file, 0, SEEK_END);
    int size = ftell(file);
    fseek(file, 0, SEEK_SET);

    unsigned char *data = (unsigned char *)malloc(size);
    fread(data, 1, size, file);
    fclose(file);

    g.music = LoadMusicStreamFromMemory(".wav", data, size);
}

void loadShadowShader() {
	g.shadowShader = LoadShader("shaders/shadowmap.vs", "shaders/shadowmap.fs");
	g.shadowShader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(g.shadowShader, "viewPos");

	g.lightDir = Vector3Normalize((Vector3){ 0.35f, -1.0f, -0.35f });
	Color lightCol = WHITE;
	g.lightColor = ColorNormalize(lightCol);
	g.lightDirLoc = GetShaderLocation(g.shadowShader, "lightDir");
	g.lightColorLoc = GetShaderLocation(g.shadowShader, "lightColor");
	SetShaderValue(g.shadowShader, g.lightDirLoc, &g.lightDir, SHADER_UNIFORM_VEC3);
	SetShaderValue(g.shadowShader, g.lightColorLoc, &g.lightColor, SHADER_UNIFORM_VEC4);

	g.ambientLoc = GetShaderLocation(g.shadowShader, "ambient");
	SET_FLOAT4(g.ambient, 0.1f, 0.1f, 0.1f, 1.0f);
	SetShaderValue(g.shadowShader, g.ambientLoc, g.ambient, SHADER_UNIFORM_VEC4);
	
	g.lightVPLoc = GetShaderLocation(g.shadowShader, "lightVP");
	g.shadowMapLoc = GetShaderLocation(g.shadowShader, "shadowMap");
	int shadowMapResolution = SHADOWMAP_RESOLUTION;
	SetShaderValue(g.shadowShader, GetShaderLocation(g.shadowShader, "shadowMapResolution"),
				   &shadowMapResolution, SHADER_UNIFORM_INT);
}


void loadLaserShader() {
	g.laserShader = LoadShader(0, "shaders/laser.fs");
	g.timeLoc = GetShaderLocation(g.laserShader, "time");
}

void handleKeys(float dt) {
	if (IsKeyPressed(KEY_F1)) {
		g.showImgui = !g.showImgui;
	}
	if (IsKeyPressed(KEY_F5)) {
		g.freeCamera = !g.freeCamera;
	}

	if (IsKeyDown(KEY_SPACE)) {
		float randomTime = ((float)rand() / (float)RAND_MAX) * g.duration;
		SeekMusicStream(g.music, randomTime);
	}
	
	handleLightKeys(dt);
}

void handleLightKeys(float dt) {
	const float cameraSpeed = 0.01f;
	if (IsKeyDown(KEY_LEFT))
	{
		if (g.lightDir.x < 1.0f)
			g.lightDir.x += cameraSpeed * 60.0f * dt;
	}
	if (IsKeyDown(KEY_RIGHT))
	{
		if (g.lightDir.x > -1.0f)
			g.lightDir.x -= cameraSpeed * 60.0f * dt;
	}
	if (IsKeyDown(KEY_UP))
	{
		if (g.lightDir.z < 1.0f)
			g.lightDir.z += cameraSpeed * 60.0f * dt;
	}
	if (IsKeyDown(KEY_DOWN))
	{
		if (g.lightDir.z > -1.0f)
			g.lightDir.z -= cameraSpeed * 60.0f * dt;
	}
}

struct Mouse {
	Vector2 position;	
	Vector2 prevPosition;
	Vector2 deltaPosition;
};
Mouse mouse = {
	.position = {0.0f, 0.0f},
	.prevPosition =  {0.0f, 0.0f},
	.deltaPosition = {0.0f, 0.0f},
};


const float MAX_ANGLE_Y = 89.0f * DEG2RAD;
const float MIN_ANGLE_Y = 3.5f * DEG2RAD;
void handleMouse(float delta) {
	
	float mouseWheel = GetMouseWheelMove();
    if (mouseWheel != 0) {
        g.cameraDistance -= mouseWheel * 1.0f;
        g.cameraDistance = fmax(5.0f, fmin(g.cameraDistance, 100.0f));
		g.camera.position.x = g.camera.target.x + g.cameraDistance * cosf(g.cameraAngleY) * sinf(g.cameraAngleX);
		g.camera.position.y = g.camera.target.y + g.cameraDistance * sinf(g.cameraAngleY); 
		g.camera.position.z = g.camera.target.z + g.cameraDistance * cosf(g.cameraAngleY) * cosf(g.cameraAngleX);
    }
	
	
	Vector3 cameraOffset = { 
		g.camera.position.x - g.camera.target.x,
		g.camera.position.y - g.camera.target.y,
		g.camera.position.z - g.camera.target.z
	};
	g.cameraDistance = sqrtf(cameraOffset.x * cameraOffset.x +
							 cameraOffset.y * cameraOffset.y +
							 cameraOffset.z * cameraOffset.z);
	
	
	if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
		HideCursor();
		mouse.position = GetMousePosition();
		
		Vector2 center = { GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };
		mouse.deltaPosition = { mouse.position.x - center.x, mouse.position.y - center.y };
		SetMousePosition(center.x, center.y);

		g.cameraAngleX -= mouse.deltaPosition.x * 0.003f;
		g.cameraAngleY += mouse.deltaPosition.y * 0.003f;
		g.cameraAngleY = Clamp(g.cameraAngleY, MIN_ANGLE_Y, MAX_ANGLE_Y);
	
		g.camera.target = Vector3Lerp(g.camera.target, Vector3Zero(), g.animationSpeed * delta);
		g.camera.position.x = g.camera.target.x + g.cameraDistance * cosf(g.cameraAngleY) * sinf(g.cameraAngleX);
		g.camera.position.y = g.camera.target.y + g.cameraDistance * sinf(g.cameraAngleY); 
		g.camera.position.z = g.camera.target.z + g.cameraDistance * cosf(g.cameraAngleY) * cosf(g.cameraAngleX);
	} else  {
		
		if (g.freeCamera)
			UpdateCamera(&g.camera, CAMERA_FREE);
		else 
			ShowCursor();
	}

}

void imguiMenus() {
	rlImGuiBegin();
	
	ImGui::Begin("x");
	ImGui::SeparatorText("Camera");
	ImGui::DragFloat3("position##1", (float *)&g.camera.position, 1.0f, -1000.0f, 1000.0f);
	ImGui::DragFloat3("target##1", (float *)&g.camera.target, 1.0f, -1000.0f, 1000.0f);
	float camAngleX = g.cameraAngleX * RAD2DEG;
	float camAngleY = g.cameraAngleY * RAD2DEG;
	ImGui::DragFloat("angle_x", (float*)&camAngleX, 1.0f, 200.0f, 2000.0f);
	ImGui::DragFloat("angle_y", (float*)&camAngleY, 1.0f, 200.0f, 2000.0f);
	ImGui::Spacing();
	
	ImGui::SeparatorText("Light");
	ImGui::DragFloat3("position##2", (float *)&g.lightCam.position, 1.0f, -1000.0f, 1000.0f);
	ImGui::DragFloat3("target##2", (float *)&g.lightCam.target, 1.0f, -1000.0f, 1000.0f);
	ImGui::Text("lightDir: (%f, %f, %f)", g.lightDir.x, g.lightDir.y, g.lightDir.z);  
	ImGui::DragFloat("factor", (float *)&g.factor, 0.01f, -20.0f, 20.0f);
	
	ImGui::End();
	
	rlImGuiEnd();
}
