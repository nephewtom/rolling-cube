#version 330

// Input vertex attributes (from vertex shader)
in vec3 fragPosition;

// Input uniform values
uniform samplerCube environmentMap;
uniform bool vflipped;
uniform bool doGamma;
uniform float time;
uniform vec2 direction;

// Output fragment color
out vec4 finalColor;


mat3 rotateY(float angle) {
    float s = sin(angle);
    float c = cos(angle);
    return mat3(
        c,  0.0, s,
        0.0, 1.0, 0.0,
		-s,  0.0, c
		);
}

void main()
{
	float angle = 0.0;
	// temporal fix to change sky direction, but... introduces 2 abrupt rotation points...
	if (direction.x == -1.0 || direction.y == 1.0) {
		angle = time * 0.003;
	}
	else {
		angle = -time * 0.003;
	}
		
	vec3 rotatedPos = rotateY(angle) * fragPosition;
	
	// Fetch color from texture map
	vec3 color = vec3(0.0);

	// if (vflipped) color = texture(environmentMap, vec3(fragPosition.x, -fragPosition.y, fragPosition.z)).rgb;
	// else color = texture(environmentMap, fragPosition).rgb;

	if (vflipped)
		color = texture(environmentMap, vec3(rotatedPos.x, -rotatedPos.y, rotatedPos.z)).rgb;
    else
		color = texture(environmentMap, rotatedPos).rgb;
	
    if (doGamma)// Apply gamma correction
    {
        color = color/(color + vec3(1.0));
        color = pow(color, vec3(1.0/2.2));
    }

    // Calculate final fragment color
    finalColor = vec4(color, 1.0);
}
