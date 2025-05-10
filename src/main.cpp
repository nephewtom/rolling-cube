#include "entity.h"
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#include "log.h"
#include "globals.cpp"
#include "entity.cpp"
#include "cube.cpp"

#ifndef NO_IMGUI
#include "imguiOptions.cpp"
#endif

#include "timer.cpp"
Timer activationLightTimer("3secsTimer");
void testLightMovement();
bool spawnCube = false;
Vector3 spawnPos = { 49.5f, -0.5f, 50.5f };
void updateSpawnedCube();

void initWave();
PositionIndex setupMap();

void handleMouseButtons();
void handleMouseWheel();
void handleKeyboard();
void drawEntities();
void drawText(int margin);

void handleDroppedFiles();
void loadNewMap(const char* name);

//********** Main
Vector2 fullHD = { 1920, 1080 };
int screenWidth = fullHD.x;
int screenHeight = fullHD.y;


Model semiSphere;


void drawGrid(int slices, float spacing)
{
    int halfSlices = slices/2;

    rlBegin(RL_LINES);
	for (int i = -halfSlices; i <= halfSlices; i++)
	{
		if (i == 0)
		{
			rlColor3f(0.5f, 0.5f, 0.5f);
		}
		else
		{
			rlColor3f(0.75f, 0.75f, 0.75f);
		}

		rlVertex3f((float)i*spacing, -0.1f, (float)-halfSlices*spacing);
		rlVertex3f((float)i*spacing, -0.1f, (float)halfSlices*spacing);

		rlVertex3f((float)-halfSlices*spacing, -0.1f, (float)i*spacing);
		rlVertex3f((float)halfSlices*spacing, -0.1f, (float)i*spacing);
	}
    rlEnd();
}


