#version 330

in vec3 a_Pos;
in vec2 a_tPos;

out vec2 v_TPos;

void main()
{
	gl_Position = vec4(a_Pos, 1);
	v_TPos = a_tPos;
}
