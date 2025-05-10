#include "./common.h"
#include "r3d.h"
/* === Resources === */

static Model		plane = { 0 };
static Model		sphere = { 0 };
static Model		cube = { 0 };
static Model        cube2 = {0};
static Camera3D		camera = { 0 };


/* === Examples === */

const char* Init(void)
{
    R3D_Init(GetScreenWidth(), GetScreenHeight(), 0);
    SetTargetFPS(60);

    plane = LoadModelFromMesh(GenMeshPlane(1000, 1000, 1, 1));
    plane.materials[0].maps[MATERIAL_MAP_OCCLUSION].value = 1;
    plane.materials[0].maps[MATERIAL_MAP_ROUGHNESS].value = 1;
    plane.materials[0].maps[MATERIAL_MAP_METALNESS].value = 0;
    plane.materials[0].maps[MATERIAL_MAP_EMISSION].value = 0;

    sphere = LoadModelFromMesh(GenMeshSphere(0.5f, 64, 64));
    sphere.materials[0].maps[MATERIAL_MAP_OCCLUSION].value = 1;
    sphere.materials[0].maps[MATERIAL_MAP_ROUGHNESS].value = 0.25;
    sphere.materials[0].maps[MATERIAL_MAP_METALNESS].value = 0.75;
    sphere.materials[0].maps[MATERIAL_MAP_EMISSION].value = 0;
    sphere.materials[0].maps[MATERIAL_MAP_EMISSION].color = BLACK;
	
    cube = LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));
    cube.materials[0].maps[MATERIAL_MAP_OCCLUSION].value = 1;
    cube.materials[0].maps[MATERIAL_MAP_ROUGHNESS].value = 1.0;
    cube.materials[0].maps[MATERIAL_MAP_METALNESS].value = 1.0;

	Texture tColor = LoadTexture("./cube_color.jpg");
	R3D_SetMaterialAlbedo(&cube.materials[0], &tColor, BLUE);
	Texture tAo = LoadTexture("./cube_ao.jpg");
	R3D_SetMaterialOcclusion(&cube.materials[0], &tAo, 1.0f);
	Texture tRo = LoadTexture("./cube_roughness.jpg");
	R3D_SetMaterialRoughness(&cube.materials[0], &tRo, 1.0f);

    cube2 = LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));
    cube2.materials[0].maps[MATERIAL_MAP_OCCLUSION].value = 1;
    cube2.materials[0].maps[MATERIAL_MAP_ROUGHNESS].value = 0.25;
    cube2.materials[0].maps[MATERIAL_MAP_METALNESS].value = 0.75;
	Texture t2Color = LoadTexture("./logo.png");
	R3D_SetMaterialAlbedo(&cube2.materials[0], &t2Color, WHITE);

	
	camera = (Camera3D) {
		.position = (Vector3) { 5, 3, 5 },
		.target = (Vector3) { 0, 0, 0 },
		.up = (Vector3) { 0, 1, 0 },
		.fovy = 60,
	};

	R3D_Light light = R3D_CreateLight(R3D_LIGHT_DIR);
	{

		R3D_SetLightDirection(light, (Vector3) { 0, -1, -1 });
		R3D_SetShadowUpdateMode(light, R3D_SHADOW_UPDATE_MANUAL);
		R3D_SetShadowBias(light, 0.005f);
		
		/* R3D_SetLightPosition(light, (Vector3) { 0, 10, 5 }); */
		/* R3D_SetLightTarget(light, (Vector3) { 0 }); */

		R3D_EnableShadow(light, 4096);
		R3D_SetLightActive(light, true);
	}

	R3D_Light light2 = R3D_CreateLight(R3D_LIGHT_SPOT);
	{
		R3D_SetLightPosition(light2, (Vector3) { 5, 10, 0 });
		R3D_LightLookAt(light2, (Vector3) { 5, 10, 0 }, (Vector3) { 0 });

		R3D_EnableShadow(light2, 4096);
		R3D_SetLightActive(light2, true);			
	}

	
	return "[r3d] - basic example";
}

void Update(float delta)
{
    UpdateCamera(&camera, CAMERA_FIRST_PERSON);
}

void Draw(void)
{

	R3D_Begin(camera);
	R3D_DrawModel(cube2, (Vector3){2.0, 0.0f, -2.0f}, 1.0f);
	R3D_DrawModel(plane, (Vector3) { 0, -0.5f, 0 }, 1.0f);
	R3D_DrawModel(sphere, (Vector3) { 0.0 }, 1.0f);
	R3D_DrawModel(cube, (Vector3){-2.0, 0.0f, 2.0f}, 1.0f);
	R3D_End();
		
	
	Vector3 target = {0, 0, 0};
	Vector3 lPos = { 0, 10, 5 };

	BeginMode3D(camera);
	DrawSphereWires(lPos, 0.2f, 10, 10, YELLOW);
	DrawSphereWires(target, 0.1f, 5, 5, YELLOW);
	DrawLine3D(lPos, target, YELLOW);
	EndMode3D();
}

void Close(void)
{
    UnloadModel(plane);
    UnloadModel(sphere);
    R3D_Close();
}
