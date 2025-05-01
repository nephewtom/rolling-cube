#ifndef SHADER_LIGHTS_CPP
#define SHADER_LIGHTS_CPP

#include "raylib.h"
#include "log.h"
#include "lights.h"

struct ShaderLightsData {
	Shader shader;
	Vector4 ambient = { 0.1f, 0.1f, 0.1f, 1.0f };
	int ambientLoc;
	
	Texture logo;
	
	Light lights[MAX_LIGHTS];
	int lightsCount = 0;    // Current amount of created lights
	
	void loadLogo();
	void loadShader();

	void createLights();
	Light createLight(int type, Vector3 position, Vector3 target, Color color);
	void updateLights();
	void updateLightValues(Light light);
	void drawLights();
};

void ShaderLightsData::loadLogo() {
	logo = LoadTexture("assets/logo.png");
}

void ShaderLightsData::loadShader() {
	shader = LoadShader("shaders/lighting_with_instancing.vs",
						"shaders/lighting_with_instancing.fs");
	shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");

	ambientLoc = GetShaderLocation(shader, "ambient");
	SetShaderValue(shader, ambientLoc, &ambient, SHADER_UNIFORM_VEC4);
}

void ShaderLightsData::createLights() {
	lights[0] = createLight(LIGHT_POINT, { 2.5f, 0.5f, 2.5f }, { 0.0f, 0.0f, 0.0f }, YELLOW);
	lights[1] = createLight(LIGHT_POINT, { -2.5f, 0.5f, 2.5f }, { 0.0f, 0.0f, 0.0f }, RED);
	lights[2] = createLight(LIGHT_POINT, { 2.5f, 0.5f, -2.5f }, { 0.0f, 0.0f, 0.0f }, BLUE);
	lights[3] = createLight(LIGHT_POINT, { -2.5f, 0.5f, -2.5f }, { 0.0f, 0.0f, 0.0f }, GREEN);
	lights[4] = createLight(LIGHT_DIRECTIONAL,
							{ 45.0f, 2.0f, 55.0f }, { 50.0f, 0.0f, 50.0f }, { 139,146,146,255 });
	lights[5] = createLight(LIGHT_DIRECTIONAL,
							{ 55.0f, 2.0f, 47.0f }, { 50.0f, 0.0f, 50.0f }, { 144, 147, 98, 255 });
	
	LOGD("sizeOf lights: %d", sizeof(lights)/sizeof(Light));
	for (int i=0; i<MAX_LIGHTS; i++){
		const char* type = lights[i].type == INACTIVE ? "INACTIVE" :
			lights[i].type == LIGHT_DIRECTIONAL ? "DIRECTIONAL" : "POINT";
		LOGD("lights[%i].type=%s", i, type);
		Vector3 p = lights[i].position;
		const char* position = TextFormat("x: %f, y:%f, z: %f", p.x, p.y, p.z);
		LOGD("lights[%i].position=%s", i, position);
		lights[i].enabled = false;
	}
	lights[4].enabled = true;
	lights[5].enabled = true;
}

Light ShaderLightsData::createLight(int type, Vector3 position, Vector3 target, Color color)
{
	Light light = { 0 };

	if (lightsCount < MAX_LIGHTS)
	{
		light.enabled = true;
		light.type = type;
		light.position = position;
		light.target = target;
		light.color = color;
		light.hidden = true;

		// NOTE: Lighting shader naming must be the provided ones
		light.enabledLoc = GetShaderLocation(shader, TextFormat("lights[%i].enabled", lightsCount));
		light.typeLoc = GetShaderLocation(shader, TextFormat("lights[%i].type", lightsCount));
		light.positionLoc = GetShaderLocation(shader, TextFormat("lights[%i].position", lightsCount));
		light.targetLoc = GetShaderLocation(shader, TextFormat("lights[%i].target", lightsCount));
		light.colorLoc = GetShaderLocation(shader, TextFormat("lights[%i].color", lightsCount));

		updateLightValues(light);
        
		lightsCount++;
		LOGD("Light created: number=%d | enabled=%s", lightsCount, light.enabled ? "true" : "false");
	} else {
		LOGD("MAX_LIGHTS exceeded!");
	}

	return light;
}

void ShaderLightsData::updateLights() {
	for (int i = 0; i < MAX_LIGHTS; i++) {
		updateLightValues(lights[i]);
	}
}

void ShaderLightsData::updateLightValues(Light light) {
	// Send to shader light enabled state and type
	SetShaderValue(shader, light.enabledLoc, &light.enabled, SHADER_UNIFORM_INT);
	SetShaderValue(shader, light.typeLoc, &light.type, SHADER_UNIFORM_INT);

	// Send to shader light position values
	float position[3] = { light.position.x, light.position.y, light.position.z };
	SetShaderValue(shader, light.positionLoc, position, SHADER_UNIFORM_VEC3);

	// Send to shader light target position values
	float target[3] = { light.target.x, light.target.y, light.target.z };
	SetShaderValue(shader, light.targetLoc, target, SHADER_UNIFORM_VEC3);

	// Send to shader light color values
	float color[4] = { (float)light.color.r/(float)255, (float)light.color.g/(float)255, 
		(float)light.color.b/(float)255, (float)light.color.a/(float)255 };
	SetShaderValue(shader, light.colorLoc, color, SHADER_UNIFORM_VEC4);
}

void ShaderLightsData::drawLights() {
	// Draw spheres to show where the lights are
	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		if (lights[i].type == INACTIVE) continue;
		if (lights[i].hidden == true) continue;

		if (lights[i].enabled)  {
			DrawSphereEx(lights[i].position, 0.2f, 8, 8, lights[i].color);
		}
		else { 
			DrawSphereWires(lights[i].position, 0.2f, 8, 8, ColorAlpha(lights[i].color, 0.3f));
		}
			
		if (lights[i].type == LIGHT_DIRECTIONAL) {
			DrawLine3D(lights[i].position, lights[i].target, lights[i].color);
			DrawSphereWires(lights[i].target, 0.021f, 4, 4, lights[i].color);
		}
	}
}


#endif
