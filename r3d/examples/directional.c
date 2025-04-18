#include "./common.h"

/* === Resources === */

static Model		plane = { 0 };
static Mesh		    sphere = { 0 };
static Material     material = { 0 };
static Camera3D		camera = { 0 };

static Matrix* transforms = NULL;


/* === Examples === */

const char* Init(void)
{
    R3D_Init(GetScreenWidth(), GetScreenHeight(), 0);
    SetTargetFPS(60);

    plane = LoadModelFromMesh(GenMeshPlane(1000, 1000, 1, 1));
    plane.materials[0].maps[MATERIAL_MAP_OCCLUSION].value = 1;
    plane.materials[0].maps[MATERIAL_MAP_ROUGHNESS].value = 1;
    plane.materials[0].maps[MATERIAL_MAP_METALNESS].value = 0;

    sphere = GenMeshSphere(0.35f, 16, 16);

    material = LoadMaterialDefault();
    material.maps[MATERIAL_MAP_OCCLUSION].value = 1;
    material.maps[MATERIAL_MAP_ROUGHNESS].value = 0.25;
    material.maps[MATERIAL_MAP_METALNESS].value = 0.75;

    camera = (Camera3D){
        .position = (Vector3) { 0, 2, 2 },
        .target = (Vector3) { 0, 0, 0 },
        .up = (Vector3) { 0, 1, 0 },
        .fovy = 60,
    };

    transforms = RL_MALLOC(100 * 100 * sizeof(Matrix));

    for (int x = -50; x < 50; x++) {
        for (int z = -50; z < 50; z++) {
            int index = (z + 50) * 100 + (x + 50);
            transforms[index] = MatrixTranslate(x * 2, 0, z * 2);
        }
    }

    R3D_Light light = R3D_CreateLight(R3D_LIGHT_DIR);
    {
        R3D_SetLightDirection(light, (Vector3) { 0, -1, -1 });
        R3D_SetShadowUpdateMode(light, R3D_SHADOW_UPDATE_MANUAL);
        R3D_SetShadowBias(light, 0.005f);
        R3D_EnableShadow(light, 4096);

        R3D_SetLightActive(light, true);
    }

    DisableCursor();

    return "[r3d] - directional example";
}

void Update(float delta)
{
    UpdateCamera(&camera, CAMERA_FREE);
}

void Draw(void)
{
    R3D_Begin(camera);
        R3D_DrawModel(plane, (Vector3) { 0, -0.5f, 0 }, 1.0f);
        R3D_DrawMeshInstanced(sphere, material, transforms, 100 * 100);
    R3D_End();

    DrawFPS(10, 10);
}

void Close(void)
{
    UnloadModel(plane);
    UnloadMesh(sphere);
    UnloadMaterial(material);
    R3D_Close();
}
