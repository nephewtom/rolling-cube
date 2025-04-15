#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"

#define GLSL_VERSION 330
#include "cubemap.c"

struct Skybox {
	Mesh cube;
	Model model;
	Shader cubemap;
};
Skybox skybox;
void loadSkybox() {
	skybox.cube = GenMeshCube(1.0f, 1.0f, 1.0f);
	skybox.model = LoadModelFromMesh(skybox.cube);	
	skybox.model.materials[0].shader = LoadShader(TextFormat("../shaders/skybox.vs", GLSL_VERSION),
												  TextFormat("../shaders/skybox.fs", GLSL_VERSION));
	
	int envMapLoc = GetShaderLocation(skybox.model.materials[0].shader, "environmentMap");
	int mmc[1] = {MATERIAL_MAP_CUBEMAP};
	int valueOne[1] = { 1 }; 
	int valueZero[1] = { 0 }; 
	SetShaderValue(skybox.model.materials[0].shader, envMapLoc, mmc, SHADER_UNIFORM_INT);
	int doGammaLoc = GetShaderLocation(skybox.model.materials[0].shader, "doGamma");
	SetShaderValue(skybox.model.materials[0].shader, doGammaLoc, valueOne, SHADER_UNIFORM_INT);
	int vflippedLoc = GetShaderLocation(skybox.model.materials[0].shader, "vflipped");
	SetShaderValue(skybox.model.materials[0].shader, vflippedLoc, valueOne, SHADER_UNIFORM_INT);
	
	skybox.cubemap = LoadShader(TextFormat("../shaders/cubemap.vs", GLSL_VERSION),
								TextFormat("../shaders/cubemap.fs", GLSL_VERSION));
	int equiMapLoc = GetShaderLocation(skybox.cubemap, "equirectangularMap");
	SetShaderValue(skybox.cubemap, equiMapLoc, valueZero, SHADER_UNIFORM_INT);

	Texture2D panorama = LoadTexture("../assets/tmp/hdr/sunflowers_puresky_4k.hdr");
	skybox.model.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture = 
		GenTextureCubemap(skybox.cubemap, panorama, 1024, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
	UnloadTexture(panorama);
}

Vector2 fullHD = { 1920, 1080 };
int main()
{
	SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    SetTraceLogLevel(LOG_ALL);
	InitWindow(fullHD.x, fullHD.y, "Skybox!");
	SetWindowPosition(25, 50);
	SetTargetFPS(60);

	TRACELOGD("*** Started Skybox! ***");
	
	Camera camera = { 0 };
    camera.position = (Vector3){ 1.0f, 1.0f, 1.0f };    // Camera position
    camera.target = (Vector3){ 4.0f, 1.0f, 4.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type
	
	loadSkybox();
	while (!WindowShouldClose()) // Main game loop
	{

		UpdateCamera(&camera, CAMERA_FIRST_PERSON);
	
		BeginDrawing();
		{
			ClearBackground(RAYWHITE);

			BeginMode3D(camera);
			{
				// We are inside the cube, we need to disable backface culling!
				rlDisableBackfaceCulling();
				rlDisableDepthMask();
				DrawModel(skybox.model, (Vector3){0, 0, 0}, 1.0f, WHITE);
				DrawPlane(Vector3Zero(), (Vector2){100.0f,100.0f}, WHITE);
				rlEnableBackfaceCulling();
				rlEnableDepthMask();

				DrawGrid(10, 1.0f);
			}
			EndMode3D();
			DrawFPS(10, 10);
		}
		EndDrawing();
	}
	UnloadShader(skybox.model.materials[0].shader);
    UnloadTexture(skybox.model.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture);

    UnloadModel(skybox.model);        // Unload skybox model

    CloseWindow();              // Close window and OpenGL context
}
