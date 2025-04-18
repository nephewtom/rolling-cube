#include "cubemap.c"

#define GLSL_VERSION 330

struct Skybox {
	Mesh cube;
	Model model;
	Shader cubemap;
	
	void load();
	void update();
	void draw(Vector3 direction);
};

void Skybox::load() {
	cube = GenMeshCube(1.0f, 1.0f, 1.0f);
	model = LoadModelFromMesh(cube);
	model.materials[0].shader = LoadShader(TextFormat("shaders/skybox.vs", GLSL_VERSION),
										   TextFormat("shaders/skybox.fs", GLSL_VERSION));
	
	int envMapLoc = GetShaderLocation(model.materials[0].shader, "environmentMap");
	int mmc[1] = {MATERIAL_MAP_CUBEMAP};
	int valueOne[1] = { 1 }; 
	int valueZero[1] = { 0 }; 
	SetShaderValue(model.materials[0].shader, envMapLoc, mmc, SHADER_UNIFORM_INT);
	int doGammaLoc = GetShaderLocation(model.materials[0].shader, "doGamma");
	SetShaderValue(model.materials[0].shader, doGammaLoc, valueOne, SHADER_UNIFORM_INT);
	int vflippedLoc = GetShaderLocation(model.materials[0].shader, "vflipped");
	SetShaderValue(model.materials[0].shader, vflippedLoc, valueOne, SHADER_UNIFORM_INT);
	
	cubemap = LoadShader(TextFormat("shaders/cubemap.vs", GLSL_VERSION),
						 TextFormat("shaders/cubemap.fs", GLSL_VERSION));
	int equiMapLoc = GetShaderLocation(cubemap, "equirectangularMap");
	SetShaderValue(cubemap, equiMapLoc, valueZero, SHADER_UNIFORM_INT);

	Texture2D panorama = LoadTexture("assets/hdr/krita_oilpaint-sunflowers_puresky_4k.hdr");

	model.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture = 
		GenTextureCubemap(cubemap, panorama, 1024, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
	UnloadTexture(panorama);
}

void Skybox::update() {
	FilePathList droppedFiles = LoadDroppedFiles();
	if (droppedFiles.count != 1) return;
	if (!IsFileExtension(droppedFiles.paths[0], ".hdr")) return;
	UnloadTexture(model.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture);
	Texture2D panorama = LoadTexture(droppedFiles.paths[0]);
	model.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture = 
		GenTextureCubemap(cubemap, panorama, 1024, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
	
	UnloadTexture(panorama);
	UnloadDroppedFiles(droppedFiles);
}

void Skybox::draw(Vector3 cubeDirection) {
	int timeLoc = GetShaderLocation(model.materials[0].shader, "time");
	float elapsedTime = GetTime();
	SetShaderValue(model.materials[0].shader, timeLoc, &elapsedTime, SHADER_UNIFORM_FLOAT);
				
	// temporal fix to change sky direction, but... introduces 2 abrupt rotation points...
	int dirLoc = GetShaderLocation(model.materials[0].shader, "direction");
	float cubeXZdirection[2] = { cubeDirection.x, cubeDirection.z };
	SetShaderValue(model.materials[0].shader, dirLoc, cubeXZdirection, SHADER_UNIFORM_VEC2);
				
	rlDisableBackfaceCulling();
	rlDisableDepthMask();
	DrawModel(model, Vector3Zero(), 1.0f, WHITE);
	rlEnableBackfaceCulling();
	rlEnableDepthMask();
}
