/* Author: BAIRAC MIHAI */

uniform mat4 u_WorldToClipMatrix;

in vec3 a_position;

out vec3 v_worldPos;

void main (void)
{
	v_worldPos = a_position;

	gl_Position = u_WorldToClipMatrix * vec4(a_position, 1.0f);
}
