#version 330

uniform float u_Time;
in vec3 a_Position;
in float a_Mass;
in vec2 a_Vel;
in float a_RV;
in float a_RV1;
in float a_RV2;

out float v_Grey;

const float c_PI = 3.141592;
const float c_GV = -9.8;

void Sin0()	// 시험 문제
{
	float startTime = a_RV1 * 2;
	float newTime = u_Time - startTime;
	if(newTime > 0) {
		float t = mod(newTime, 1.0);
		float amp = 0.5 * (a_RV - 0.5) * 2 * (1-t);
		float size = 0.3 * a_RV2 * (1-t);
		float period = 0.5 * a_RV1;
		vec4 newPosition;
		newPosition = vec4(a_Position.x * size + t, a_Position.y * size + amp * sin(t * c_PI * period), a_Position.z, 1);
		gl_Position = newPosition;
		v_Grey = (1-t);
	}
	else {
		gl_Position = vec4(1000, 10, 10, 10);
		v_Grey = 0;
	}
}

void foundation()
{
// 1. 생명 주기 설정 (분수는 약 1.5초 정도 유지)
    float lifeDuration = 1.5;
    float startTime = a_RV1 * 2.0; 
    float newTime = u_Time - startTime;

    if(newTime > 0) {
        float t = mod(newTime, lifeDuration);
        float normalizedLife = t / lifeDuration; // 0.0 ~ 1.0

        // 2. 물리 연산: 위로 솟구치는 힘 (Initial Velocity)
        // a_RV1을 이용해 물줄기마다 높이를 다르게 설정
        float launchVelocity = 2.5 + a_RV1 * 1.5;
        
        // y = v0 * t + 0.5 * g * t^2 (기본 포물선)
        float posY = (launchVelocity * t) + (0.5 * c_GV * t * t);
        
        // 3. 사인파를 이용한 바람 효과 (Swaying)
        // 위로 갈수록 바람의 영향을 많이 받아 더 크게 흔들림
        float windStrength = 0.2 * a_RV; 
        float sway = sin(u_Time * 2.0 + a_RV1 * c_PI) * windStrength * t;
        
        // x축은 기본 퍼짐(a_RV) + 바람에 의한 흔들림(sway)
        float posX = (a_RV - 0.5) * 0.5 * t + sway;

        // 4. 입자 크기 변화
        // 물방울이 정점에서 떨어질 때 공기 중에서 흩어지며 작아짐
        float size = a_RV2 * 0.2 * (1.0 - normalizedLife * 0.5);

        // 5. 최종 좌표 계산
        vec4 newPosition;
        newPosition.x = (a_Position.x * size) + posX;
        newPosition.y = (a_Position.y * size) + posY;
        newPosition.z = 0.0;
        newPosition.w = 1.0;

        gl_Position = newPosition;
    }
    else {
        gl_Position = vec4(1000.0, 0.0, 0.0, 1.0); // 화면 밖
    }
}

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
	float startTime = a_RV1 * 3;
	float newTime = u_Time - startTime;

	vec4 newPosition;
	if(newTime > 0) {
		float lifeScale = 2;
		float lifeTime = 0.5 + a_RV2 * lifeScale;
		float t = lifeTime * fract(newTime / lifeTime); // 0~lifeTime 구간 반복
		// float t = mod(newTime, lifeTime); // 0~lifeTime 구간 반복
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
	Sin0();
}
