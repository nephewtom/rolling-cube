#ifndef GROUND_H
#define GROUND_H

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#include <assert.h> // gets rid of Emacs Flycheck complain in RL_CALLOC

#define GLSL_VERSION 330

const int X_CELLS = 199;
const int Z_CELLS = 199;
const int BEGIN_CELL_POS = 0;
const int END_CELL_POS = 199;

class Cell {
public:
	bool isEmpty;
	int entityId;
	Color color;
};
struct Ground {
	Mesh plane;
	Model model;
	Material material;
	Matrix *transforms;
	Cell cells[X_CELLS][Z_CELLS];

	int instancingLoc;
	
	void init(Shader& shader, Texture& texture) {
		instancingLoc = GetShaderLocation(shader, "instancing");
		
		plane = GenMeshPlane(1.0f,1.0f,1,1);
		model = LoadModelFromMesh(plane);
		model.materials[0].shader = shader;

		Texture logoGround = LoadTexture("assets/logo-ground.png");
		model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = logoGround;

		material = LoadMaterialDefault();
		material.shader = shader;
		material.maps[MATERIAL_MAP_DIFFUSE].texture = texture;

		transforms = (Matrix *)RL_CALLOC(X_CELLS*Z_CELLS, sizeof(Matrix));
		
		int i=0;
		for (int ix = 0, xCoord = BEGIN_CELL_POS; ix < X_CELLS; ix++, xCoord++) {
			for (int iz = 0, zCoord = BEGIN_CELL_POS; iz < Z_CELLS; iz++, zCoord++) {
				transforms[i] = MatrixTranslate(xCoord + 0.5f, 0.0f, zCoord + 0.5f);
			
				cells[ix][iz].isEmpty = true;
				cells[ix][iz].entityId = -1;
				cells[ix][iz].color = getRandomColor();
				i++;
			}
		}	
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
		float fx = BEGIN_CELL_POS;
		for (int x = 0; x < X_CELLS; x++, fx++) {
			float fz = BEGIN_CELL_POS;
			for (int z = 0; z < Z_CELLS; z++, fz++) {
				DrawTriangle3D({fx, 0.0f, fz}, {fx, 0.0f, fz+1}, {fx+1, 0.0f, fz+1}, cells[x][z].color);
				DrawTriangle3D({fx+1, 0.0f, fz+1}, {fx+1, 0.0f, fz}, {fx, 0.0f, fz}, cells[x][z].color);
			}
		}
	}
	
	void drawInstances(Shader& shader) {
		int instancing = 1;
		SetShaderValue(shader, instancingLoc, &instancing, SHADER_UNIFORM_INT);
		BeginShaderMode(shader);
		DrawMeshInstanced(plane, material, transforms, X_CELLS*Z_CELLS);
		EndShaderMode();
	}
	
};


#endif
