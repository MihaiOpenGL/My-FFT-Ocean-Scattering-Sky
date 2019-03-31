/* Author: BAIRAC MIHAI */

// Projector data
uniform mat4 u_ProjectingMatrix;
uniform float u_PlaneDistance;
// Matrices
uniform mat4 u_WorldToCameraMatrix;
uniform mat4 u_WorldToClipMatrix;

struct PerlinData
{
	sampler2D DisplacementMap;
	float Scale;
	float Amplitude;
};
uniform PerlinData u_PerlinData;

uniform float u_PatchSize;

in vec2 a_uv;

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


bool projectPosToWorld (in vec2 uv, out vec3 worldPos)
{
	// ray in homogenous normalized device coordinates (post-perspective space)
    vec4 origin = vec4(uv, -1.0f, 1.0f);
    vec4 direction = vec4(uv, 1.0f, 1.0f);
    
	// transform them to world space homogenous coordinates (projection is done here)
    origin	  = u_ProjectingMatrix * origin;
	direction = u_ProjectingMatrix * direction;

	// make the perspective division to bring each position in 3d world space coordinates
//	if (equalAbs(origin.w, 0.0f, 1e-6f)) origin.w = 1e-6f;
//	if (equalAbs(direction.w, 0.0f, 1e-6f)) direction.w = 1e-6f;
	origin /= origin.w;
	direction /= direction.w;

	// obtain the actual direction of the ray: dir = normalize(p2 - p1)
	direction = normalize(direction - origin);
	//direction -= origin;

	if (equalAbs(direction.y, 0.0f, 1e-6f)) // ray parallel to plane (no intersection or infinite number of intersections)
	{
		worldPos = vec3(0.0f);
		return false;
	}

	// intersect ray (O, D) with plane (P, N, d)
    float t = - (origin.y + u_PlaneDistance) / direction.y;

	if (t <= 0.0f) // no intersection
	{
		worldPos = vec3(0.0f);
		return false;
	}
    
	// obtain the point of intersection in world space homogenous coordinates
    vec4 wP = origin + direction * t;

	worldPos = wP.xyz;

	return true;
}

void main (void)
{
	vec3 worldPos;

#ifdef USE_GRID_CORNERS_BOTTOM
	// by computing the grid vertex positions on CPU based on the 4 edges of the grid we avoid interpolation and float precision errors
	vec4 result = mix(mix(u_ProjectingMatrix[0], u_ProjectingMatrix[1], a_uv.x), mix(u_ProjectingMatrix[2], u_ProjectingMatrix[3], a_uv.x), a_uv.y);

	if (result.w < 1e-6f) result.w = 1e-6f;
	float divide = 1.0f / result.w;

	result.xz *= divide;

	worldPos = result.xyz;
#else
	projectPosToWorld(a_uv, worldPos);
#endif // USE_GRID_CORNERS_BOTTOM


	vec4 v = u_WorldToCameraMatrix * vec4(worldPos, 1.0f);
	v_fogCoord = abs(v.z / v.w);

	v_baseUV = worldPos.xz / u_PatchSize;

	v_causUV = a_uv; //works pretty well with some artifacts

	float height = texture(u_PerlinData.DisplacementMap, v_baseUV * u_PerlinData.Scale).w * u_PerlinData.Amplitude;
	worldPos.y += height;

	gl_Position = u_WorldToClipMatrix * vec4(worldPos, 1.0f);
}