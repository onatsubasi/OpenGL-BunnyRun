#version 330 core

layout(location = 0) in vec3 aPos;

uniform mat4 modelingMatrix;
uniform mat4 viewingMatrix;
uniform mat4 projectionMatrix;

out vec3 pos;
void main() {
    gl_Position = projectionMatrix * viewingMatrix * modelingMatrix * vec4(aPos, 1.0);
    pos = gl_Position.xyz;
}