int main()
{
	SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
	SetTraceLogLevel(LOG_ALL);
	InitWindow(screenWidth, screenHeight, "Cube!");
	SetWindowPosition(25, 50);

	LOGD("*** Started Cube! ***");
	
// SetTargetFPS(60);
	InitAudioDevice();
	initWave();

	if (mouse.cursorHidden) {
		HideCursor();
	}
	
	sld.loadShader();
	sld.loadLogo();
	sld.createLights();
	
	skybox.load();

	ground.init(sld.shader, sld.logo, "./assets/test-map.png");
	entityPool.init(1000);
	entityModels.init();
	PositionIndex initPos = setupMap();
	
	
	
	
	Vector3 cubeInitPos = { initPos.x + 0.5f, 0.51f, initPos.z+ 0.5f};
	cube.init(cubeInitPos);
	camera.init(cubeInitPos);	

	int instancing = 0;
	int instancingLoc = GetShaderLocation(sld.shader, "instancing");
	SetShaderValue(sld.shader, instancingLoc, &instancing, SHADER_UNIFORM_INT);

	activationLightTimer.start(3.0f);
	
#ifndef NO_IMGUI
	rlImGuiSetup(true);
#endif

	Mesh sphere = GenMeshHemiSphere(0.5, 32, 32);
	semiSphere = LoadModelFromMesh(sphere);
	semiSphere.materials[0].shader = sld.shader;
	semiSphere.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = ground.logoGround;
		
	LOGD("Load rock");
	Model rock = LoadModel("assets/rock.obj");
	rock.materials[0].shader = sld.shader;

	LOGD("Load tom-rock");
	Model tom_rock2 = LoadModel("assets/tom-rock.obj");
	tom_rock2.materials[0].shader = sld.shader;
	tom_rock2.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = LoadTexture("assets/Rock020_2K-PNG_Color.png");
	tom_rock2.materials[0].maps[MATERIAL_MAP_NORMAL].texture = LoadTexture("assets/Rock020_2K-PNG_NormalGL.png");
	

	LOGD("Load cubeGtlfModel");	
	Model cubeGtlfModel = LoadModel("tests/assets/tom-cube.gltf");
	cubeGtlfModel.materials[0].shader = sld.shader;

	LOGD("Load cubeObjModel");	
	Model cubeObjModel = LoadModel("assets/tom-cube.obj");
	cubeObjModel.materials[0].shader = sld.shader;

	Model cubeWithLogo = LoadModelFromMesh(GenMeshCube(1,1,1));
	cubeWithLogo.materials[0].shader = sld.shader;
	cubeWithLogo.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = sld.logo;
	
	Model cubeWithLogo2 = LoadModelFromMesh(GenMeshCube(1,1,1));
	// cubeWithLogo.materials[0].shader = sld.shader;
	cubeWithLogo2.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = sld.logo;
	
	while (!WindowShouldClose()) // Main game loop
	{
		delta = GetFrameTime();

		handleMouseButtons();
		handleMouseWheel();
		handleKeyboard();

		if (cube.state != Cube::QUIET) {
			cube.update();
		}
		else if (kb.hasQueuedKey) {
			cube.checkMovement(kb.queuedKey);
			cube.update();
			
			if (!kb.shiftPressed)
				kb.hasQueuedKey = false;
		}

		camera.update();

		// Update the shader with the camera view vector (points towards { 0.0f, 0.0f, 0.0f })
		float cameraPos[3] = { camera.c3d.position.x, camera.c3d.position.y, camera.c3d.position.z };
		SetShaderValue(sld.shader, sld.shader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);


		SetShaderValue(sld.shader, sld.ambientLoc, &sld.ambient, SHADER_UNIFORM_VEC4);

		sld.updateLights();

		testLightMovement();
		if (spawnCube) {
			updateSpawnedCube();
		}
		
		handleDroppedFiles();
		
		BeginDrawing();
		{
			ClearBackground(BLACK);

			BeginMode3D(camera.c3d);
			{
				skybox.draw(cube.direction);
				
				// Draw Ground & axis
				if (ops.coloredGround) {
					ground.drawColored();
				} else {
					ground.drawInstances(sld.shader);
					drawGrid(200, 1);
					DrawModel(rock, { 5.5f, 0.2f, 2.5f }, 0.30f, WHITE);
					DrawModel(tom_rock2, { 7.5f, 0.2f, 2.5f }, 1.0f, WHITE);
					
					DrawModel(cubeWithLogo, { 9.5f, 0.5f, 2.5f }, 1.0f, WHITE);
					DrawModel(cubeWithLogo2, { 9.5f, 0.5f, 4.5f }, 1.0f, WHITE);
				}
				if (ops.drawAxis) {
					drawAxis();
				}

				{// Draw Cube & texture sample
					// Draw several planes with ground texture to check its appearance
					DrawModel(ground.model, {-2.5f,0.05,-2.5f}, 1.0f, RED);
					DrawModel(ground.model, {-3.5f,0.05,-2.5f}, 1.0f, RED);
					DrawModel(ground.model, {-4.5f,0.05,-2.5f}, 1.0f, RED);
					DrawModel(ground.model, {-4.5f,0.05,-3.5f}, 1.0f, RED);

					cube.draw();
				}
				
				// Draw entities, spawn cube and lights
				drawEntities();
				if (spawnCube) {
					DrawModel(cube.model, spawnPos, 1.0f, MAGENTA);
				}				
				sld.drawLights();

			}
			EndMode3D();

			int tp = 10; // topMargin
			DrawRectangle(8, 8, 200, 64, RAYWHITE);
			DrawFPS(12, tp);
			DrawText("F1 - Toggle Menus", 12, tp + 20, 20, BLACK);
			DrawText("F4 - Edit Enabled", 12, tp + 40, 20, BLACK);
			// DrawText("F5 - Toggle ground colors", 12, tp + 60, 20, BLACK);

#ifndef NO_IMGUI
			if (!mouse.cursorHidden) {
				imguiMenus(cube, camera);
			}
#endif

// drawText(tp+120);
		}
		EndDrawing();
	}
#ifndef NO_IMGUI
	rlImGuiShutdown();
#endif
	LOGD("Ending program!");
	
	UnloadShader(sld.shader);
	UnloadShader(skybox.model.materials[0].shader);
	UnloadTexture(skybox.model.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture);
	UnloadModel(skybox.model);
	
	UnloadSound(cube.rollWav);
	CloseAudioDevice();
	CloseWindow();
    return 0;
}

void drawText(int margin) {
	DrawText(TextFormat("mouse.position: {%.2f, %.2f}",
						mouse.position.x, mouse.position.y),
			 10, margin, 20, BLACK);
}

