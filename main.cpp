#include "raylib.h"
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

#define GLSL_VERSION            330
#include "rlights.c"


struct Grid {
	Color color;
};
Grid grid[103][103];
Color defaultGridColor = {212,215,211,255};

Color getRandomColor()
{
	return (Color){
		(unsigned char) GetRandomValue(0, 255), // Red
		(unsigned char) GetRandomValue(0, 255), // Green
		(unsigned char) GetRandomValue(0, 255), // Blue
		255 // Alpha (fully opaque)
	};
}

void initGrid() {
	for (int x = 0; x < 103; x++) {
		for (int y = 0; y < 103; y++) {
			grid[x][y].color = getRandomColor();
		}
	}
}


struct Cube {
	Vector3 position;
	Vector3 targetPosition;
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
	
	Sound rollWavSlow;
	Sound rollWavNormal;
	Sound rollWavFast;
	Sound rollWavMax;
};
Cube cube;

Vector3 cubeInitPos = {0.5f, 0.51f, 0.5f};

class Speed {
public:
    static constexpr float Slow = 2.5f;
    static constexpr float Normal = 3.5f;
    static constexpr float Fast = 4.5;
    static constexpr float Max = 5.5;
};

void initCube() {
	cube = {
		.position = cubeInitPos,
		.targetPosition = cubeInitPos,
		.direction = {-1.0f, 0.0f, 0.0f},
		.moveStep = {0.0f, 0.0f, 0.0f},

		.rotationAxis = {0.0f, 0.0f, 1.0f},
		.rotationOrigin = {0.0f, 0.0f, 1.0f},
		.rotationAngle = 0.0f,
		.transform = MatrixIdentity(),

		.isMoving = false,
		.animationProgress = 0.0f,
		.animationSpeed = Speed::Slow,

		.facesColor = RED,
		.wiresColor = GREEN,
	
		.rollWavSlow = LoadSound("assets/roll-slow.wav"),
		.rollWavNormal = LoadSound("assets/roll-normal.wav"),
		.rollWavFast = LoadSound("assets/roll-fast.wav"),
		.rollWavMax = LoadSound("assets/roll-max.wav"),
	};
}

struct CubeCamera {
	Camera c3d;
	float distance; // distance to target
	Vector3 direction;
	float angleX;
	float angleY;

	Light light;
};
CubeCamera camera;

