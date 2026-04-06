#version 330

layout(location=0) out vec4 FragColor;

in vec2 v_TPos;
uniform float u_Time;
uniform vec4 u_DropInfo[1000];	// vec4(x, y, sT, lT)

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

void CircleSin()    // 퍼져나가는 이미지에 사용(파동, 레이더)
{
	vec2 center = vec2(0.5, 0.5);
	float time = mod(u_Time, 1.0f);
	float d = distance(center, v_TPos) - time * 2;
	float value = abs(sin(d * c_PI * 16));
	FragColor = vec4(pow(value,	16));
}

void RainDrop()
{
	float accum = 0;
	for(int i = 0; i < 1000; ++i) {
		float lifeTime = u_DropInfo[i].w;
		float startTime = u_DropInfo[i].z;
		float newTime = u_Time - startTime;
		if(newTime > 0) {
			newTime = fract(newTime/lifeTime);
			float oneMinus = 1 - newTime;
			float t = newTime * lifeTime;

			vec2 center = u_DropInfo[i].xy;
			float d = distance(center, v_TPos);
			float range = t * 0.1;
			float fade = 30 * clamp(range - d, 0, 1);

			float value = pow(abs(sin(d * 4 * c_PI * 10 - t *15)), 16);
			accum += value * fade * oneMinus;
		}
	}
	FragColor = vec4(accum);
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

// 발자국 모양의 거리를 계산하는 함수
float getFootprintDistance(vec2 center, vec2 uv)
{
    vec2 p = uv - center;
    
    // 1. 메인 발바닥 (약간 타원형)
    float mainPad = length(p * vec2(1.0, 1.2)) - 0.04;
    
    // 2. 발가락 3개 (작은 원들)
    float toe1 = length(p - vec2(-0.03, 0.05)) - 0.015;
    float toe2 = length(p - vec2( 0.0,  0.07)) - 0.015;
    float toe3 = length(p - vec2( 0.03, 0.05)) - 0.015;
    
    // 모든 모양 중 가장 가까운 거리를 선택 (합치기)
    return min(mainPad, min(toe1, min(toe2, toe3)));
}

void FootprintDrop()
{
    float accum = 0;
    for(int i = 0; i < 50; ++i) { // 성능을 위해 루프 횟수 조절 (테스트용)
        float lifeTime = u_DropInfo[i].w;
        float startTime = u_DropInfo[i].z;
        float newTime = u_Time - startTime;
        
        if(newTime > 0) {
            float progress = fract(newTime / lifeTime); // 0~1 반복
            float fade = 1.0 - progress;
            
            vec2 center = u_DropInfo[i].xy;
            
            // 발자국 모양과의 거리 계산
            float d = getFootprintDistance(center, v_TPos);
            
            // 파동이 밖으로 퍼져나가는 범위
            float range = progress * 0.15;
            
            // 발자국 테두리 근처에서만 빛나게 설정
            // d가 0에 가까울수록(발자국 안쪽), 그리고 range와 비슷할수록 강해짐
            float edge = 1.0 - smoothstep(0.0, 0.02, abs(d - range * 0.2));
            
            // 기존의 sin 파동 느낌 추가
            float ripple = pow(abs(sin(d * 100.0 - progress * 20.0)), 8.0);
            
            accum += edge * ripple * fade;
        }
    }
    // 발자국이니까 약간 따뜻한 색감이나 흰색으로 출력
    FragColor = vec4(accum * vec3(1.0, 0.8, 0.9), 1.0);
}

void main()
{
    FootprintDrop();
}