#version 330

// An introduction to Shader Art Coding
// https://www.youtube.com/watch?v=f4s1h2YETNY

in vec2 fragTexCoord; // From vertex shader

uniform vec2 resolution;
uniform float time;
uniform vec3 mycolor;

out vec4 FragColor;

void main()
{
    // Shadertoy-style UV mapping
    vec2 uv = fragTexCoord * 2 - 1.0;
	uv.x *= resolution.x / resolution.y;

	uv = fract(uv * 2.0) - 0.5;
	
	float d = length(uv);
	d = sin(d*8. + time)/8.;
	d = abs(d);
	// d = smoothstep(0.0, 0.1, d);

	d = 0.02/d;

	// vec3 col = mycolor;
	vec3 col = vec3(1.0, 2.0, 3.0);
	col *= d;
	
	// float t = 0.5f + sin(time)/2.0f;

	// FragColor = vec4(uv.x+mycolor.r, uv.y+mycolor.g, mycolor.b, 1.0); // Red color for testing
	// FragColor = vec4(d, d, d, 1.0); // Red color for testing
	FragColor = vec4(col, 1.0); // Red color for testing
}
