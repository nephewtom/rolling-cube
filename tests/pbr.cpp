#include "raylib.h"
#include "raymath.h"
#define SUPPORT_TRACELOG
#define SUPPORT_TRACELOG_DEBUG
#include "utils.h"

#include "imgui.h"
#include "rlImGui.h"
#include "utils.h"

#define GLSL_VERSION            330

// Light type
typedef enum {
    LIGHT_DIRECTIONAL = 0,
    LIGHT_POINT,
    LIGHT_SPOT
} LightType;

// Light data
typedef struct {
    int type;
    int enabled;
    Vector3 position;
    Vector3 target;
    float color[4];
    float intensity;

    // Shader light parameters locations
    int typeLoc;
    int enabledLoc;
    int positionLoc;
    int targetLoc;
    int colorLoc;
    int intensityLoc;
} Light;
#define MAX_LIGHTS  6           // Max dynamic lights supported by shader
static int lightCount = 0;     // Current number of dynamic lights that have been created

// Create a light and get shader locations
static Light CreateLight(int type, Vector3 position, Vector3 target, Color color, float intensity, Shader shader);

// Update light properties on shader
// NOTE: Light shader locations should be available
static void UpdateLight(Shader shader, Light light);


Camera3D camera = {
    .position = {3.0f, 7.0f, 3.0f},
    .target = {0.0f, 0.0f, 0.0f},
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
	mouse.angleY = Clamp(mouse.angleY, -0.1f, PI/2 - 0.3f);
}

void handleMouseButton() {
	if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
		mouseUpdateCameraAngles();
	} else {
        mouse.prevPosition = (Vector2){ 0.0f, 0.0f };
    }
}
const float MIN_CAMERA_DISTANCE = 2.0f;  // Minimum zoom distance
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

void imguiMenus();

