#include "./common.h"
#include "rlgl.h"
#include "raylib.h"
#include "stdio.h"

/* === Resources === */

static Model		sphere = { 0 };
static Model		laser = { 0 };
static R3D_Skybox	skybox = { 0 };
static Camera3D		camera = { 0 };

static Material materials[5 * 5] = { 0 };


/* === Examples === */
const char* Init(void)
{
    R3D_Init(GetScreenWidth(), GetScreenHeight(), 0);
    SetTargetFPS(60);

    R3D_SetBloomMode(R3D_BLOOM_ADDITIVE);

    float bi = R3D_GetBloomIntensity();
	printf("bi: %f\n", bi);
	R3D_SetBloomIntensity(5.0f);
	
	sphere = LoadModelFromMesh(GenMeshSphere(0.5f, 64, 64));
    laser = LoadModelFromMesh(GenMeshCylinder(0.1, 5, 16));
    laser.transform = MatrixMultiply(laser.transform, MatrixRotateX(PI/2.0f));
	
	UnloadMaterial(sphere.materials[0]);
	UnloadMaterial(laser.materials[0]);

    for (int y = 0; y < 5; y++) {
        for (int x = 0; x < 5; x++) {
            int i = y * 5 + x;
            materials[i] = LoadMaterialDefault();
            materials[i].maps[MATERIAL_MAP_EMISSION].value = 1.0f;
            materials[i].maps[MATERIAL_MAP_OCCLUSION].value = 1.0f;
            materials[i].maps[MATERIAL_MAP_ROUGHNESS].value = (float)x / 5;
            materials[i].maps[MATERIAL_MAP_METALNESS].value = (float)y / 5;
            materials[i].maps[MATERIAL_MAP_ALBEDO].color = ColorFromHSV(x / 5.0f * 330, 1.0f, 1.0f);
            materials[i].maps[MATERIAL_MAP_EMISSION].color = materials[i].maps[MATERIAL_MAP_ALBEDO].color;
            materials[i].maps[MATERIAL_MAP_EMISSION].texture = materials[i].maps[MATERIAL_MAP_ALBEDO].texture;
        }
    }

    skybox = R3D_LoadSkybox(RESOURCES_PATH "sky/skybox1.png", CUBEMAP_LAYOUT_AUTO_DETECT);
    R3D_EnableSkybox(skybox);

    camera = (Camera3D){
        .position = (Vector3) { 0, 0, 5 },
        .target = (Vector3) { 0, 0, 0 },
        .up = (Vector3) { 0, 1, 0 },
        .fovy = 60,
    };

    return "[r3d] - bloom example";
}


float time = 0.0f;
float value = 0.0f;
void Update(float delta) {
    time += delta;
    value = 1.0f * sinf(25.0f*time) + 3.0f;  // amplitude 3, offset 4
    /* value = 3.0f * sinf(100.0f*time) + 4.0f;  // amplitude 3, offset 4 */
	R3D_SetBloomIntensity(value);

	
	UpdateCamera(&camera, CAMERA_FREE);
}

void Draw(void)
{
    R3D_Begin(camera);
    for (int y = -2; y <= 1; y++) {
        for (int x = -1; x <= 2; x++) {
            sphere.materials[0] = materials[(y + 2) * 5 + (x + 2)];
            R3D_DrawModel(sphere, (Vector3) { x * 1.1f, y * 1.1f, 0.0f }, 1.0f);
        }
    }
    laser.materials[0] = materials[0];
	/* laser.transform = MatrixMultiply(laser.transform, MatrixRotateX(0.01)); */
	R3D_DrawModel(laser, (Vector3){-2 * 1.1f, -2 * 1.1f, 0.0f}, 1.0f);
	R3D_End();

	R3D_DrawBufferEmission(10, 10, 100, 100);
    R3D_DrawBufferBloom(120, 10, 100, 100);
}

void Close(void)
{
    UnloadModel(sphere);
    R3D_UnloadSkybox(skybox);
    R3D_Close();
}
