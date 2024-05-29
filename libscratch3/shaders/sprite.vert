#version 330 core

layout (location = 0) in vec4 aVertex; // <vec2 pos, vec2 tex>

out vec2 TexCoords;

uniform mat4 uProj;
uniform mat4 uModel;

void main()
{
    TexCoords = aVertex.zw;
    gl_Position = uProj * uModel * vec4(aVertex.xy, 0.0, 1.0);
}
