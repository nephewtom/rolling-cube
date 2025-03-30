#version 330

in vec2 fragTexCoord; // From vertex shader

uniform vec2 resolution;
uniform float time;

out vec4 FragColor;

void main()
{
    // Shadertoy-style UV mapping
    vec2 uv = fragTexCoord;

	float t = 0.5f + sin(time)/2.0f;
    // Apply some simple Shadertoy-style effect
    // FragColor = vec4(uv, 0.5 + 0.5 * sin(time), 1.0);
	FragColor = vec4(uv.x, 0, 0.0, 1.0); // Red color for testing
}
