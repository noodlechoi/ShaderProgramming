#version 330

uniform float u_Time;

in vec3 a_Pos;
out float v_Grey;

const float c_PI = 3.141592;

void main()
{
	float value = a_Pos.x + 0.5;
	float newX = a_Pos.x;
	// x가 -0.5에 가까울 수록 0이 되도록
	float sinNum = sin((newX + 0.5) * c_PI * 2 - u_Time);
	float newY = a_Pos.y + 0.25 * value * sinNum;

	vec4 final = vec4(newX, newY, 0.0, 1.0);

	v_Grey = (sinNum + 1) / 2;

	gl_Position = final;
}