void initCamera() {
	camera.c3d = {
		.position = (Vector3){7.5f, 7.5f, 0.5f},
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
	camera.c3d.target = Vector3Lerp(camera.c3d.target, cube.targetPosition, cube.animationSpeed * delta);

	camera.c3d.position.x = camera.c3d.target.x + camera.distance * cosf(camera.angleY) * sinf(camera.angleX);
	camera.c3d.position.y = camera.c3d.target.y + camera.distance * sinf(camera.angleY);
	camera.c3d.position.z = camera.c3d.target.z + camera.distance * cosf(camera.angleY) * cosf(camera.angleX);
}

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

struct Keyboard {
	int pressedKey;
	double keyPressTime;
	double keyReleaseTime;
	float releasePressTimeElapsed; // Between press and release
	float lastPressedKeyTime;
		
	bool hasQueuedKey;
	int queuedKey;

	bool cursorHidden;
	bool shaderEnable;
	bool gridRandomColors;
};
Keyboard kb = {
    .pressedKey = 0,
    .keyPressTime = 0.0f,
    .keyReleaseTime = 0.0f,
    .releasePressTimeElapsed = 0.0f,
    .hasQueuedKey = false,
    .queuedKey = 0,

    .cursorHidden = true,
    .shaderEnable = true,
	.gridRandomColors = false,
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
	camera.angleY = Clamp(camera.angleY, 0.1f, PI/2 - 0.3f);

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

void handleMouseButton() {
	if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
		mouseUpdateCameraAngles();
		mouseUpdateCubeDirection();
			
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


void calculateCubeMovement(int pressedKey) {

	if (cube.animationSpeed == Speed::Max) {
		PlaySound(cube.rollWavMax);
	} else if (cube.animationSpeed == Speed::Fast) {
		PlaySound(cube.rollWavFast);
	} else if (cube.animationSpeed == Speed::Normal) {
		PlaySound(cube.rollWavNormal);
	} else {
		PlaySound(cube.rollWavSlow);
	}
	
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

	cube.targetPosition.x = cube.position.x + cube.moveStep.x;
	cube.targetPosition.y = cube.position.y + cube.moveStep.y;
	cube.targetPosition.z = cube.position.z + cube.moveStep.z;
                
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
		cube.position = cube.targetPosition;
		cube.isMoving = false;
		cube.animationProgress = 0.0f;
		cube.rotationAngle = 0.0f;
	}
}

void handleKeyboard() {
	if (IsKeyPressed(KEY_F1)) {
		kb.cursorHidden = !kb.cursorHidden;
		if (kb.cursorHidden)
			HideCursor();
		else 
			ShowCursor();
	}
	if (IsKeyPressed(KEY_F4)) { kb.shaderEnable = !kb.shaderEnable; }
	if (IsKeyPressed(KEY_F10)) { imgui_demo_window = !imgui_demo_window; }
	if (IsKeyPressed(KEY_F5)) { kb.gridRandomColors = !kb.gridRandomColors; }

	int releasedKey =
		IsKeyReleased(KEY_W) ? KEY_W :
		IsKeyReleased(KEY_S) ? KEY_S :
		IsKeyReleased(KEY_A) ? KEY_A :
		IsKeyReleased(KEY_D) ? KEY_D : 0;

	if (kb.pressedKey !=0 && releasedKey == kb.pressedKey) {
		kb.keyReleaseTime = GetTime();
		kb.releasePressTimeElapsed = kb.keyReleaseTime - kb.keyPressTime;
			
		if (kb.releasePressTimeElapsed < 0.08) {
			cube.animationSpeed = Speed::Max;
		} else if (kb.releasePressTimeElapsed >= 0.08f && kb.releasePressTimeElapsed < 0.11f) {
			cube.animationSpeed = Speed::Fast;
		} else if (kb.releasePressTimeElapsed >= 0.11f && kb.releasePressTimeElapsed < 0.14f) {
			cube.animationSpeed = Speed::Normal;
		} else {
			cube.animationSpeed = Speed::Slow;
		}
	}
		
	int pressedKey =
		IsKeyPressed(KEY_W) ? KEY_W :
		IsKeyPressed(KEY_S) ? KEY_S :
		IsKeyPressed(KEY_A) ? KEY_A :
		IsKeyPressed(KEY_D) ? KEY_D : 0;
		
	// cube.animationSpeed = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)
	// 	? fastSpeed : normalSpeed;

	if (pressedKey) {
		kb.pressedKey = pressedKey;
		kb.keyPressTime = GetTime();
		if (kb.keyPressTime - kb.keyReleaseTime >= 0.14f) {
			// Too much time between last release and next press
			cube.animationSpeed = Speed::Slow;					
		}

		if (!cube.isMoving) { // First time is pressed
			calculateCubeMovement(pressedKey);
		} else {
			kb.hasQueuedKey = true;
			kb.queuedKey = pressedKey;
		}
	}
	
}

void drawRollingCube() {

	// DrawCubeWiresV(cube.position, (Vector3){1.0f, 1.0f, 1.0f}, BLUE);
  
	if (cube.isMoving) {
			
		rlPushMatrix();
		{
			rlMultMatrixf(MatrixToFloat(cube.transform));
			DrawCube(cube.position, 1.0f, 1.0f, 1.0f, cube.facesColor);
			DrawCubeWires(cube.position, 1.0f, 1.0f, 1.0f, cube.wiresColor);
		}
		rlPopMatrix();

		Vector3 vOffset = Vector3Scale(cube.rotationAxis, 0.2);
		DrawCylinderEx(
			Vector3Subtract(cube.rotationOrigin, vOffset),
			Vector3Add(cube.rotationOrigin, vOffset),
			0.05f, 0.05f, 20, ORANGE);
		// DrawSphere(cube.rotationOrigin, 0.1f, YELLOW);
				
	} else {
		DrawCube(cube.position, 1.0f, 1.0f, 1.0f, cube.facesColor);
		DrawCubeWires(cube.position, 1.0f, 1.0f, 1.0f, cube.wiresColor);
	}
}

void drawGrid() {
	float fx = -50.5f;
	for (int x = 0; x < 103; x++, fx++) {
		float fz = -50.5f;
		for (int z = 0; z < 103; z++, fz++) {
			if (kb.gridRandomColors) {
				DrawPlane({fx, 0.0f, fz}, {1.0f, 1.0f}, grid[x][z].color);
			} else {
				DrawPlane({fx, -0.1f, fz}, {1.0f, 1.0f}, defaultGridColor);
			}
			DrawLine3D({ fx - 0.5f, 0.0f, fz - 0.5f }, { fx + 0.5f, 0.0f, fz - 0.5f }, BLACK);
			DrawLine3D({ fx + 0.5f, 0.0f, fz - 0.5f }, { fx + 0.5f, 0.0f, fz + 0.5f }, BLACK);
			DrawLine3D({ fx - 0.5f, 0.0f, fz - 0.5f }, { fx - 0.5f, 0.0f, fz + 0.5f }, BLACK);
			DrawLine3D({ fx - 0.5f, 0.0f, fz + 0.5f }, { fx + 0.5f, 0.0f, fz + 0.5f }, BLACK);
		}
	}
}

void drawStuff() {
	// DrawGrid(100, 1.0f);
	// DrawPlane({0.0f, -0.1f, 0.0f}, (Vector2) { 100.0, 100.0 }, WHITE);
	drawGrid();
	DrawCube({-3.5f, 0.51f, 2.5f}, 1.0f, 1.0f, 1.0f, BLUE);

	DrawCubeWires({-3.5f, 0.51f, 2.5f}, 1.0f, 1.0f, 1.0f, GREEN);

	drawRollingCube();	
}

void imguiMenus();

Vector2 fullHD = { 1920, 1080 };
int main()
{
	SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    SetTraceLogLevel(LOG_ALL);
	InitWindow(fullHD.x, fullHD.y, "Cube!");
	SetWindowPosition(25, 50);

	TRACELOGD("*** Started Cube! ***");
	SetTargetFPS(60);
	InitAudioDevice();

	bool cursorHidden = true;
	HideCursor();

	bool shaderEnable = true;
	Shader shader = LoadShader(TextFormat("shaders/lighting.vs", GLSL_VERSION),
							   TextFormat("shaders/lighting.fs", GLSL_VERSION));
	shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");

	int ambientLoc = GetShaderLocation(shader, "ambient");
	float ftmp[4] = {0.2f, 0.5f, 0.2f, 1.0f};
	SetShaderValue(shader, ambientLoc, &ftmp, SHADER_UNIFORM_VEC4);


	camera.light = CreateLight(LIGHT_POINT, camera.c3d.position, cube.position, WHITE, shader);
	// Light redLight = CreateLight(LIGHT_POINT, { 10.0f, 5.0f, 10.0f }, { 10.0f, 0.0f, 10.0f }, RED, shader);
	// Light blueLight = CreateLight(LIGHT_POINT, { 10.0f, 5.0f, -10.0f }, { 10.0f, 0.0f, -10.0f }, BLUE, shader);
	// Light greenLight = CreateLight(LIGHT_POINT, { -10.0f, 5.0f, -10.0f }, { -10.0f, 0.0f, -10.0f }, GREEN, shader);
	// // Light yellowLight = CreateLight(LIGHT_POINT, { 20.0f, 3.0f, -20.0f }, { 20.0f, 0.0f, -20.0f }, YELLOW, shader);
	// // Light blueLight2 = CreateLight(LIGHT_POINT, { 25.0f, 3.0f, 20.0f }, { 25.0f, 0.0f, 20.0f }, BLUE, shader);

	initGrid();
	initCube();
	initCamera();
    
	rlImGuiSetup(true);

	
	while (!WindowShouldClose()) // Main game loop
	{
		float delta = GetFrameTime();

		handleMouseButton();
		handleMouseWheel();
		handleKeyboard();

		if (cube.isMoving) {
			updateCubeMovement(delta);
		} else if (kb.hasQueuedKey) {
			calculateCubeMovement(kb.queuedKey);
			updateCubeMovement(delta);
			kb.hasQueuedKey = false;
		}

		updateCamera(delta);

		// Update the shader with the camera view vector (points towards { 0.0f, 0.0f, 0.0f })
		float cameraPos[3] = { camera.c3d.position.x, camera.c3d.position.y, camera.c3d.position.z };
		SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);

		camera.light.position = camera.c3d.position;
		camera.light.target = cube.position;
		UpdateLightValues(shader, camera.light);
		
		BeginDrawing();
		{
			ClearBackground(RAYWHITE);

			BeginMode3D(camera.c3d);
			{
				drawAxis();
				// drawGrid();
				if (shaderEnable) {
					BeginShaderMode(shader);
					drawStuff();
					EndShaderMode();
				} else {
					drawStuff();
				}
			}
			EndMode3D();

			if (!cursorHidden) {
				imguiMenus();
			}
			
			DrawText(TextFormat("cube.position: {%.2f, %.2f, %.2f}",
								cube.position.x, cube.position.y, cube.position.z),
					 10, 10, 20, BLACK);
			DrawText(TextFormat(
						 "cube.targetPosition: {%.2f, %.2f, %.2f}",
						 cube.targetPosition.x,
						 cube.targetPosition.y,
						 cube.targetPosition.z),
					 10, 30, 20, BLACK);

			DrawText(TextFormat("pressedKey: %d", kb.pressedKey),
					 10, 80, 20, BLACK);
			DrawText(TextFormat("releasePressTimeElapsed: %.4f", kb.releasePressTimeElapsed),
					 10, 100, 20, BLACK);
			DrawText(TextFormat("cube.animationSpeed: %.4f", cube.animationSpeed),
					 10, 120, 20, BLACK);
		}
		EndDrawing();
	}
	rlImGuiShutdown();

	UnloadShader(shader);   // Unload shader
	// UnloadSound(cube.rollWav);
	CloseAudioDevice();
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
void imguiMenus() {

	ImVec4 cubeFacesColor = RaylibColorToImVec4(cube.facesColor);
	ImVec4 cubeWiresColor = RaylibColorToImVec4(cube.wiresColor);
	ImVec4 gridColor = RaylibColorToImVec4(defaultGridColor);

	rlImGuiBegin();
	ImGui::Begin("ImGui window");

	ImGui::SeparatorText("Cube");
	ImGui::DragFloat3("position", (float *)&cube.position, 1.0f, -1000.0f, 1000.0f);
	ImGui::DragFloat3("direction", (float *)&cube.direction, 1.0f, -1000.0f, 1000.0f);
	ImGui::DragFloat3("rotationAxis", (float *)&cube.rotationAxis, 1.0f, -1000.0f, 1000.0f);
	ImGui::DragFloat("rotationAngle", (float *)&cube.rotationAngle, 1.0f, 200.0f, 2000.0f);
	if (ImGui::ColorEdit4("faces color", &cubeFacesColor.x)) {
		cube.facesColor = ImVec4ToRaylibColor(cubeFacesColor);

	}
	if (ImGui::ColorEdit4("wires color", &cubeWiresColor.x)) {
		cube.wiresColor = ImVec4ToRaylibColor(cubeWiresColor);
	}

	ImGui::Spacing();

	ImGui::SeparatorText("Camera");
	ImGui::DragFloat3("position ", (float *)&camera.c3d.position, 1.0f, -1000.0f, 1000.0f);
	ImGui::DragFloat3("target", (float *)&camera.c3d.target, 1.0f, -1000.0f, 1000.0f);
	float camAngleX = camera.angleX * RAD2DEG;
	float camAngleY = camera.angleY * RAD2DEG;
	ImGui::DragFloat("angle_x", (float*)&camAngleX, 1.0f, 200.0f, 2000.0f);
	ImGui::DragFloat("angle_y", (float*)&camAngleY, 1.0f, 200.0f, 2000.0f);

	ImGui::DragFloat3("lightPosition ", (float *)&camera.light.position, 1.0f, -1000.0f, 1000.0f);
	ImGui::DragFloat3("lightTarget", (float *)&camera.light.target, 1.0f, -1000.0f, 1000.0f);

	ImGui::Spacing();

	ImGui::SeparatorText("Lights");

	if (ImGui::ColorEdit4("grid color", &gridColor.x)) {
		defaultGridColor = ImVec4ToRaylibColor(gridColor);
	}
	ImGui::End();

	if (imgui_demo_window) {
		ImGui::ShowDemoWindow(&imgui_demo_window);
	}
	rlImGuiEnd();
}
