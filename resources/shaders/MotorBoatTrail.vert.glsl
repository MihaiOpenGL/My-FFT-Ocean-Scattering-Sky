/* Author: BAIRAC MIHAI */

uniform mat4 u_WorldToClipMatrix;
uniform mat4 u_ObjectToWorldMatrix;

in vec3 a_position;


void main (void)
{
	gl_PointSize = 5; // Debug

	vec4 world_pos = u_ObjectToWorldMatrix * vec4(a_position, 1.0f);

	gl_Position = u_WorldToClipMatrix * world_pos;
}
