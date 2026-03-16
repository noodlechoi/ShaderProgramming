#version 330

uniform float u_Time;
in vec3 a_Position;

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

void main()
{
	Circle();
}
