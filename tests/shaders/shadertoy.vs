#version 330

layout(location = 0) in vec2 aPos;        // Position
layout(location = 1) in vec2 aTexCoord;   // Texture coordinates (for Shadertoy-like effect)

// Vertex attributes from Raylib
// in vec3 vertexPosition;
// in vec2 vertexTexCoord;

out vec2 fragTexCoord; // Pass to fragment shader

void main()
{
//     gl_Position = vec4(vertexPosition, 1.0);
//     fragTexCoord = vertexTexCoord;

    gl_Position = vec4(aPos, 0.0, 1.0); // Position
    fragTexCoord = aPos*0.5 + 0.5;           // Pass texture coordinates to fragment shader
}
