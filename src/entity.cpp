#include "entity.h"
#include "raylib.h"
#include "globals.cpp"

void EntityModels::init() {
	
	Mesh cube = GenMeshCube(1.0, 1.0, 1.0);
	wall = LoadModelFromMesh(cube);
	wall.materials[0].shader = sld.shader;
	wall.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = sld.logo;
	
	Mesh cone = GenMeshCone(0.5, 1.0, 8);
	obstacle = LoadModelFromMesh(cone);
	obstacle.materials[0].shader = sld.shader;
	obstacle.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = sld.logo;
	
	// LoadModel("assets/tom-cube.obj");
	
	Mesh sphere = GenMeshHemiSphere(0.5, 8, 8);
	pushBox = LoadModelFromMesh(sphere);
	pushBox.materials[0].shader = sld.shader;
	pushBox.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = ground.logoGround;
	
	pullBox = pushBox;
	pushPullBox = pullBox;
}
