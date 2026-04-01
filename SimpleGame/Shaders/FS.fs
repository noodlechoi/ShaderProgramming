#version 330

layout(location=0) out vec4 FragColor;

in vec2 v_TPos;
uniform float u_Time;

const float c_PI = 3.141592;

void Colorful()
{
	if(v_TPos.x + v_TPos.y < 1)
		FragColor = vec4(0, 0, 0, 1);
	else
		FragColor = vec4(v_TPos, 0, 1);
}

void LinePattern()
{
	float lineCountH = 10;
	float lineCountV = 2;
	float lineWidth = 1;
	lineCountH /= 2;
	lineCountV /= 2;
	lineWidth = 50 / lineWidth;
	float per = -0.5 * c_PI;
	float grey = pow(abs(sin((v_TPos.y*2*c_PI + per) * lineCountH)), lineWidth);
	grey += pow(abs(sin((v_TPos.x*2*c_PI + per) * lineCountV)), lineWidth);
	FragColor = vec4(grey);
}

void Circle()
{
	vec2 center = vec2(0.5, 0.5);
	float d = distance(center, v_TPos);
	float lineWidth = 0.01;
	float radius = 0.1;

	if(d > radius - lineWidth && d < radius)
		FragColor = vec4(1);
	else
		FragColor = vec4(0);
}

void CircleSin()
{
	vec2 center = vec2(0.5, 0.5);
	float time = mod(u_Time, 1.0f);
	float d = distance(center, v_TPos) - time * 2;
	float value = abs(sin(d * c_PI * 16));
	FragColor = vec4(pow(value,	16));
}

void main()
{
	CircleSin();	
}
