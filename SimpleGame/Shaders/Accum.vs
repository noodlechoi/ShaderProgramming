#version 330

in vec3 a_Pos;
uniform bool u_Flip;

out vec2 v_Tex;

void main()
{
	vec4 newPosition = vec4(a_Pos, 1);
	gl_Position = newPosition;

	v_Tex.x = (a_Pos.x + 1.0) / 2.0;
	if(u_Flip)
		v_Tex.y = 1.0 - (a_Pos.y + 1.0) / 2.0;
	else
		v_Tex.y = (a_Pos.y + 1.0) / 2.0;
}
