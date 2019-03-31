/* Author: BAIRAC MIHAI

Based on Eric Bruneton's work:

The precomputed scattering algorithms and glsl functions are taken and adapted based on
Precomputed Atmospheric Scattering effect
Code + paper: http://www-evasion.imag.fr/people/Eric.Bruneton/
License: check the code

*/

uniform mat4 u_ClipToCameraMatrix;
uniform mat4 u_CameraToWorldMatrix;

in vec3 a_position;

out vec3 v_viewDir;
out vec2 v_UV;

void main (void)
{
	vec3 viewPos = (u_ClipToCameraMatrix * vec4(a_position, 1.0f)).xyz;
	v_viewDir = (u_CameraToWorldMatrix * vec4(viewPos, 0.0f)).xyz;

	v_UV = a_position.xz;

    gl_Position = vec4(a_position.xy, 0.9999999f, 1.0f);
}
