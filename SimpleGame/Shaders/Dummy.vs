#version 330

uniform float u_Time;
uniform vec4 u_DropInfo[1000];	// vec4(x, y, sT, lT)

in vec3 a_Pos;
out float v_Grey;
out vec2 v_Tex;

const float c_PI = 3.141592;

void Flag()
{
	float tx, ty;
	tx = a_Pos.x + 0.5;
	ty = 1.0 - (a_Pos.y + 0.5);
	v_Tex = vec2(tx, ty);
	float value = a_Pos.x + 0.5;
	float newX = a_Pos.x;
	// x가 -0.5에 가까울 수록 0이 되도록
	float sinNum = sin((newX + 0.5) * c_PI * 2 - u_Time);
	float newY = a_Pos.y * (1.0 - value * 0.5) + 0.25 * value * sinNum;

	vec4 final = vec4(newX, newY, 0.0, 1.0);

	v_Grey = (sinNum + 1) / 2;

	gl_Position = final;
}

void Circles()
{
	float accum = 0;
	for(int i = 0; i < 1000; ++i) {
		vec2 center = u_DropInfo[i].xy - vec2(0.5, 0.5);
		vec2 pos = a_Pos.xy;
		float lTime = u_DropInfo[i].z;
		float sTime = u_DropInfo[i].w;
		float nTime = u_Time-sTime;

		if(nTime > 0)
		{
			float lVal = fract(nTime / lTime);
			float oneMinus = 1.0 - lVal;
			float t = lVal*lTime;
			float d = distance(center, pos);
			float range = t/30.0;
			float fade = 15.0 * clamp(range - d, 0, 1.0);
			float sinValue = pow(abs(sin(d*4*c_PI*8 + t * 2)), 3.0);

			accum += sinValue * fade * oneMinus;
		}

	}
	v_Grey = accum;
	//gl_Position = vec4(a_Pos, 1.0);
	gl_Position = vec4(a_Pos. x, a_Pos.y+accum* 0.1, a_Pos.z, 1.0);
}

void main()
{
	Circles();
}