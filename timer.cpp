#ifndef TIMER_CPP
#define TIMER_CPP
#include "utils.h"

class Timer {
public:
	Timer(const char* name) : _name(name) {}
	const char* _name;
	bool _enabled = false;
	float _lifeTime;
	float _initTime;

	void start(float secs = 0.0f) {
		if (_enabled) { return; } // TODO: implement restart() to start before it has expired
		if (secs == 0.0f) { 
			secs = _initTime;
		}
		_initTime = secs;

		// TRACELOGD("%s started with %f", _name, _initTime);
		_lifeTime = _initTime;
		_enabled = true;
	}

	bool isEnabled() {
		return _enabled;
	}

	void update(float delta) {
		if (_enabled && _lifeTime > 0) {
			_lifeTime -= delta;
		}
	}

	bool isDone() {
		if (_enabled) {
			bool isDone = _lifeTime <= 0;
			if (isDone) { 
				// TRACELOGD("%s expired!", _name); 
				_enabled = false;
			}
			return isDone;
		}
		return true;
	}
};
#endif