void editEntity() {
	PositionIndex pIndex;
	getMouseXZindexes(camera.c3d, pIndex);
	LOGD("Entity at pIndex: (%i, %i)", pIndex.x, pIndex.z);
	if (!isValidPositionIndex(pIndex)) {
		LOGW(": Invalid pIndex!");
		return; // Invalid index
	}
		
	if (ground.cells[pIndex.x][pIndex.z].isEmpty) {
		int id = entityPool.add(pIndex, (BoxType)ops.entityType);
		ground.cells[pIndex.x][pIndex.z].entityId = id;
		ground.cells[pIndex.x][pIndex.z].isEmpty = false;
		LOGD("Added entity!");			
			
	} else {
		int id = ground.cells[pIndex.x][pIndex.z].entityId;
		LOGD("entity to swap: %i\n", id);
		ground.cells[pIndex.x][pIndex.z].entityId = -1;
		ground.cells[pIndex.x][pIndex.z].isEmpty = true;
		
		EntityQuery eq = { {}, id };
		entityPool.remove(eq);
		
		// store new entityId in the position
		ground.cells[eq.pIndex.x][eq.pIndex.z].entityId = eq.id;
		LOGD("Updated cell: ground.cells[%i][%i].entityId = %i\n", eq.pIndex.x, eq.pIndex.z, eq.id);		
	}
}


//********** Mouse management
const float MAX_ANGLE_Y = 89.0f * DEG2RAD;
const float MIN_ANGLE_Y = -1.5f * DEG2RAD;
void handleMouseButtons() {
	
	mouse.position = GetMousePosition();
	
	if (mouse.cursorHidden) {
		// In gameplay, update mouse position to the center, to avoid reaching screen limits
		Vector2 center = { GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };
		mouse.deltaPosition = { mouse.position.x - center.x, mouse.position.y - center.y };
		SetMousePosition(center.x, center.y);
		
	} else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
		// In edit mode, screen limits do not matter much
		if (mouse.prevPosition.x != 0.0f || mouse.prevPosition.y != 0.0f) {
			mouse.deltaPosition.x = mouse.position.x - mouse.prevPosition.x;
			mouse.deltaPosition.y = mouse.position.y - mouse.prevPosition.y;
		}
	}
	mouse.prevPosition = mouse.position;
	
	camera.angleX -= mouse.deltaPosition.x * 0.003f;
	camera.angleY += mouse.deltaPosition.y * 0.003f;
	camera.angleY = Clamp(camera.angleY, MIN_ANGLE_Y, MAX_ANGLE_Y);
	
	cube.updateDirection();
	
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !mouse.cursorHidden && ops.editEnabled) {
		editEntity();
	}
}

void handleMouseWheel() {
	
	if (!mouse.zoomEnabled) return;
	// Handle mouse wheel for zoom
	float mouseWheel = GetMouseWheelMove();
    if (mouseWheel != 0) {
        camera.distance -= mouseWheel * mouse.zoomSpeed;
        camera.distance = fmax(mouse.minZoomDistance, fmin(camera.distance, mouse.maxZoomDistance));
    }
}

//********** Keyboard management
void handleKeyboard() {
	
	if (IsKeyPressed(KEY_F1)) {
		mouse.cursorHidden = !mouse.cursorHidden;
		if (mouse.cursorHidden)
			HideCursor();
		else 
			ShowCursor();
	}
	if (IsKeyPressed(KEY_F10)) { ops.demoWindow = !ops.demoWindow; }
	if (IsKeyPressed(KEY_F4)) { ops.editEnabled = !ops.editEnabled; }
	if (IsKeyPressed(KEY_F5)) { ops.coloredGround = !ops.coloredGround; }
	if (IsKeyPressed(KEY_F11)) { ToggleFullscreen(); }

	
	// TODO: Tom, check what this function does
	IsKeyPressedRepeat(0);
	
	kb.shiftPressed = IsKeyDown(kb.pressedKey) && (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT));
	
	int releasedKey =
		IsKeyReleased(KEY_W) ? KEY_W :
		IsKeyReleased(KEY_S) ? KEY_S :
		IsKeyReleased(KEY_A) ? KEY_A :
		IsKeyReleased(KEY_D) ? KEY_D : 0;

	if (kb.pressedKey !=0 && releasedKey == kb.pressedKey) {
		kb.keyReleaseTime = GetTime();
		kb.pressReleaseTime = kb.keyReleaseTime - kb.keyPressTime;
		float t = kb.pressReleaseTime;
		
		if (!kb.shiftPressed)
			cube.animationSpeed = KeyDelay::lerpSpeed(t, 0.01f, 0.5f);
	}
		
	int pressedKey =
		IsKeyPressed(KEY_W) ? KEY_W :
		IsKeyPressed(KEY_S) ? KEY_S :
		IsKeyPressed(KEY_A) ? KEY_A :
		IsKeyPressed(KEY_D) ? KEY_D : 0;
		
	// cube.animationSpeed = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)
	// 	? fastSpeed : normalSpeed;

	if (kb.shiftPressed) {
		if (pressedKey == 0)
			pressedKey = kb.pressedKey;
	} 

	if (pressedKey) {
		
		kb.pressedKey = pressedKey;
		kb.keyPressTime = GetTime();

		float t = kb.keyPressTime - kb.keyReleaseTime;
		// Different than before (pressed after released)                 
		if (!kb.shiftPressed)
			cube.animationSpeed = KeyDelay::lerpSpeed(t, 0.01f, 0.5f);

		if (cube.state == Cube::QUIET) { // pressed after a stop
			cube.checkMovement(pressedKey);
		}
		else if (!kb.shiftPressed){
			kb.hasQueuedKey = true;
			kb.queuedKey = pressedKey;
		}
	}
}


