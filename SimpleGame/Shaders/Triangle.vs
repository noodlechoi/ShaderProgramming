#version 330

uniform float u_Time;
in vec3 a_Position;
in float a_Mass;
in vec2 a_Vel;

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

void Circle()
{
	float offset = u_Time * 2 * 3.141592;
	float r = 1.0f;
	vec4 newPosition;
	newPosition = vec4(a_Position.x + r * cos(offset) , a_Position.y + r * sin(offset), a_Position.z, 1);
	gl_Position = newPosition;
}

void Particle()
{
	float t = mod(u_Time, 1.0);
	float tt = t*t;
	vec4 newPosition;
	newPosition.x = a_Position.x + t * a_Vel.x;
	newPosition.y = a_Position.y + t * a_Vel.x + 0.5 * c_GV * tt;
	newPosition.z = 0;
	newPosition.w = 1;

	gl_Position = newPosition;
}

void main()
{
	Particle();
}
