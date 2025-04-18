#ifndef LIGHTS_H
#define LIGHTS_H

#include "raylib.h"

#define MAX_LIGHTS  10         // Max dynamic lights supported by shader

typedef struct {   
    int type;
    bool enabled;
    Vector3 position;
    Vector3 target;
    Color color;
    bool hidden;
	
	float attenuation;
	
  
    // Shader locations
    int enabledLoc;
    int typeLoc;
    int positionLoc;
    int targetLoc;
    int colorLoc;
    int attenuationLoc;
} Light;

// Light type
typedef enum {
	INACTIVE = 0,
    LIGHT_DIRECTIONAL = 1,
    LIGHT_POINT
} LightType;

#endif
