/* Author: BAIRAC MIHAI */

uniform sampler2D u_ColorMap;

/// Interpolated inputs across mesh
in vec2 v_uv;
///

out vec4 fragColor;

void main(void)
{	
	vec3 finalColor = texture(u_ColorMap, v_uv).rgb;

	fragColor = vec4(finalColor, 1.0f);
}
