#ifndef ENTITY_H
#define ENTITY_H

#include <stdio.h>
#include <stdlib.h>
#include "raylib.h"

struct PositionIndex {
	int x;
	int z;
	PositionIndex operator+(const PositionIndex& other) const {
		return {x + other.x, z + other.z};
	}
	
	PositionIndex operator*(int scalar) const {
		return {x * scalar, z * scalar};
	}
	
	bool operator==(const PositionIndex& other) const {
		if (x == other.x && z == other.z) return true;
		else return false;
	}

};
typedef PositionIndex PIndex;

struct EntityQuery {
	PIndex pIndex;
	int id;
};

enum BoxType {
	NONE, WALL, OBSTACLE, PUSHBOX, PULLBOX, PUSHPULLBOX, OTHER
};

inline const char* getBoxType(BoxType type) {
	const char* s = 
		type == NONE ? "NONE" :
		type == WALL ? "WALL" :
		type == OBSTACLE ? "OBSTACLE" :
		type == PUSHBOX ? "PUSHBOX" :
		type == PULLBOX ? "PULLBOX" :
		type == PUSHPULLBOX ? "PUSHPULLBOX" :
		"OTHER";
	return s;
}

class Entity {
public:
	PIndex pIndex;
	BoxType type;
	bool hidden;
};

class EntityModels {
public:
	Model wall;
	Model obstacle;
	Model pushBox;
	Model pullBox;
	Model pushPullBox;
	
	void init();
};

class EntityPool {
private:
	Entity *_entity;
	int _capacity;
	int _count;

public:
	int getCount() { return _count; }
	PIndex getPositionIndex(int id) { return _entity[id].pIndex; }
	Entity& getEntity(int id) { return _entity[id]; }
	
	// Initialize the structure with an initial capacity
	void init(int capacity) {
		_capacity = capacity;
		_count = 0;
		_entity = (Entity*)malloc(sizeof(Entity) * _capacity);
		if (!_entity) {
			printf("Failed to allocate memory for EntityPool\n");
			exit(1);
		}
	}

	// Resize the entityPool array when _capacity is exceeded
	void expand() {
		_capacity *= 2;  // Double the _capacity
		_entity = (Entity*) realloc(_entity, sizeof(Entity) * _capacity);
		if (!_entity) {
			printf("Failed to reallocate memory for EntityPool\n");
			exit(1);
		}
	}

	// Free the dynamically allocated memory for this
	void freeEntities() {
		free(_entity);
		_entity = NULL;
		_count = 0;
		_capacity = 0;
	}

	// Add an obstacle to the pool
	int add(PIndex pIndex, BoxType type) {
		if (_count == _capacity) {
			expand();
		}
		int id = _count;
		_entity[id].pIndex = pIndex;
		_entity[id].type = type;
		_entity[id].hidden = false;
		_count++;
		
		return id;
	}

	// Remove an obstacle from the pool
	// Input: id of the obstacle
	// Output: the PositionIndex of the swapped id, so ground.cell[ix][iz].obstacleId gets updated
	void remove(EntityQuery& eq) {
		if (eq.id < 0 || eq.id >= _count) 
			return;  // Invalid id
		
		// Fill the obstacle to be removed with the last one and decrease _count
		_entity[eq.id] = _entity[_count - 1];
		_count--;

		// Return swaped pIndex
		eq.pIndex = _entity[eq.id].pIndex;
	}
};

#endif
