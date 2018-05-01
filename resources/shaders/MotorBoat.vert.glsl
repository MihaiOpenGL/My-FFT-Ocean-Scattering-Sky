/* Author: BAIRAC MIHAI

opengl clipping planes info: http://mrkaktus.org/opengl-clip-planes-explained/
modern way for clip planes: http://github.prideout.net/clip-planes

*/

uniform mat4 u_WorldToClipMatrix;
uniform mat4 u_ObjectToWorldMatrix;

uniform vec4 u_ReflClipPlane;
uniform vec4 u_RefrClipPlane;

in vec3 a_position;
in vec3 a_normal; ///
in vec2 a_uv;

out vec3 v_normal; ///
out vec2 v_uv;

out float gl_ClipDistance[2]; //clip plane for local reflection and refraction

void main (void)
{
	v_normal = a_normal; ///
	v_uv = a_uv;

	vec4 world_pos = u_ObjectToWorldMatrix * vec4(a_position, 1.0f);

	gl_Position = u_WorldToClipMatrix * world_pos;

	// clip plane for above water reflection
	gl_ClipDistance[0] = dot(world_pos, u_ReflClipPlane);

	// clip plane for under water refraction
	gl_ClipDistance[1] = dot(world_pos, u_RefrClipPlane);
}