float ambientIntensity;
Vector3 ambientColorNormalized;
Light lights[MAX_LIGHTS] = { 0 };

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 1920;
    const int screenHeight = 1080;

    SetTraceLogLevel(LOG_ALL);
	SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "raylib [shaders] example - basic pbr");
	SetTargetFPS(60);
	
    // // Define the camera to look into our 3d world
    // Camera camera = { 0 };
    // camera.position = (Vector3){ 2.0f, 2.0f, 6.0f };    // Camera position
    // camera.target = (Vector3){ 0.0f, 0.5f, 0.0f };      // Camera looking at point
    // camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    // camera.fovy = 45.0f;                                // Camera field-of-view Y
    // camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type

    // Load PBR shader and setup all required locations
    Shader shader = LoadShader(TextFormat("shaders/pbr.vs", GLSL_VERSION),
                               TextFormat("shaders/pbr.fs", GLSL_VERSION));
    shader.locs[SHADER_LOC_MAP_ALBEDO] = GetShaderLocation(shader, "albedoMap");
    // WARNING: Metalness, roughness, and ambient occlusion are all packed into a MRA texture
    // They are passed as to the SHADER_LOC_MAP_METALNESS location for convenience,
    // shader already takes care of it accordingly
    shader.locs[SHADER_LOC_MAP_METALNESS] = GetShaderLocation(shader, "mraMap");
    shader.locs[SHADER_LOC_MAP_NORMAL] = GetShaderLocation(shader, "normalMap");
    // WARNING: Similar to the MRA map, the emissive map packs different information 
    // into a single texture: it stores height and emission data
    // It is binded to SHADER_LOC_MAP_EMISSION location an properly processed on shader
    shader.locs[SHADER_LOC_MAP_EMISSION] = GetShaderLocation(shader, "emissiveMap");
    shader.locs[SHADER_LOC_COLOR_DIFFUSE] = GetShaderLocation(shader, "albedoColor");

    // Setup additional required shader locations, including lights data
    shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");
    int lightCountLoc = GetShaderLocation(shader, "numOfLights");
    int maxLightCount = MAX_LIGHTS;
    SetShaderValue(shader, lightCountLoc, &maxLightCount, SHADER_UNIFORM_INT);

    // Setup ambient color and intensity parameters
    Color ambientColor = (Color){ 26, 32, 135, 255 };
    ambientColorNormalized = (Vector3){ ambientColor.r/255.0f, ambientColor.g/255.0f, ambientColor.b/255.0f };
    SetShaderValue(shader, GetShaderLocation(shader, "ambientColor"), &ambientColorNormalized, SHADER_UNIFORM_VEC3);
    ambientIntensity = 0.02f;
    SetShaderValue(shader, GetShaderLocation(shader, "ambient"), &ambientIntensity, SHADER_UNIFORM_FLOAT);

    // Get location for shader parameters that can be modified in real time
    int emissiveIntensityLoc = GetShaderLocation(shader, "emissivePower");
    int emissiveColorLoc = GetShaderLocation(shader, "emissiveColor");
    int textureTilingLoc = GetShaderLocation(shader, "tiling");

    // Load old car model using PBR maps and shader
    // WARNING: We know this model consists of a single model.meshes[0] and
    // that model.materials[0] is by default assigned to that mesh
    // There could be more complex models consisting of multiple meshes and
    // multiple materials defined for those meshes... but always 1 mesh = 1 material
    Model car = LoadModel("assets/old_car_new.glb");

    // Assign already setup PBR shader to model.materials[0], used by models.meshes[0]
    car.materials[0].shader = shader;

    // Setup materials[0].maps default parameters
    car.materials[0].maps[MATERIAL_MAP_ALBEDO].color = WHITE;
    car.materials[0].maps[MATERIAL_MAP_METALNESS].value = 0.0f;
    car.materials[0].maps[MATERIAL_MAP_ROUGHNESS].value = 0.0f;
    car.materials[0].maps[MATERIAL_MAP_OCCLUSION].value = 1.0f;
    car.materials[0].maps[MATERIAL_MAP_EMISSION].color = (Color){ 255, 162, 0, 255 };

    // Setup materials[0].maps default textures
    car.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = LoadTexture("assets/old_car_d.png");
    car.materials[0].maps[MATERIAL_MAP_METALNESS].texture = LoadTexture("assets/old_car_mra.png");
    car.materials[0].maps[MATERIAL_MAP_NORMAL].texture = LoadTexture("assets/old_car_n.png");
    car.materials[0].maps[MATERIAL_MAP_EMISSION].texture = LoadTexture("assets/old_car_e.png");
    
    // Load floor model mesh and assign material parameters
    // NOTE: A basic plane shape can be generated instead of being loaded from a model file
    Model floor = LoadModel("assets/plane.glb");
    //Mesh floorMesh = GenMeshPlane(10, 10, 10, 10);
    //GenMeshTangents(&floorMesh);      // TODO: Review tangents generation
    //Model floor = LoadModelFromMesh(floorMesh);

    // Assign material shader for our floor model, same PBR shader 
    floor.materials[0].shader = shader;
    
    floor.materials[0].maps[MATERIAL_MAP_ALBEDO].color = WHITE;
    floor.materials[0].maps[MATERIAL_MAP_METALNESS].value = 0.0f;
    floor.materials[0].maps[MATERIAL_MAP_ROUGHNESS].value = 0.0f;
    floor.materials[0].maps[MATERIAL_MAP_OCCLUSION].value = 1.0f;
    floor.materials[0].maps[MATERIAL_MAP_EMISSION].color = BLACK;

    floor.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = LoadTexture("assets/road_a.png");
    floor.materials[0].maps[MATERIAL_MAP_METALNESS].texture = LoadTexture("assets/road_mra.png");
    floor.materials[0].maps[MATERIAL_MAP_NORMAL].texture = LoadTexture("assets/road_n.png");

    // Models texture tiling parameter can be stored in the Material struct if required (CURRENTLY NOT USED)
    // NOTE: Material.params[4] are available for generic parameters storage (float)
    Vector2 carTextureTiling = (Vector2){ 0.5f, 0.5f };
    Vector2 floorTextureTiling = (Vector2){ 5.0f, 5.0f };
    Vector2 cubeTextureTiling = (Vector2){ 0.5f, 0.5f };

	
		
	Model cube = LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));
	
	cube.materials[0].shader = shader;
    cube.materials[0].maps[MATERIAL_MAP_ALBEDO].color = WHITE;
    cube.materials[0].maps[MATERIAL_MAP_METALNESS].value = 1.0f;
    cube.materials[0].maps[MATERIAL_MAP_ROUGHNESS].value = 1.0f;
	cube.materials[0].maps[MATERIAL_MAP_OCCLUSION].value = 1.0f;
	cube.materials[0].maps[MATERIAL_MAP_EMISSION].color = WHITE;

	cube.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = LoadTexture("assets/cube_color.jpg");
	cube.materials[0].maps[MATERIAL_MAP_NORMAL].texture = LoadTexture("assets/cube_normal.png");	
	cube.materials[0].maps[MATERIAL_MAP_METALNESS].texture = LoadTexture("assets/cube_color.jpg");	
	cube.materials[0].maps[MATERIAL_MAP_ROUGHNESS].texture = LoadTexture("assets/cube_roughness.jpg");	
	cube.materials[0].maps[MATERIAL_MAP_OCCLUSION].texture = LoadTexture("assets/cube_ao.jpg");
	cube.materials[0].maps[MATERIAL_MAP_HEIGHT].texture = LoadTexture("assets/cube_height.png");
	
