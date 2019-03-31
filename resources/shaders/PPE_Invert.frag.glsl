/* Author: BAIRAC MIHAI */

uniform sampler2D u_ColorMap;

/// Interpolated inputs across mesh
in vec2 v_uv;
///

out vec4 fragColor;

void main(void)
{	
	vec3 finalColor = vec3(0.0f);

	vec3 color = texture(u_ColorMap, v_uv).rgb;
  
	finalColor = vec3(1.0f) - color;

	fragColor = vec4(finalColor, 1.0f);
}
