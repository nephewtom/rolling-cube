#include <stdio.h>
#include <stdlib.h>

struct PositionIndex {
	int x;
	int z;
};

struct Entity {
	PositionIndex pIndex;
	int id;
};

class EntityPool {
private:
	PositionIndex *_pIndex;
	int _capacity;
	int _count;

public:
	int getCount() { return _count; }
	PositionIndex getPositionIndex(int i) { return _pIndex[i]; }
	
    // Initialize the structure with an initial capacity
	void init(int capacity) {
		_capacity = capacity;
		_count = 0;
		_pIndex = (PositionIndex*)malloc(sizeof(PositionIndex) * _capacity);
		if (!_pIndex) {
			printf("Failed to allocate memory for entityPool\n");
			exit(1);
		}
	}

    // Resize the entityPool array when _capacity is exceeded
	void expand() {
		_capacity *= 2;  // Double the _capacity
		_pIndex = (PositionIndex*) realloc(_pIndex, sizeof(PositionIndex) * _capacity);
		if (!_pIndex) {
			printf("Failed to reallocate memory for this\n");
			exit(1);
		}
	}

    // Free the dynamically allocated memory for this
	void freeEntities() {
		free(_pIndex);
		_pIndex = NULL;
		_count = 0;
		_capacity = 0;
	}

    // Add an obstacle to the pool
	int addEntity(PositionIndex ip) {
		if (_count == _capacity) {
			expand();
		}
		int id = _count;
		_pIndex[id] = ip;
		_count++;
		
		return id;
	}

    // Remove an obstacle from the pool
	// Input: id of the obstacle
	// Output: the PositionIndex of the swapped id, so ground.cell[ix][iz].obstacleId gets updated
	void removeEntity(Entity& e) {
		if (e.id < 0 || e.id >= _count) 
			return;  // Invalid id
		
		// Fill the obstacle to be removed with the last one and decrease _count
		_pIndex[e.id] = _pIndex[_count - 1];
		_count--;
		
		e.pIndex = _pIndex[e.id];
	}

};

