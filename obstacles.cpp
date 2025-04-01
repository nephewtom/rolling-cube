#include <stdio.h>
#include <stdlib.h>

struct IndexPos {
	int x;
	int z;
};

struct Obstacle {
	IndexPos indexPos;
	int id;
};

class ObstaclePool {
private:
	IndexPos *_indexPos;   // Dynamically allocated array of obstaclePool
	int _count;             // Number of obstaclePool currently in the pool
	int _capacity;          // Maximum number of obstaclePool before resizing

public:
    // Initialize the structure with an initial _capacity
	void init(int capacity) {
		_capacity = capacity;
		_count = 0;
		_indexPos = (IndexPos*)malloc(sizeof(IndexPos) * _capacity);
		if (!_indexPos) {
			printf("Failed to allocate memory for obstaclePool\n");
			exit(1);
		}
	}

    // Resize the obstaclePool array when _capacity is exceeded
	void expandObstacles() {
		_capacity *= 2;  // Double the _capacity
		_indexPos = (IndexPos*) realloc(_indexPos, sizeof(IndexPos) * _capacity);
		if (!_indexPos) {
			printf("Failed to reallocate memory for this\n");
			exit(1);
		}
	}

    // Free the dynamically allocated memory for this
	void freeObstacles() {
		free(_indexPos);
		_indexPos = NULL;
		_count = 0;
		_capacity = 0;
	}

    // Add an obstacle to the pool
	int addObstacle(IndexPos ip) {
		if (_count == _capacity) {
			expandObstacles();
		}
		int id = _count;
		_indexPos[id] = ip;
		_count++;
		
		return id;
	}

    // Remove an obstacle from the pool
	// it receives the obstacle to be removed and returns the updated obstacle (IndexPos & id),
	// so the corresponding ground.cell[][] updates its obstacleId
	void removeObstacle(Obstacle& o) {
		if (o.id < 0 || o.id >= _count) return;  // Invalid id

		// Fill the obstacle to be removed with the last one and decrease _count
		_indexPos[o.id] = _indexPos[_count - 1];
		_count--;
		
		o.indexPos = _indexPos[o.id];
	}

};

