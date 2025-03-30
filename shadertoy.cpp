#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#include "imgui.h"
#include "rlImGui.h"

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

void imguiMenu();

Color myColor = RED;

int main()
{
    const int screenWidth = 1280;
    const int screenHeight = 720;
    InitWindow(screenWidth, screenHeight, "Shaded Cube in Raylib");
	SetTargetFPS(60);
		
	Shader shader = LoadShader("shaders/shadertoy.vs", "shaders/shadertoy.fs");
	int resLoc = GetShaderLocation(shader, "resolution");
	int timeLoc = GetShaderLocation(shader, "time");
	int mycolorLoc = GetShaderLocation(shader, "mycolor");
	Vector2 screenSize = { (float)GetScreenWidth(), (float)GetScreenHeight() };
	SetShaderValue(shader, resLoc, &screenSize, SHADER_UNIFORM_VEC2);

	rlImGuiSetup(true);
	while (!WindowShouldClose()) {

		float time = GetTime();
		SetShaderValue(shader, timeLoc, &time, SHADER_UNIFORM_FLOAT);
		ImVec4 v = RaylibColorToImVec4(myColor);
		Vector3 col = { v.x, v.y, v.z };
		SetShaderValue(shader, mycolorLoc, &col, SHADER_UNIFORM_VEC3);

		BeginDrawing();
		ClearBackground(WHITE);

		BeginShaderMode(shader);

		rlBegin(RL_TRIANGLES); // Use quads, or use triangles

		rlVertex2f(-1.0f, -1.0f); // Bottom-left
		rlVertex2f( 1.0f, -1.0f); // Bottom-right
		rlVertex2f(-1.0f,  1.0f); // Top-left

		rlVertex2f(-1.0f,  1.0f); // Top-left
		rlVertex2f( 1.0f, -1.0f); // Bottom-right
		rlVertex2f( 1.0f,  1.0f); // Top-right

		rlEnd();

		EndShaderMode();
		imguiMenu();
        
		EndDrawing();
	}
	rlImGuiShutdown();
	UnloadShader(shader);
    CloseWindow();
    return 0;
}

void imguiMenu() {
	rlImGuiBegin();
	ImGui::Begin("ImGui window");
	ImGui::SeparatorText("Element");
	ImVec4 imMyColor = RaylibColorToImVec4(myColor);
	if (ImGui::ColorEdit4("my color", &imMyColor.x)) {
		myColor = ImVec4ToRaylibColor(imMyColor);
	}
	ImGui::Spacing();

	ImGui::End();
	rlImGuiEnd();
}
