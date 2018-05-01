/* Author: BAIRAC MIHAI */

in vec3 a_position;
in vec2 a_uv;

out vec2 v_uv;

void main (void)
{	
	gl_PointSize = 5; //Wireframe

	v_uv = a_uv;

	gl_Position = vec4(a_position, 1.0f);
}