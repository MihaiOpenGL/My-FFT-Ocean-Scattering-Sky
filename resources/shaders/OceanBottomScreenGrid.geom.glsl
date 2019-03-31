/* Author: BAIRAC MIHAI */

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

// Projector data
uniform mat4 u_ProjectingMatrix;
uniform float u_PlaneDistance;

// Matrices
uniform mat4 u_WorldToCameraMatrix;
uniform mat4 u_WorldToClipMatrix;
uniform mat4 u_ClipToCameraMatrix;
uniform mat4 u_CameraToWorldMatrix;

uniform vec3 u_CameraPosition;

struct PerlinData
{
	sampler2D DisplacementMap;
	float Scale;
	float Amplitude;
};
uniform PerlinData u_PerlinData;

uniform float u_PatchSize;

in vec2 v_uv[];

out vec2 v_baseUV;
out vec2 v_causUV;
out float v_fogCoord;

/*  Ray - Plane Intersection
	source: https://www.cs.princeton.edu/courses/archive/fall00/cs426/lectures/raycast/sld017.htm

	Ray: P = O + t * D; O - origin, D - direction, t - scalar
	Plane: dot(P, N) + d = 0; P - point, N - normal, d - distance from origin

	Solution:
	t = - (dot(O, N) + d) / dot(D, N)

	eps - epsilon

	// if abs(dot(D, N)) <= eps then ray and plane are parallel
	// if t <= eps there is no intersection point

	P = O + t * D // point of intersection
*/

bool equalAbs (float a, float b, float epsilon)
{
	// default epsilon = 1e-4f;

	return abs(a - b) < epsilon;
}

bool projectScreenPosToWorld (in vec3 pos_screen, out vec3 pos_world) 
{
    vec3 cameraDir = normalize((u_ClipToCameraMatrix * vec4(pos_screen, 1.0f)).xyz);

    vec3 worldDir = (u_CameraToWorldMatrix * vec4(cameraDir, 0.0f)).xyz;

	if (equalAbs(worldDir.y, 0.0f, 1e-6f)) // ray parallel to plane (no intersection or infinite number of intersections)
	{
		pos_world = vec3(0.0f);
		return false;
	}

    float t = - (u_CameraPosition.y + u_PlaneDistance) / worldDir.y;

	if (t <= 0.0f) // no intersection
	{
		pos_world = vec3(0.0f);
		return false;
	}

	pos_world = u_CameraPosition + t * worldDir;

	return true;
}

void main (void)
{
	vec3 worldPos;

	for (int i = 0; i < gl_in.length(); i ++)
	{
		if (projectScreenPosToWorld(gl_in[i].gl_Position.xyz, worldPos))
		{

			////////////////////////////

			vec4 v = u_WorldToCameraMatrix * vec4(worldPos, 1.0f);
			v_fogCoord = abs(v.z / v.w);

			v_baseUV = worldPos.xz / u_PatchSize;

			// NOTE! Not yet sure, but for it to work properly I had to inverse the the uv on OY axis !
			v_causUV = vec2(v_uv[i].x, 1.0f - v_uv[i].y); //works pretty well with some artifacts

			float height = texture(u_PerlinData.DisplacementMap, v_baseUV * u_PerlinData.Scale).w * u_PerlinData.Amplitude;
			worldPos.y += height;

			gl_Position = u_WorldToClipMatrix * vec4(worldPos, 1.0f);

			EmitVertex();
		}
	}

	EndPrimitive();
}