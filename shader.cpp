#include "raylib.h"
#include "raymath.h"

#include "imgui.h"
#include "rlImGui.h"

#define GLSL_VERSION 330

typedef enum { WIREFRAME, GOURAUD, PHONG, TOY } ShaderMode;

Vector3 lightPos = { 2.0f, 4.0f, 2.0f };

float ambientColor[3] = { 0.2f, 0.2f, 0.2f };
float diffuseColor[3] = { 0.8f, 0.8f, 0.8f };
float specularColor[3] = {1.0f, 1.0f, 1.0f};

Camera3D camera = { 0 };

void imguiMenu();

int main()
{
    const int screenWidth = 1280;
    const int screenHeight = 720;
    InitWindow(screenWidth, screenHeight, "Shaded Cube in Raylib");

    camera.position = (Vector3){ 4.0f, 2.0f, 3.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;


	Shader gouraudShader = LoadShader(TextFormat("shaders/lighting.vs", GLSL_VERSION),
									  TextFormat("shaders/gouraud.fs", GLSL_VERSION));

	Shader phongShader = LoadShader(TextFormat("shaders/lighting.vs", GLSL_VERSION),
									TextFormat("shaders/phong.fs", GLSL_VERSION));


	Shader toyShader = LoadShader(TextFormat("shaders/lighting.vs", GLSL_VERSION),
								  TextFormat("shaders/compare.fs", GLSL_VERSION));

	int resolutionLoc = GetShaderLocation(toyShader, "iResolution");
	float resolution[2] = { (float)screenWidth, (float)screenHeight };
	SetShaderValue(toyShader, resolutionLoc, resolution, SHADER_UNIFORM_VEC2);

	ShaderMode mode = WIREFRAME;
    

	int lightPosLoc = GetShaderLocation(gouraudShader, "lightPos");
	int viewPosLoc = GetShaderLocation(gouraudShader, "viewPos");
	int ambientLoc = GetShaderLocation(gouraudShader, "ambientColor");
	int diffuseLoc = GetShaderLocation(gouraudShader, "diffuseColor");
	int specularLoc = GetShaderLocation(gouraudShader, "specularColor");

	int phongLightPosLoc = GetShaderLocation(phongShader, "lightPos");
	int phongViewPosLoc = GetShaderLocation(phongShader, "viewPos");
	int phongAmbientLoc = GetShaderLocation(phongShader, "ambientColor");
	int phongDiffuseLoc = GetShaderLocation(phongShader, "diffuseColor");
	int phongSpecularLoc = GetShaderLocation(phongShader, "specularColor");

	SetTargetFPS(60);
	rlImGuiSetup(true);

	while (!WindowShouldClose()) {

		Vector3 viewPos = camera.position;

		SetShaderValue(gouraudShader, lightPosLoc, &lightPos, SHADER_UNIFORM_VEC3);
		SetShaderValue(gouraudShader, viewPosLoc, &viewPos, SHADER_UNIFORM_VEC3);
		SetShaderValue(gouraudShader, ambientLoc, ambientColor, SHADER_UNIFORM_VEC3);
		SetShaderValue(gouraudShader, diffuseLoc, diffuseColor, SHADER_UNIFORM_VEC3);
		SetShaderValue(gouraudShader, specularLoc, specularColor, SHADER_UNIFORM_VEC3);
    
		SetShaderValue(phongShader, phongLightPosLoc, &lightPos, SHADER_UNIFORM_VEC3);
		SetShaderValue(phongShader, phongViewPosLoc, &viewPos, SHADER_UNIFORM_VEC3);
		SetShaderValue(phongShader, phongAmbientLoc, ambientColor, SHADER_UNIFORM_VEC3);
		SetShaderValue(phongShader, phongDiffuseLoc, diffuseColor, SHADER_UNIFORM_VEC3);
		SetShaderValue(phongShader, phongSpecularLoc, specularColor, SHADER_UNIFORM_VEC3);
				
		if (IsKeyPressed(KEY_ONE)) mode = WIREFRAME;
		if (IsKeyPressed(KEY_TWO)) mode = GOURAUD;
		if (IsKeyPressed(KEY_THREE)) mode = PHONG;
		if (IsKeyPressed(KEY_THREE)) mode = TOY;
        
		BeginDrawing();
		ClearBackground(RAYWHITE);
		BeginMode3D(camera);
        
		if (mode == WIREFRAME)
		{
			DrawCubeWiresV((Vector3){ 0, 0, 0 }, (Vector3){ 2, 2, 2 }, BLACK);
		}
		else
		{
			Shader activeShader = (mode == GOURAUD) ? gouraudShader
				: (mode == PHONG) ? phongShader
				: toyShader;
				  
			BeginShaderMode(activeShader);
			DrawCube((Vector3){ 0, 0, 0 }, 2, 2, 2, GRAY);
			EndShaderMode();
		}
        
		DrawSphere(lightPos, 0.2f, YELLOW);
		EndMode3D();

		imguiMenu();
        
		DrawText("1: Wireframe | 2: Gouraud | 3: Phong", 10, 10, 20, DARKGRAY);
		EndDrawing();
	}
	rlImGuiShutdown();
	UnloadShader(gouraudShader);
    UnloadShader(phongShader);
    CloseWindow();
    return 0;
}

void imguiMenu() {
	rlImGuiBegin();
	ImGui::Begin("ImGui window");
	ImGui::SeparatorText("Shader");
	ImGui::DragFloat3("lightPos", (float *)&lightPos, 1.0f, -10.0f, 10.0f);

	ImGui::ColorEdit3("ambient", ambientColor);
	ImGui::ColorEdit3("diffuse", diffuseColor);
	ImGui::ColorEdit3("specular", specularColor);
	
	ImGui::Spacing();
	ImGui::SeparatorText("Camera");
	ImGui::DragFloat3("position ", (float *)&camera.position, 1.0f, -10.0f, 10.0f);
	ImGui::DragFloat3("target", (float *)&camera.target, 1.0f, -10.0f, 10.0f);
	ImGui::End();
	rlImGuiEnd();
}



// #include "raylib.h"
// #include "rlgl.h"  // Required for low-level OpenGL functions
// #define GLSL_VERSION 330

// int main(void)
// {
//     const int screenWidth = 800;
//     const int screenHeight = 450;


// 	InitWindow(screenWidth, screenHeight, "Raylib Shader Test");
// 	SetExitKey(0); // Disable ESC from closing the window
// 	bool cursorHidden = true; // Track cursor state
// 	DisableCursor();

// 	// Load shader
// 	Shader shader = LoadShader(TextFormat("shaders/gradient.vs", GLSL_VERSION),
// 							   TextFormat("shaders/gradient.fs", GLSL_VERSION));

// 	int resolutionLoc = GetShaderLocation(shader, "iResolution");
// 	float resolution[2] = { (float)screenWidth, (float)screenHeight };
// 	SetShaderValue(shader, resolutionLoc, resolution, SHADER_UNIFORM_VEC2);

// // Check for shader errors
// 	if (shader.id == 0) {
// 		CloseWindow();
// 		return -1;  // Shader failed to load
// 	}

// 	SetTargetFPS(60);

// 	while (!WindowShouldClose()) {

// 		if (IsKeyPressed(KEY_ESCAPE)) // Check if ESC is pressed
// 		{
// 			cursorHidden = !cursorHidden; // Toggle state
// 			if (cursorHidden)
// 				DisableCursor(); // Hide and lock cursor
// 			else
// 				EnableCursor(); // Show cursor
// 		}

		
// 		if (IsKeyDown(KEY_SPACE)) {
// 			ToggleFullscreen();  // Toggle fullscreen mode in Raylib
// 		}
// 		BeginDrawing();
// 		ClearBackground(RAYWHITE);

// 		BeginShaderMode(shader);


// 		rlBegin(RL_QUADS);
// 		rlTexCoord2f(0.0f, 0.0f); rlVertex2f(-screenWidth, -screenHeight);
// 		rlTexCoord2f(1.0f, 0.0f); rlVertex2f(screenWidth, -screenHeight);
// 		rlTexCoord2f(1.0f, 1.0f); rlVertex2f(screenWidth, screenHeight);
// 		rlTexCoord2f(0.0f, 1.0f); rlVertex2f(-screenWidth, screenHeight);
// 		rlEnd();

// 		EndShaderMode();
// 		DrawRectangle(0, 0, 100, 100, WHITE);

// 		DrawText("Shader Test", 10, 10, 20, BLACK);

// 		EndDrawing();
// 	}

// 	UnloadShader(shader);
//     CloseWindow();
//     return 0;
// }