//********** Sound
// Generate sine wave
#define SAMPLE_RATE 44100  // Standard sample rate
#define SAMPLES 5500    // 1 second of audio

Wave wave = {
	.frameCount = SAMPLES,
	.sampleRate = 44100, 
	.sampleSize = 16, // 16-bit samples
	.channels = 1,	  // Mono sound
	.data = 0
};

float frequency[5] = { 220, 246.94, 277.18, 329.63, 369.99 };
Sound waveSound[5] = {};
void initWave() {
	short* data = (short*)MemAlloc(SAMPLES * sizeof(short)); // Allocate buffer
    
	for (int i=0; i<5; i++) {
		for (int s = 0; s < SAMPLES; s++) {
			float t = (float)s / SAMPLE_RATE; // Time
			data[s] = (short)(sinf(2.0f * PI * frequency[i] * t) * 32000); // Sine wave
		}
		wave.data = data;
		waveSound[i] = LoadSoundFromWave(wave);
	}
}

//********** Timers
Timer moveTimer("MoveTimer");
int countTimer = 0;
bool spawnDirUp = true;
void testLightMovement() {

	if (activationLightTimer.isEnabled()) {
		activationLightTimer.update(delta);
		if (!activationLightTimer.isDone()) {
			return;
		}
		sld.lights[1].position.x = 5.5f;
		sld.lights[1].enabled = true;
		moveTimer.start(0.5f);
		// playSound(waveSound[0]);
	}

	moveTimer.update(delta);
	if (!moveTimer.isDone()) {
		return;
	}
	countTimer++;
	if (countTimer == 5) {
		// LOGD("testLightMovement: countTimer = 5!");
		sld.lights[1].enabled = false;
		countTimer = 0;
		activationLightTimer.start(3.0f);
		spawnCube = true;
		spawnDirUp = true;
		return;
	}
	// playSound(waveSound[countTimer]);
	sld.lights[1].position.x += 1.0f;
	moveTimer.start();
}

float EaseInOut(float t) {
    return t * t * (3.0f - 2.0f * t); // Smoothstep function
}

float elapsedTime = 0.0f;
void updateSpawnedCube() {
    if (elapsedTime < 1.0f) { //secs
		// float t = elapsedTime / duration; // Normalize time [0,1]
        // cubePosition.y = startY + t * (endY - startY); // Lerp formula
		float t = elapsedTime / 1.0f;
		
		if (spawnDirUp == true)
			spawnPos.y = -1.0f + EaseInOut(t) * (3.0f + 1.0f);
		else 
			spawnPos.y = 3.0f + EaseInOut(t) * (-1.0f - 3.0f);

		elapsedTime += delta;
    } else {
		if (spawnDirUp) {
			spawnDirUp = false;
			elapsedTime = 0.0f;
		} else {
			spawnCube = false;
			elapsedTime = 0.0f;
			spawnDirUp = true;
		}
	}
}

