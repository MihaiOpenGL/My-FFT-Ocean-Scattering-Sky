/* Author: BAIRAC MIHAI

Implemented a simplied version of linear gaussian blur
info: https://github.com/mattdesl/lwjgl-basics/wiki/shaderlesson5

 */

uniform sampler2D u_ColorMap;

uniform vec4 u_Step;

/// Interpolated inputs across mesh
in vec2 v_uv;
///

out vec4 fragColor;

void main(void)
{	
	vec3 finalColor = vec3(0.0f);

	vec3 color1 = texture(u_ColorMap, v_uv).rgb;
  
	const float weights[3] = float[3](2.0f, 3.0f, 5.0f);
  
	// iterate 3 times each step being based on the weight
	for ( int i = 0; i < 3; ++ i)
	{
		vec4 step = u_Step * weights[i];	

		// the method with 8 points - we sample both horizontally and vertically
		vec3 color2 = texture(u_ColorMap, vec2(v_uv.x + step.x, v_uv.y)).rgb;
		vec3 color3 = texture(u_ColorMap, vec2(v_uv.x - step.x, v_uv.y)).rgb;
		vec3 color4 = texture(u_ColorMap, vec2(v_uv.x, v_uv.y + step.y)).rgb; 
		vec3 color5 = texture(u_ColorMap, vec2(v_uv.x, v_uv.y - step.y)).rgb;

		vec3 color6 = texture(u_ColorMap, vec2(v_uv.x + step.z, v_uv.y + step.w)).rgb;
		vec3 color7 = texture(u_ColorMap, vec2(v_uv.x - step.z, v_uv.y + step.w)).rgb;
		vec3 color8 = texture(u_ColorMap, vec2(v_uv.x - step.z, v_uv.y - step.w)).rgb;
		vec3 color9 = texture(u_ColorMap, vec2(v_uv.x + step.z, v_uv.y - step.w)).rgb;

		finalColor += (color1 * 2.0f + color2 + color3 + color4 + color5 + color6 + color7 + color8 + color9) * 0.15f;
	}
	finalColor *= 0.2f;

	fragColor = vec4(finalColor, 1.0f);
}
