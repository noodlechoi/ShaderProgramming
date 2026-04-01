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

void CuteFractal()
{
    // 1. 좌표계 중심 이동 (-0.5 ~ 0.5)
    vec2 uv = v_TPos - 0.5;
    vec3 finalColor = vec3(0.0);
    
    // 2. 프랙탈 반복 (3~4회 반복하면 충분히 귀여운 디테일이 나옵니다)
    for (float i = 0.0; i < 3.0; i++) {
        // 공간을 계속 쪼개고 반복시킴 (핵심 프랙탈 로직)
        uv = fract(uv * 1.5) - 0.5;
        
        // 중심으로부터의 거리 계산
        float d = length(uv) * exp(-length(v_TPos - 0.5));
        
        // 귀여운 파동의 색상 (분홍, 민트, 보라 계열)
        vec3 col = 0.5 + 0.5 * cos(u_Time + uv.xyx + vec3(0, 2, 4));
        
        // 제공해주신 CircleSin의 파동 로직을 변형해서 적용
        // 몽글몽글한 느낌을 위해 sin 값을 부드럽게 조절
        d = sin(d * 8.0 + u_Time) / 8.0;
        d = abs(d);
        
        // 파동을 아주 얇고 선명하게 (pow 사용)
        d = pow(0.01 / d, 1.2);
        
        finalColor += col * d;
    }
    
    FragColor = vec4(finalColor, 1.0);
}

void main()
{
    CuteFractal();
}