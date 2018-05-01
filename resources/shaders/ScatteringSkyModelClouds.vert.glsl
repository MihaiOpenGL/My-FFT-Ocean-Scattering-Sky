/* Author: BAIRAC MIHAI */

uniform mat4 u_WorldToClipMatrix;
uniform mat4 u_ObjectToWorldMatrix;

in vec3 a_position;
in vec2 a_uv;

out vec2 v_uv;

void main(void)
{
	v_uv = a_uv;

	vec4 world_pos = u_ObjectToWorldMatrix * vec4(a_position, 1.0f);

	// we use the w value instead of z as explained in here: https://learnopengl.com/#!Advanced-OpenGL/Cubemaps
	gl_Position = u_WorldToClipMatrix * world_pos;
}
