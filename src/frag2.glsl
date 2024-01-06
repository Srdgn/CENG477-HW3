#version 330 core

varying vec4 FragPos;  // Assuming this is the fragment's position in world space


out vec4 FragColor;

float offset = 0.0f;
float scale = 1.0f;
void main()
{
    // Assuming you have computed x, y, and z in the vertex shader and passed them as varying to the fragment shader
    FragPos = gl_Vertex;
    bool x = int((FragPos.x + offset) * scale) % 2 == 1;
    bool y = int((FragPos.y + offset) * scale) % 2 == 1;
    bool z = int((FragPos.z + offset) * scale) % 2 == 1 ;
    bool xorXY = x != y;

    if (xorXY != z )
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);  // Black
    else
        FragColor = vec4(1.0, 1.0, 1.0, 1.0);  // White
}
