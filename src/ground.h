#ifndef GROUND_H
#define GROUND_H

#include "log.h"
#include "raylib.h"
#include "rlgl.h"

#include <assert.h> // gets rid of Emacs Flycheck complain in RL_CALLOC
#include "entity.h"

struct Cell {
	bool isEmpty;
	int entityId;
	Color color;
};
struct Ground {
	
	Model model;
	Texture logoGround;
	
	Mesh plane;
	Material material;
	int instancingLoc;
	
	// static const int X_CELLS = 199;
	// static const int Z_CELLS = 199;
	Color* pixelMap; // map of pixels that will generate the game level map
	int width;    // width of the map
	int height;   // height of the map
	Cell **cells; // map of entities
	Matrix *transforms;

	
	void init(Shader& shader, Texture& texture, const char* filename) {

		plane = GenMeshPlane(1.0f,1.0f,1,1);
		model = LoadModelFromMesh(plane);
		model.materials[0].shader = shader;

		logoGround = LoadTexture("assets/logo-ground.png");
		model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = logoGround;
		
		material = LoadMaterialDefault();
		material.shader = shader;
		material.maps[MATERIAL_MAP_DIFFUSE].texture = texture;
		instancingLoc = GetShaderLocation(shader, "instancing");		

		loadGroundMap(filename);
		LOGD("Finished!");
	}

	void loadGroundMap(const char* filename) {
		Image mapImage = LoadImage(filename);
		Texture2D texMap = LoadTextureFromImage(mapImage);
		width = texMap.width;
		height = texMap.height;
		LOGD("map size: width=%i, height%i", width, height);
		pixelMap = LoadImageColors(mapImage);
		UnloadImage(mapImage);
		
		allocGround();
	}
	
	void allocGround() {
		transforms = (Matrix *)RL_CALLOC(width * height, sizeof(Matrix));
		cells = (Cell **)RL_CALLOC(width, sizeof(Cell*)); // Allocate row pointers
		for (int i = 0; i < width; i++) {
			cells[i] = (Cell *)RL_CALLOC(height, sizeof(Cell)); // Allocate each row
		}	
	}
		
	void clearGroundMap() {
		UnloadImageColors(pixelMap);
		freeGround();
	}
	
	void freeGround() {
		for (int i = 0; i < width; i++) {
			RL_FREE(cells[i]);
		}
		RL_FREE(cells);
		RL_FREE(transforms);
	}

	Color getRandomColor() {
		return (Color){
			(unsigned char) GetRandomValue(0, 255), // Red
			(unsigned char) GetRandomValue(0, 255), // Green
			(unsigned char) GetRandomValue(0, 255), // Blue
			255 // Alpha (fully opaque)
		};
	}
	
	void drawColored() {
		float fx = 0;
		for (int x = 0; x < width; x++, fx++) {
			float fz = 0;
			for (int z = 0; z < height; z++, fz++) {
				DrawTriangle3D({fx, 0.0f, fz}, {fx, 0.0f, fz+1}, {fx+1, 0.0f, fz+1}, cells[x][z].color);
				DrawTriangle3D({fx+1, 0.0f, fz+1}, {fx+1, 0.0f, fz}, {fx, 0.0f, fz}, cells[x][z].color);
			}
		}
	}
	
	void drawInstances(Shader& shader) {
		int instancing = 1;
		SetShaderValue(shader, instancingLoc, &instancing, SHADER_UNIFORM_INT);
		BeginShaderMode(shader);
		DrawMeshInstanced(plane, material, transforms, width*height);
		EndShaderMode();
	}

	bool isEmptyCell(PIndex pIndex) {
		return cells[pIndex.x][pIndex.z].isEmpty;
	}
	
	int getEntityId(PIndex pIndex) {
		return cells[pIndex.x][pIndex.z].entityId;
	}
	void markEmptyCell(PIndex pIndex) {
		cells[pIndex.x][pIndex.z].isEmpty = true;
		cells[pIndex.x][pIndex.z].entityId = -1;
	}
	void markEntityInCell(PIndex pIndex, int id) {
		cells[pIndex.x][pIndex.z].isEmpty = false;
		cells[pIndex.x][pIndex.z].entityId = id;
	}
};

#endif
