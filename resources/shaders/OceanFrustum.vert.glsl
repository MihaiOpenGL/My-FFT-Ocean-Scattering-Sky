/* Author: BAIRAC MIHAI */

uniform mat4 u_ViewClipToWorldMatrix;
uniform mat4 u_ProjectorClipToWorldMatrix;
uniform mat4 u_WorldToClipMatrix;

uniform bool u_IsViewFrustum;

in vec3 a_position;

void main (void)
{	
	vec4 pos = (u_IsViewFrustum ? u_ViewClipToWorldMatrix * vec4(a_position, 1.0f) : u_ProjectorClipToWorldMatrix * vec4(a_position, 1.0f));

	gl_Position = u_WorldToClipMatrix * pos;
}