// ****** Entities (Obstacles, Pushbox, etc)
void drawEntities() {
	for (int i=0; i<entityPool.getCount(); i++) {
		Entity e = entityPool.getEntity(i);
		Vector3 v = getPositionFromIndexes(e.pIndex);
		Vector3 vOffset = Vector3Add(v, {0, -0.5, 0});	
		
		Color color = 
			e.type == OBSTACLE ? RED :
			e.type == PUSHBOX ? BLUE : 
			e.type == PULLBOX ? GREEN : 
			e.type == PUSHPULLBOX ? YELLOW : 
			MAGENTA;
					
		if (e.type == WALL) {
			DrawModel(entityModels.wall, v, 1.0f, RED);
		} else if (e.type == OBSTACLE) {
			DrawModel(entityModels.obstacle, vOffset, 1.0f, PINK);
		} else if(!e.hidden) {
			if (e.type  == PUSHBOX)
				DrawModel(entityModels.pushBox, vOffset, 1.0f, color);
			else 
				DrawModel(entityModels.pullBox, vOffset, 1.0f, color);
		}
	}
}


namespace Map {
	Color Red = { 255, 0, 0, 255 };
	Color Orange = { 255, 161, 0, 255 };
	Color Green = { 0, 255, 0, 255 };
	Color Blue = { 0, 0, 255, 255 };
	Color Yellow = { 255, 255, 0, 255 };
	Color Black = { 0, 0, 0, 255 };
	Color Transparent = { 255, 255, 255, 0 };
};

PositionIndex setupMap() {
	
	LOGD("Setting map with: width=%i, height=%i", ground.width, ground.height);
	PositionIndex cubePosIndex = { 0, 0 };
	Vector3 farAway = { 0.0, -1000000.0f, 0.0f };
	int i=0;
	for (int ix = 0; ix < ground.width; ix++) {
		for (int iz = 0; iz < ground.height; iz++) {
			
			int id;
			PositionIndex pi = { ix, iz };
			ground.cells[ix][iz].isEmpty = false;
			
			int colorIndex = ix + iz*ground.width;
			Color color = ground.pixelMap[colorIndex];
			if (ColorIsEqual(color, Map::Red)) {
				id = entityPool.add(pi, WALL);			
			} else if (ColorIsEqual(color, Map::Orange)) {
				id = entityPool.add(pi, OBSTACLE);			
			} else if (ColorIsEqual(color, Map::Green)) {
				id = entityPool.add(pi, PULLBOX);
			} else if (ColorIsEqual(color, Map::Blue)) {
				id = entityPool.add(pi, PUSHBOX);
			} else if (ColorIsEqual(color, Map::Yellow)) {
				id = entityPool.add(pi, PUSHPULLBOX);
			} else {
				id = -1;
				ground.cells[ix][iz].isEmpty = true;
				if (ColorIsEqual(color, Map::Black)) { // player cube
					cubePosIndex = pi;
				}
			}
			ground.cells[ix][iz].entityId = id;
			ground.cells[ix][iz].color = ground.getRandomColor();

			Vector3 move = { ix + 0.5f, 0.0f, iz + 0.5f };
			if (ColorIsEqual(color, Map::Transparent)) {
				move = Vector3Add(move, farAway);
			}
			ground.transforms[i] = MatrixTranslate(move.x, move.y, move.z);
			i++;
		}
	}
	
	return cubePosIndex;
}

void loadNewMap(const char* name) {

	ground.clearGroundMap();
	entityPool.freeEntities();
					
	ground.loadGroundMap(name);
	entityPool.init(500);
					
	PositionIndex pi = setupMap();
	cube.pIndex = pi;
	cube.position = { pi.x + 0.5f, 0.51f, pi.z+ 0.5f};
	camera.c3d.position = Vector3Add(cube.position, Vector3({9.5f, 2.5f, 0.5f}));
	camera.c3d.target = cube.position;

	cube.animationProgress = 0.0f;
	cube.nextPosition = cube.position;
	cube.update();
}

void handleDroppedFiles() {
	if (IsFileDropped()) {
		FilePathList droppedFiles = LoadDroppedFiles();
		if (droppedFiles.count == 1) {// Only support one file 
				
			const char* droppedFile = droppedFiles.paths[0];
			if (IsFileExtension(droppedFile, ".png")) {
					
				loadNewMap(droppedFile);
					
			} else if (IsFileExtension(droppedFile, ".hdr")) {
				skybox.update();
			}
		}
		UnloadDroppedFiles(droppedFiles);
	}
}
