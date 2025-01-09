#version 130

in vec2 position;  // Input position attribute
in vec2 texCoord;  // Input texture coordinate attribute

out vec2 TexCoord; // Output texture coordinates to fragment shader

void main()
{
    gl_Position = vec4(position, 0.0, 1.0); // Transform to clip space
    TexCoord = texCoord; // Pass texture coordinates
}