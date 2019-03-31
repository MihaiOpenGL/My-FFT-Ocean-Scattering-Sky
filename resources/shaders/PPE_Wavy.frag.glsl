/* Author: BAIRAC MIHAI

Implemented a wavy like effect just playing with the sin function
https://stackoverflow.com/questions/1061093/how-is-a-sepia-tone-created

*/

uniform sampler2D u_ColorMap;
uniform float u_CrrTime;

/// Interpolated inputs across mesh
in vec2 v_uv;
///

out vec4 fragColor;

#define PI 3.141592657f

void main(void)
{	
	vec3 finalColor = vec3(0.0f);
  
	vec2 uv_offset = v_uv;
	uv_offset.x += sin((v_uv.x * 3.0f + u_CrrTime * 0.5f) * 2.0f * PI) / 100.0f;
	uv_offset.y += sin((v_uv.y * 5.0f + u_CrrTime * 0.5f) * 2.0f * PI) / 100.0f;

	finalColor = texture(u_ColorMap, uv_offset).rgb;

	fragColor = vec4(finalColor, 1.0f);
}
