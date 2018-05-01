/* Author: BAIRAC MIHAI */

uniform float u_SunDirY;

out vec4 fragColor;

void main (void)
{
	// [0, 1] -> [many values near 0, 1]
	// https://www.wolframalpha.com/input/?i=1-0.01%2Fx,+x+%3D+%5B0,+1%5D

	float x = u_SunDirY;
	float y = max(1.0f - 0.07f / x, 0.0f);
	vec3 occluderColor = vec3(1.0f, y + 0.5f, y * y); //mostly white to orange at the end

	fragColor = vec4(occluderColor, 1.0f);
}