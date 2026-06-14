#version 330

layout(location=0) out vec4 FragColor;

uniform vec4 u_Color;

in vec2 v_Tex;

void main()
{
	FragColor = u_Color;
}
