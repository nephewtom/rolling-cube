#version 330

// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

// Input uniform values
in mat4 instanceTransform;
uniform int instance;  // Choice variable

uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matNormal;

// Output vertex attributes (to fragment shader)
out vec3 fragPosition;
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragNormal;

// NOTE: Add your custom variables here

void main()
{
    // Send vertex attributes to fragment shader
	
    fragPosition = (instance == 0) ? vec3(matModel*vec4(vertexPosition, 1.0)) : vec3(instanceTransform*vec4(vertexPosition, 1.0));
    fragTexCoord = vertexTexCoord;
    fragColor = (instance == 0) ? vertexColor : vec4(1.0);;
    fragNormal = normalize(vec3(matNormal*vec4(vertexNormal, 1.0)));

    // Calculate final vertex position
    gl_Position = (instance == 0 ) ? mvp*vec4(vertexPosition, 1.0) : mvp*instanceTransform*vec4(vertexPosition, 1.0);
}
