/* Author: BAIRAC MIHAI */

uniform mat4 u_WorldToClipMatrix;
uniform mat4 u_ObjectToWorldMatrix;

in vec3 a_position;

out vec3 v_uv;
out float v_fogCoord;

void main (void)
{	
	v_uv = a_position;

	vec4 world_pos = u_ObjectToWorldMatrix * vec4(a_position, 1.0f);

	// we use the w value instead of z as explained in here: https://learnopengl.com/#!Advanced-OpenGL/Cubemaps
	vec4 clip_pos = u_WorldToClipMatrix * world_pos;

	gl_Position = clip_pos.xyww;
}
