/* Author: BAIRAC MIHAI 

Black and White effect is based on the Greyscale effect + a step function
more info: http://www.tannerhelland.com/3643/grayscale-image-algorithm-vb6/

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
  
	float greyscale = color.r * 0.299f + color.g * 0.587f + color.b * 0.114f;
	greyscale = step(0.35f, greyscale);
	finalColor = vec3(greyscale);

	fragColor = vec4(finalColor, 1.0f);
}
