#include "cubemap.c"

#define GLSL_VERSION 330

struct Skybox {
	Mesh cube;
	Model model;
	Shader cubemap;
};

void loadSkybox(Skybox& skybox) {
	skybox.cube = GenMeshCube(1.0f, 1.0f, 1.0f);
	skybox.model = LoadModelFromMesh(skybox.cube);
	skybox.model.materials[0].shader = LoadShader(TextFormat("shaders/skybox.vs", GLSL_VERSION),
												  TextFormat("shaders/skybox.fs", GLSL_VERSION));
	
	int envMapLoc = GetShaderLocation(skybox.model.materials[0].shader, "environmentMap");
	int mmc[1] = {MATERIAL_MAP_CUBEMAP};
	int valueOne[1] = { 1 }; 
	int valueZero[1] = { 0 }; 
	SetShaderValue(skybox.model.materials[0].shader, envMapLoc, mmc, SHADER_UNIFORM_INT);
	int doGammaLoc = GetShaderLocation(skybox.model.materials[0].shader, "doGamma");
	SetShaderValue(skybox.model.materials[0].shader, doGammaLoc, valueOne, SHADER_UNIFORM_INT);
	int vflippedLoc = GetShaderLocation(skybox.model.materials[0].shader, "vflipped");
	SetShaderValue(skybox.model.materials[0].shader, vflippedLoc, valueOne, SHADER_UNIFORM_INT);
	
	skybox.cubemap = LoadShader(TextFormat("shaders/cubemap.vs", GLSL_VERSION),
								TextFormat("shaders/cubemap.fs", GLSL_VERSION));
	int equiMapLoc = GetShaderLocation(skybox.cubemap, "equirectangularMap");
	SetShaderValue(skybox.cubemap, equiMapLoc, valueZero, SHADER_UNIFORM_INT);

	Texture2D panorama = LoadTexture("assets/hdr/krita_oilpaint-sunflowers_puresky_4k.hdr");

	skybox.model.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture = 
		GenTextureCubemap(skybox.cubemap, panorama, 1024, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
	UnloadTexture(panorama);
}

void updateSkybox(Skybox& skybox) {
	FilePathList droppedFiles = LoadDroppedFiles();
	if (droppedFiles.count != 1) return;
	if (!IsFileExtension(droppedFiles.paths[0], ".hdr")) return;
	UnloadTexture(skybox.model.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture);
	Texture2D panorama = LoadTexture(droppedFiles.paths[0]);
	skybox.model.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture = 
		GenTextureCubemap(skybox.cubemap, panorama, 1024, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
	UnloadTexture(panorama);
	UnloadDroppedFiles(droppedFiles);
}
