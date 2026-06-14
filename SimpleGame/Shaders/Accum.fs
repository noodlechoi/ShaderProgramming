#version 330

layout(location=0) out vec4 FragColor;

uniform sampler2D u_Tex;
uniform sampler2D u_TexBlurred;

uniform float u_Exposure;

in vec2 v_Tex;

void main()
{
	float gamma = 2.2;
	vec3 color = texture(u_Tex, v_Tex).rgb;
	vec3 blurred = texture(u_TexBlurred, vec2(v_Tex.x, 1- v_Tex.y)).rgb;

	vec3 summed = color + blurred;

	vec3 tonemapped = vec3(1.0) - exp(-summed * u_Exposure);

	vec3 result = pow(tonemapped, vec3(1.0/gamma));

	FragColor = vec4(summed, 1);
}
