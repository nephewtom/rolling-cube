#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#include "imgui.h"
#include "rlImGui.h"

Camera3D camera = {
    .position = {9.0f, 7.0f, 3.0f},
    .target = {-1.5f, 0.0f, 2.4f},
    .up = {0.0f, 1.0f, 0.0f},
    .fovy = 45.0f,
    .projection = CAMERA_PERSPECTIVE,
};

struct Mouse {
	Vector2 position;
	Vector2 prevPosition;
	Vector2 deltaPosition;

	float cameraDistance;
	float angleX;
	float angleY;
};
Mouse mouse = {
    .position = {0.0f, 0.0f},
    .prevPosition = {0.0f, 0.0f},
    .deltaPosition = {0.0f, 0.0f},

	.cameraDistance = 10.0f,
	.angleX = 7.2f,
	.angleY = 0.5f,
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
	mouse.angleX -= mouse.deltaPosition.x * 0.003f;
	mouse.angleY = mouse.angleY + mouse.deltaPosition.y * 0.003f;
	mouse.angleY = Clamp(mouse.angleY, 0.1f, PI/2 - 0.3f);
}

void handleMouseButton() {
	if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
		mouseUpdateCameraAngles();
	} else {
        mouse.prevPosition = (Vector2){ 0.0f, 0.0f };
    }
}

const float MIN_CAMERA_DISTANCE = 5.0f;  // Minimum zoom distance
const float MAX_CAMERA_DISTANCE = 20.0f;  // Maximum zoom distance
const float ZOOM_SPEED = 1.0f;           // Zoom sensitivity
void handleMouseWheel() {
    // Handle mouse wheel for zoom
	float mouseWheel = GetMouseWheelMove();
	if (mouseWheel != 0) {
		mouse.cameraDistance -= mouseWheel * ZOOM_SPEED;
		mouse.cameraDistance = fmax(MIN_CAMERA_DISTANCE, fmin(mouse.cameraDistance, MAX_CAMERA_DISTANCE));
	}
}

void updateCamera(float delta) {
	Vector3 tmp = camera.target;
	camera.target = { 0, 0, 0 };
	camera.position.x = camera.target.x + mouse.cameraDistance * cosf(mouse.angleY) * sinf(mouse.angleX);
	camera.position.y = camera.target.y + mouse.cameraDistance * sinf(mouse.angleY);
	camera.position.z = camera.target.z + mouse.cameraDistance * cosf(mouse.angleY) * cosf(mouse.angleX);
	camera.target = tmp;
}

Color cubeColor = WHITE;
Color planeColor = WHITE;
Color cubeWireColor = DARKBLUE;
float lineWidth = 1.0f;

Vector3 lightPos = { 2.0f, 4.0f, 2.0f };

float ambientColor[3] = { 0.2f, 0.2f, 0.2f };
float diffuseColor[3] = { 0.8f, 0.8f, 0.8f };
float specularColor[3] = {1.0f, 1.0f, 1.0f};


void imguiMenu();

#define NUM_VERTICES 3 // Example for a triangle