// Create some lights
	lights[0] = CreateLight(LIGHT_POINT, (Vector3){ -1.0f, 1.0f, -2.0f }, (Vector3){ 0.0f, 0.0f, 0.0f }, YELLOW, 4.0f, shader);
	lights[1] = CreateLight(LIGHT_POINT, (Vector3){ 2.0f, 1.0f, 1.0f }, (Vector3){ 0.0f, 0.0f, 0.0f }, GREEN, 3.3f, shader);
	lights[2] = CreateLight(LIGHT_POINT, (Vector3){ -2.0f, 1.0f, 1.0f }, (Vector3){ 0.0f, 0.0f, 0.0f }, RED, 8.3f, shader);
	lights[3] = CreateLight(LIGHT_POINT, (Vector3){ 1.0f, 1.0f, -2.0f }, (Vector3){ 0.0f, 0.0f, 0.0f }, BLUE, 2.0f, shader);
	lights[4] = CreateLight(LIGHT_DIRECTIONAL, (Vector3){ 0.0f, 1.5f, 4.0f }, (Vector3){ 0.0f, 0.0f, 0.0f }, WHITE, 2.0f, shader);

	// Setup material texture maps usage in shader
	// NOTE: By default, the texture maps are always used
	int usage = 1;
	SetShaderValue(shader, GetShaderLocation(shader, "useTexAlbedo"), &usage, SHADER_UNIFORM_INT);
	SetShaderValue(shader, GetShaderLocation(shader, "useTexNormal"), &usage, SHADER_UNIFORM_INT);
	SetShaderValue(shader, GetShaderLocation(shader, "useTexMRA"), &usage, SHADER_UNIFORM_INT);
	SetShaderValue(shader, GetShaderLocation(shader, "useTexEmissive"), &usage, SHADER_UNIFORM_INT);
    
	int invertLoc = GetShaderLocation(shader, "invertAlbedo");
	
	SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
//---------------------------------------------------------------------------------------

	rlImGuiSetup(true);
	while (!WindowShouldClose())    // Detect window close button or ESC key
	{
		handleMouseButton();
		handleMouseWheel();

		updateCamera(0);

		// Update the shader with the camera view vector (points towards { 0.0f, 0.0f, 0.0f })
		float cameraPos[3] = {camera.position.x, camera.position.y, camera.position.z};
		SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);


		// Update light values on shader (actually, only enable/disable them)
		for (int i = 0; i < MAX_LIGHTS; i++) UpdateLight(shader, lights[i]);

        
		SetShaderValue(shader, GetShaderLocation(shader, "ambientColor"), &ambientColorNormalized, SHADER_UNIFORM_VEC3);
		SetShaderValue(shader, GetShaderLocation(shader, "ambient"), &ambientIntensity, SHADER_UNIFORM_FLOAT);


		usage = 1;
		SetShaderValue(shader, GetShaderLocation(shader, "useTexAlbedo"), &usage, SHADER_UNIFORM_INT);
		SetShaderValue(shader, GetShaderLocation(shader, "useTexNormal"), &usage, SHADER_UNIFORM_INT);
		SetShaderValue(shader, GetShaderLocation(shader, "useTexMRA"), &usage, SHADER_UNIFORM_INT);
		SetShaderValue(shader, GetShaderLocation(shader, "useTexEmissive"), &usage, SHADER_UNIFORM_INT);
