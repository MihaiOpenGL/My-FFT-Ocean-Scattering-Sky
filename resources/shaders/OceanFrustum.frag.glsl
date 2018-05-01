/* Author: BAIRAC MIHAI */

uniform vec3 u_Color;

out vec4 fragColor;

void main (void)
{
	fragColor = vec4(u_Color, 1.0f);
}	