int main()
{
    const int screenWidth = 1280;
    const int screenHeight = 720;
    InitWindow(screenWidth, screenHeight, "Shaded Cube in Raylib");
	SetMousePosition(1024, 415);
	SetTargetFPS(60);
	
	Shader myShader = LoadShader("shaders/base.vs", "shaders/base.fs");
	int lightPosLoc = GetShaderLocation(myShader, "lightPos");
	int viewPosLoc = GetShaderLocation(myShader, "viewPos");
	int ambientLoc = GetShaderLocation(myShader, "ambientColor");
	int diffuseLoc = GetShaderLocation(myShader, "diffuseColor");
	int specularLoc = GetShaderLocation(myShader, "specularColor");
	

	rlImGuiSetup(true);
	while (!WindowShouldClose()) {

		float time = GetTime();

		handleMouseButton();
		handleMouseWheel();

		updateCamera(0);

		Vector3 viewPos = camera.position;		
		SetShaderValue(myShader, lightPosLoc, &lightPos, SHADER_UNIFORM_VEC3);
		SetShaderValue(myShader, viewPosLoc, &viewPos, SHADER_UNIFORM_VEC3);
		SetShaderValue(myShader, ambientLoc, &ambientColor, SHADER_UNIFORM_VEC3);
		SetShaderValue(myShader, diffuseLoc, &diffuseColor, SHADER_UNIFORM_VEC3);
		SetShaderValue(myShader, specularLoc, &specularColor, SHADER_UNIFORM_VEC3);

		BeginDrawing();
		ClearBackground(WHITE);
		int defaultLineWidth = rlGetLineWidth();
		BeginMode3D(camera);
		{
			BeginShaderMode(myShader);
			{
				DrawCube({0,0.5,0}, 1, 1, 1, cubeColor);
				DrawPlane({0,0,0}, { 10.0f, 10.0f }, planeColor);
			}
			EndShaderMode();

			rlSetLineWidth(lineWidth);
			DrawCubeWires((Vector3){0, 0.5, 0}, 1, 1, 1, cubeWireColor);
        
			DrawSphere(lightPos, 0.2f, YELLOW);
			DrawSphereWires(lightPos, 0.21f, 16, 16, ORANGE);

		}
		EndMode3D();

		EndShaderMode();
		imguiMenu();
        
		DrawText("Testing shader", 10, 10, 20, DARKGRAY);
		rlSetLineWidth(defaultLineWidth);
		EndDrawing();
	}
	rlImGuiShutdown();
	UnloadShader(myShader);
    CloseWindow();
    return 0;
}

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
void imguiMenu() {
	rlImGuiBegin();
	ImGui::Begin("ImGui window");
	ImGui::SeparatorText("Element");
	ImVec4 imPlaneColor = RaylibColorToImVec4(planeColor);
	if (ImGui::ColorEdit4("plane color", &imPlaneColor.x)) {
		planeColor = ImVec4ToRaylibColor(imPlaneColor);
	}
	ImVec4 imCubeColor = RaylibColorToImVec4(cubeColor);
	if (ImGui::ColorEdit4("cube color", &imCubeColor.x)) {
		cubeColor = ImVec4ToRaylibColor(imCubeColor);
	}
	ImVec4 imCubeWireColor = RaylibColorToImVec4(cubeWireColor);
	if (ImGui::ColorEdit4("wires color", &imCubeWireColor.x)) {
		cubeWireColor = ImVec4ToRaylibColor(imCubeWireColor);
	}

	ImGui::SliderFloat("lineWidth", &lineWidth, 0.0f, 8.0f, "%.1f");
	ImGui::Spacing();

	ImGui::SeparatorText("Light/Shader");
	ImGui::DragFloat3("lightPos", (float *)&lightPos, 0.02f, -10.0f, 10.0f);
	ImGui::ColorEdit3("ambient", ambientColor);
	ImGui::ColorEdit3("diffuse", diffuseColor);
	ImGui::ColorEdit3("specular", specularColor);
	ImGui::Spacing();
	
	ImGui::SeparatorText("Camera");
	ImGui::DragFloat3("position ", (float *)&camera.position, 0.1f, -10.0f, 10.0f);
	ImGui::DragFloat3("target", (float *)&camera.target, 0.1f, -10.0f, 10.0f);
	ImGui::DragFloat("distance", &mouse.cameraDistance, 0.1, 3.0f, 20.0f);

	ImGui::Spacing();
	ImGui::SeparatorText("Mouse");
	Vector2 mousePos = GetMousePosition();
	if (ImGui::DragFloat2("position  ", (float*)&mousePos, 0.1f, 0.0f, 1000.0f)) {
		SetMousePosition(mousePos.x, mousePos.y);
	}
	ImGui::DragFloat("angleX", &mouse.angleX);
	ImGui::DragFloat("angleY", &mouse.angleY);
	ImGui::End();
	rlImGuiEnd();
}
