/* Author: BAIRAC MIHAI

Implemented a sepia effect using values from 
https://stackoverflow.com/questions/1061093/how-is-a-sepia-tone-created

*/

uniform sampler2D u_ColorMap;

/// Interpolated inputs across mesh
in vec2 v_uv;
///

out vec4 fragColor;

void main(void)
{	
	vec3 finalColor = vec3(0.0f);

	vec3 color = texture(u_ColorMap, v_uv).rgb;
  
	finalColor.r = (color.r * 0.393f) + (color.g * 0.769f) + (color.b * 0.189f);
	finalColor.g = (color.r * 0.349f) + (color.g * 0.686f) + (color.b * 0.168f);
	finalColor.b = (color.r * 0.272f) + (color.g * 0.534f) + (color.b * 0.131f);

	fragColor = vec4(finalColor, 1.0f);
}