//----------------------------------------------------------------------------------

		// Draw
		//----------------------------------------------------------------------------------
		BeginDrawing();
		{
			ClearBackground(BLACK);
            
			BeginMode3D(camera);
			{
				int invert = 0; // Set to 1 to invert, 0 to disable
				SetShaderValue(shader, invertLoc, &invert, SHADER_UNIFORM_INT);

				// Set floor model texture tiling and emissive color parameters on shader
				SetShaderValue(shader, textureTilingLoc, &floorTextureTiling, SHADER_UNIFORM_VEC2);
				Vector4 floorEmissiveColor = ColorNormalize(floor.materials[0].maps[MATERIAL_MAP_EMISSION].color);
				SetShaderValue(shader, emissiveColorLoc, &floorEmissiveColor, SHADER_UNIFORM_VEC4);
                
				DrawModel(floor, (Vector3){ 0.0f, 0.0f, 0.0f }, 50.0f, WHITE);   // Draw floor model

				// Set old car model texture tiling, emissive color and emissive intensity parameters on shader
				SetShaderValue(shader, textureTilingLoc, &carTextureTiling, SHADER_UNIFORM_VEC2);
				Vector4 carEmissiveColor = ColorNormalize(car.materials[0].maps[MATERIAL_MAP_EMISSION].color);
				SetShaderValue(shader, emissiveColorLoc, &carEmissiveColor, SHADER_UNIFORM_VEC4);
				float emissiveIntensity = 0.01f;
				SetShaderValue(shader, emissiveIntensityLoc, &emissiveIntensity, SHADER_UNIFORM_FLOAT);
				DrawModel(car, (Vector3){ 0.0f, 0.0f, 0.0f }, 0.25f, WHITE);   // Draw car model

				
				invert = 1; // Set to 1 to invert, 0 to disable
				SetShaderValue(shader, invertLoc, &invert, SHADER_UNIFORM_INT);
				DrawModel(cube, (Vector3){ 0.0f, 0.5f, 3.5f }, 1.0f, WHITE);

// BeginShaderMode(shader);
				DrawCube({ 2.0f, 0.5f, 3.5f }, 1.0f, 1.0f, 1.0f, RED);
				DrawCubeWires({ 2.0f, 0.5f, 3.5f }, 1.0f, 1.0f, 1.0f, GREEN);
// EndShaderMode();
				
// Draw spheres to show the lights positions
				for (int i = 0; i < MAX_LIGHTS; i++) {
					unsigned char r = lights[i].color[0]*255;
					unsigned char g = lights[i].color[1]*255;
					unsigned char b = lights[i].color[2]*255;
					unsigned char a = lights[i].color[3]*255;
					
					Color lightColor = (Color){ r, g , b, a };
                    
					if (lights[i].enabled) DrawSphereEx(lights[i].position, 0.2f, 8, 8, lightColor);
					else DrawSphereWires(lights[i].position, 0.2f, 8, 8, ColorAlpha(lightColor, 0.3f));
				}
             
			}
			EndMode3D();
            
			DrawFPS(10, 10);
			
			imguiMenus();
		}
		EndDrawing();
		//----------------------------------------------------------------------------------
	}
	rlImGuiShutdown();
    // De-Initialization
    //--------------------------------------------------------------------------------------
    // Unbind (disconnect) shader from car.material[0] 
    // to avoid UnloadMaterial() trying to unload it automatically
    car.materials[0].shader = (Shader){ 0 };
    UnloadMaterial(car.materials[0]);
    car.materials[0].maps = NULL;
    UnloadModel(car);
    
    floor.materials[0].shader = (Shader){ 0 };
    UnloadMaterial(floor.materials[0]);
    floor.materials[0].maps = NULL;
    UnloadModel(floor);
    
    UnloadShader(shader);       // Unload Shader
    
    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

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

	rlImGuiBegin();
	
	ImGui::Begin("Lighting");
	ImGui::SeparatorText("Lights");
	ImGui::DragFloat("ambientIntensity", (float*)&ambientIntensity, 0.001f, 0.0f, 1.0f);
	Vector4 vAmbient = { ambientColorNormalized.x, ambientColorNormalized.y, ambientColorNormalized.z, 1.0 };
	if (ImGui::ColorEdit4("ambientColorNormalized", &vAmbient.x, ImGuiColorEditFlags_Float)) {
		ambientColorNormalized.x = vAmbient.x;
		ambientColorNormalized.y = vAmbient.y;
		ambientColorNormalized.z = vAmbient.z;
	}
	ImGui::Text("lightCount:"); ImGui::SameLine(180);
	ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%i", lightCount);
	for (int i=0; i<5; i++) {
		const char* posStr = TextFormat("position %i", i);
		const char* colorStr = TextFormat("color %i", i);
		const char* type = (lights[i].type == LIGHT_POINT ? "Point" : "Directional");
		const char* enabledStr = TextFormat("Light %i %s", i, type);
		const char* intensityStr = TextFormat("intensity %i", i);
		bool enabled = lights[i].enabled;
		if (ImGui::Checkbox(enabledStr, &enabled)) {
			lights[i].enabled = enabled ? 1 : 0;
		}
		ImGui::DragFloat(intensityStr, &lights[i].intensity, 0.1f, 0.0f, 100.0f);
		ImGui::DragFloat3(posStr, (float *)&lights[i].position, 0.5f, -20.0f, 20.0f);
		ImGui::ColorEdit4(colorStr, &lights[i].color[0], ImGuiColorEditFlags_Float);
		if (lights[i].type == LIGHT_DIRECTIONAL) {
			const char* targetStr = TextFormat("target %i", i);
			ImGui::DragFloat3(targetStr, (float *)&lights[i].target, 0.5f, -100.0f, 100.0f);
		}
		ImGui::Spacing();
	}
	ImGui::End();


	rlImGuiEnd();
}



