#version 330

uniform float u_Time;
in vec3 a_Position;
in float a_Mass;
in vec2 a_Vel;
in float a_RV;
in float a_RV1;

const float c_PI = 3.141592;
const float c_GV = -9.8;

void Sin1()
{
	float t = u_Time;
	vec4 newPosition;
	newPosition = vec4(a_Position.x + t, a_Position.y + 0.5 * sin(t * 2 * 3.141592), a_Position.z, 1);
	gl_Position = newPosition;
}

void Sin2()
{
	float offset = -1.0f;
	float t = u_Time;
	vec4 newPosition;
	newPosition = vec4(a_Position.x + t + offset, a_Position.y + 0.5 * sin(t * 3.141592), a_Position.z, 1);
	gl_Position = newPosition;
}
/*
void Circle()
{
	float offset = u_Time * 2 * 3.141592;
	float r = 1.0f;
	vec4 newPosition;
	newPosition = vec4(a_Position.x + r * cos(offset) , a_Position.y + r * sin(offset), a_Position.z, 1);
	gl_Position = newPosition;
}
*/

// psuedo random generator
float random(float n) {
    return fract(sin(n) * 43758.5453123);
}

vec3 Circle()
{
	float rv = mod(a_RV, 1.0);
	float offset = rv * 2 * 3.141592;
	float r = 1.0f;
	vec3 newPosition;
	newPosition = vec3(a_Position.x * (1 - a_RV1)+ r * cos(offset) , a_Position.y * (1 - a_RV1) + r * sin(offset), a_Position.z);
	return newPosition;
}

vec4 Falling(vec3 pos)
{
	float startTime = a_RV1;
	float newTime = u_Time - startTime;

	vec4 newPosition;
	if(newTime > 0) {
		float t = mod(newTime, 1.0);
		float tt = t*t;

		newPosition.x = pos.x + t * a_Vel.x;
		newPosition.y = pos.y + t * a_Vel.y + 0.5 * c_GV * tt;
		newPosition.z = 0;
		newPosition.w = 1;
	}
	else {
		newPosition = vec4(-500, 0, 0, 0);
	}

	return newPosition;
}

void main()
{
	vec3 pos = Circle();
	gl_Position = Falling(pos);
}
