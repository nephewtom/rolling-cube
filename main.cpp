#include "raylib.h"
#include "raymath.h" // Needed for matrix and quaternion operations
#include "rlgl.h"

#include "imgui.h"
#include "rlImGui.h"

#include <math.h>
#include <stdarg.h>

struct Cube {
    Vector3 position;
    Vector3 targetPosition;
    Vector3 direction;
	Vector3 moveStep;
	
    Vector3 rotationAxis;
    Vector3 rotationOrigin;
    float rotationAngle;
    float targetAngle;
    Matrix transform;
    
    bool isMoving;
    float animationProgress;
    float animationSpeed;
};
Cube cube;

Vector3 cubeInitPos = {0.5f, 0.5f, 0.5f};
float normalSpeed = 2.5f;
float fastSpeed = 4.5f;
void initCube() {
	cube = {
		.position = cubeInitPos,
		.targetPosition = cubeInitPos,
		.direction = {-1.0f, 0.0f, 0.0f},
		.moveStep = {0.0f, 0.0f, 0.0f},
	
		.rotationAxis = {0.0f, 0.0f, 1.0f},
		.rotationOrigin = {0.0f, 0.0f, 1.0f},
		.rotationAngle = 0.0f,
		.targetAngle = 0.0f,
		.transform = MatrixIdentity(),
    
		.isMoving = false,
		.animationProgress = 0.0f,
		.animationSpeed = normalSpeed,
	};
}

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
	  .position = (Vector3){6.0f, 6.0f, 2.0f},
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
	bool hasQueuedKey;	
	int queuedKey;
};
Keyboard keyboard = {
    // .pressedKey = 0,
	.hasQueuedKey = false,
    .queuedKey = 0,
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
	// camera.angleY = fmax(fmin(camera.angleY + mouse.deltaPosition.y * 0.003f, PI / 2.0f), -PI / 2.0f);
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
	cube.targetAngle = PI/2.0f; // 90 degrees in radians
	cube.animationProgress = 0.0f;
	cube.isMoving = true;
}

void updateCubeMovement(float delta) {

	cube.animationProgress += delta * cube.animationSpeed;

	// Use smooth easing for animation
	float t = cube.animationProgress;
	float smoothT = t * t * (3.0f - 2.0f * t); // Smoothstep formula
                    
// Calculate current rotation angle
	cube.rotationAngle = cube.targetAngle * smoothT;
                    
	Matrix translateToOrigin = MatrixTranslate(-cube.rotationOrigin.x, 
											   -cube.rotationOrigin.y, 
											   -cube.rotationOrigin.z);
	Matrix rotation = MatrixRotate(cube.rotationAxis, cube.rotationAngle);
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

void drawRollingCube() {

	// DrawCubeWiresV(cube.position, (Vector3){1.0f, 1.0f, 1.0f}, BLUE);
  
	if (cube.isMoving) {
			
		rlPushMatrix();
		{
			rlMultMatrixf(MatrixToFloat(cube.transform));
			DrawCube(cube.position, 1.0f, 1.0f, 1.0f, RED);
			DrawCubeWires(cube.position, 1.0f, 1.0f, 1.0f, GREEN);
		}
		rlPopMatrix();
				
		DrawSphere(cube.rotationOrigin, 0.1f, YELLOW);
				
	} else {
		DrawCube(cube.position, 1.0f, 1.0f, 1.0f, RED);
		DrawCubeWires(cube.position, 1.0f, 1.0f, 1.0f, GREEN);
	}
}

Vector2 fullHD = { 1920, 1080 };
float screenWidth = fullHD.x;
float screenHeight = fullHD.y;

int main()
{
	SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
	InitWindow(screenWidth, screenHeight, "Cube!");
	SetWindowPosition(25, 50);

	SetTargetFPS(60);
	initCube();
	initCamera();
    
	rlImGuiSetup(true);

    float rectSize = 400.0f;

    while (!WindowShouldClose()) // Main game loop
    {
        float delta = GetFrameTime();

        handleMouseButton();
        handleMouseWheel();
		
		int pressedKey =
			IsKeyPressed(KEY_W) ? KEY_W :
			IsKeyPressed(KEY_S) ? KEY_S :
			IsKeyPressed(KEY_A) ? KEY_A :
			IsKeyPressed(KEY_D) ? KEY_D : 0;

		cube.animationSpeed = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)
			? fastSpeed : normalSpeed;
		
		if (pressedKey) {
			if (!cube.isMoving) {
				calculateCubeMovement(pressedKey);
			} else {
				keyboard.hasQueuedKey = true;
				keyboard.queuedKey = pressedKey;
			}
		}

		if (cube.isMoving) {
			updateCubeMovement(delta);
		} else if (keyboard.hasQueuedKey) {
			calculateCubeMovement(keyboard.queuedKey);
			updateCubeMovement(delta);
			keyboard.hasQueuedKey = false;
		}
		
				
		BeginDrawing();
		ClearBackground(RAYWHITE);

		BeginMode3D(camera.c3d);
		updateCamera(delta);

		DrawGrid(100, 1.0f);
		drawAxis();
		DrawCube({-3.5f, 0.5f, 2.5f}, 1.0f, 1.0f, 1.0f, BLUE);
		DrawCubeWires({-3.5f, 0.5f, 2.5f}, 1.0f, 1.0f, 1.0f, GREEN);

		drawRollingCube();
		EndMode3D();

        rlImGuiBegin();
        ImGui::Begin("ImGui window");
        ImGui::SeparatorText("Rectangle");
        ImGui::DragFloat("rectSize", &rectSize, 1.0f, 100.0f, 400.0f);
        ImGui::End();
        rlImGuiEnd();

        DrawRectangle(1500, 600, rectSize, rectSize, GREEN);
        DrawText(TextFormat("Rectangle rectSize: %.0f", rectSize), 1500, 580, 20, BLACK);

        DrawText(TextFormat("cube.position: {%.2f, %.2f, %.2f}",
                            cube.position.x,
                            cube.position.y,
                            cube.position.z), 10, 10, 20, BLACK);
        DrawText(TextFormat("cube.targetPosition: {%.2f, %.2f, %.2f}",
                            cube.targetPosition.x,
							cube.targetPosition.y,
							cube.targetPosition.z), 10, 30, 20, BLACK);
        DrawText(TextFormat("cube.rotationAxis: {%.2f, %.2f, %.2f}",
                            cube.rotationAxis.x,
                            cube.rotationAxis.y,
                            cube.rotationAxis.z), 10, 50, 20, BLACK);

        DrawText(TextFormat("cube.direction: {%.2f, %.2f, %.2f}",
                            cube.direction.x,
                            cube.direction.y,
                            cube.direction.z), 10, 70, 20, BLACK);

        DrawText(TextFormat("rotationAngle: %.0f", cube.rotationAngle*RAD2DEG), 10, 90, 20, BLACK);
        DrawText(TextFormat("camera.target: {%.2f, %.2f, %.2f}",
                            camera.c3d.target.x,
                            camera.c3d.target.y,
							camera.c3d.target.z), 10, 130, 20, BLACK);

        DrawText(TextFormat("camera.angleX: %.2f", camera.angleX*RAD2DEG), 10, 150, 20, BLACK);
        DrawText(TextFormat("camera.angleY: %.2f", camera.angleY*RAD2DEG), 10, 170, 20, BLACK);

        EndDrawing();
    }
    rlImGuiShutdown();

    CloseWindow();
    return 0;
}

