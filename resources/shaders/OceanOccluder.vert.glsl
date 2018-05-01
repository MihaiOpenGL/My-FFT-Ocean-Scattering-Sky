/* Author: BAIRAC MIHAI */

uniform sampler2DArray u_FFTWaveDataMap;
uniform float u_ChoppyScale;

uniform mat4 u_WorldToClipMatrix;

in vec3 a_position;
in vec2 a_uv;


void main (void)
{
	vec3 disp = a_position;

	disp += texture(u_FFTWaveDataMap, vec3(a_uv, 0)).xyz;
	disp.xz *= u_ChoppyScale;

	gl_Position = u_WorldToClipMatrix * vec4(disp, 1.0f);
}
