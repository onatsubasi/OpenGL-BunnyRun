#version 330 core

out vec4 FragColor;
in vec3 pos;
uniform vec3 color1; // First color (e.g., white)
uniform vec3 color2; // Second color (e.g., black)
uniform float offsetX;
uniform float offsetZ; // Offset for shifting pattern
uniform float scale;  // Scale for the size of squares

bool checkered(vec3 pos) {
    bool x = int((pos.x + offsetX) * scale) % 2 == 0;
    bool y = int((pos.y + offsetX) * scale) % 2 == 0;
    bool z = int((pos.z + offsetZ) * scale) % 2 == 0;
    bool xorXY = x != y;
    return xorXY != z;
}

void main() {

    if (checkered(pos)) {
        FragColor = vec4(color1, 1.0);
    } else {
        FragColor = vec4(color2, 1.0);
    }
}