// Create light with provided data
// NOTE: It updated the global lightCount and it's limited to MAX_LIGHTS
static Light CreateLight(int type, Vector3 position, Vector3 target, Color color, float intensity, Shader shader)
{
    Light light = { 0 };

    if (lightCount < MAX_LIGHTS)
    {
        light.enabled = 1;
        light.type = type;
        light.position = position;
        light.target = target;
        light.color[0] = (float)color.r/255.0f;
        light.color[1] = (float)color.g/255.0f;
        light.color[2] = (float)color.b/255.0f;
        light.color[3] = (float)color.a/255.0f;
        light.intensity = intensity;
        
        // NOTE: Shader parameters names for lights must match the requested ones
        light.enabledLoc = GetShaderLocation(shader, TextFormat("lights[%i].enabled", lightCount));
        light.typeLoc = GetShaderLocation(shader, TextFormat("lights[%i].type", lightCount));
        light.positionLoc = GetShaderLocation(shader, TextFormat("lights[%i].position", lightCount));
        light.targetLoc = GetShaderLocation(shader, TextFormat("lights[%i].target", lightCount));
        light.colorLoc = GetShaderLocation(shader, TextFormat("lights[%i].color", lightCount));
        light.intensityLoc = GetShaderLocation(shader, TextFormat("lights[%i].intensity", lightCount));
        
        UpdateLight(shader, light);

        lightCount++;
    }

    return light;
}

// Send light properties to shader
// NOTE: Light shader locations should be available
static void UpdateLight(Shader shader, Light light)
{
    SetShaderValue(shader, light.enabledLoc, &light.enabled, SHADER_UNIFORM_INT);
    SetShaderValue(shader, light.typeLoc, &light.type, SHADER_UNIFORM_INT);
    
    // Send to shader light position values
    float position[3] = { light.position.x, light.position.y, light.position.z };
    SetShaderValue(shader, light.positionLoc, position, SHADER_UNIFORM_VEC3);

    // Send to shader light target position values
    float target[3] = { light.target.x, light.target.y, light.target.z };
    SetShaderValue(shader, light.targetLoc, target, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, light.colorLoc, light.color, SHADER_UNIFORM_VEC4);
    SetShaderValue(shader, light.intensityLoc, &light.intensity, SHADER_UNIFORM_FLOAT);
}
