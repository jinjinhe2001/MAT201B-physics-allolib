#version 410
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;

out vec3 fragPos;
out vec3 fragNorm;
out vec2 fragTex;

uniform mat4 model;
uniform mat4 al_ModelViewMatrix;
uniform mat4 al_ProjectionMatrix;

void main()
{
    fragPos = vec3(model * vec4(aPos, 1.0));
    fragNorm =  mat3(transpose(inverse(model))) * aNormal;
    fragTex = aTexCoord;

    gl_Position = al_ProjectionMatrix * al_ModelViewMatrix * vec4(aPos, 1.0);
}
