#version 330

layout(location=0) out vec4 FragColor;

in float v_Grey;
in vec2 v_Tex;
in vec3 v_Color;

void CircleShape()
{
	float d = distance(vec2(0.5,0.5), v_Tex);
	if(d < 0.5) {
		FragColor = vec4(v_Color, clamp(0.5 - d, 0, 0.5) * 2);
	}
	else
		FragColor = vec4(0);
}

void main()
{
	CircleShape();
}
