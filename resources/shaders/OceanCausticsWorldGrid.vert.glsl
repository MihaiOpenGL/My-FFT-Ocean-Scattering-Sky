/* Author: BAIRAC MIHAI

 Caustics code uses a small portion of the below work
 code: http://madebyevan.com/webgl-water/renderer.js
 paper: https://medium.com/@evanwallace/rendering-realtime-caustics-in-webgl-2a99a29a0b2c

 License: MIT License

*/

uniform mat4 u_BottomProjectingMatrix;
uniform float u_BottomPlaneDistance;
uniform float u_PlaneDistanceOffset;

uniform mat4 u_WorldToClipMatrix;

uniform sampler2DArray u_FFTWaveDataMap;

uniform float u_PatchSize;
uniform float u_ChoppyScale;

uniform vec3 u_SunDirection;

in vec2 a_uv;

out vec3 v_oldPos;
out vec3 v_newPos;

bool equalAbs (float a, float b, float epsilon)
{
	// default epsilon = 1e-4f;

	return abs(a - b) < epsilon;
}

bool projectWavePosToWorld (in vec2 uv, out vec3 worldPos)
{
	// ray in homogenous normalized device coordinates (post-perspective space)
    vec4 origin = vec4(uv, -1.0f, 1.0f);
    vec4 direction = vec4(uv, 1.0f, 1.0f);
    
	// transform them to world space homogenous coordinates (projection is done here)
	origin	  = u_BottomProjectingMatrix * origin;
	direction = u_BottomProjectingMatrix * direction;

	// make the perspective division to bring each position in 3d world space coordinates
	// avoid division by zero
	if (equalAbs(origin.w, 0.0f, 1e-6f)) origin.w = 1e-6f;
	if (equalAbs(direction.w, 0.0f, 1e-6f)) direction.w = 1e-6f;
	origin /= origin.w;
	direction /= direction.w;

	// obtain the actual direction of the ray: dir = normalize(p2 - p1)
	direction = normalize(direction - origin);

	if (equalAbs(direction.y, 0.0f, 1e-6f)) // ray parallel to plane (no intersection or infinite number of intersections)
	{
		worldPos = vec3(0.0f);
		return false;
	}

	// intersect ray (O, D) with plane (P, N, d)
	float t = - (origin.y + u_BottomPlaneDistance) / direction.y;

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

bool calcIntersectionPos (in vec3 i_rayOrigin, in vec3 i_rayDirection, in float i_planeDistance, out vec3 o_intersection)
{
	if (equalAbs(i_rayDirection.y, 0.0f, 1e-6f))  // ray parallel to plane (no intersection or infinite number of intersections)
	{
		o_intersection = vec3(0.0f);
		return false;
	}

	// intersect ray (O, D) with plane (P, N, d)
    float t = - (i_rayOrigin.y + i_planeDistance) / i_rayDirection.y;

	if (t <= 0.0f) // no intersection
	{
		o_intersection = vec3(0.0f);
		return false;
    }

	// obtain the point of intersection in world space coordinates
    o_intersection = i_rayOrigin + i_rayDirection * t;

	return true;
}

void main (void)
{
	vec3 worldPos, rayOrigin, rayDirection;

#ifdef USE_GRID_CORNERS_BOTTOM
	vec4 result = mix(mix(u_BottomProjectingMatrix[0], u_BottomProjectingMatrix[1], a_uv.x), mix(u_BottomProjectingMatrix[2], u_BottomProjectingMatrix[3], a_uv.x), a_uv.y);

	// result.w can be negative !!!
//	if (result.w < 1e-6f) result.w = 1e-6f;
	float divide = 1.0f / result.w;

	result.xz *= divide;

	worldPos = result.xyz;
#else
	projectWavePosToWorld(a_uv, worldPos);
#endif // USE_GRID_CORNERS_BOTTOM

	vec2 waveUV = worldPos.xz / u_PatchSize;

	vec3 disp = texture(u_FFTWaveDataMap, vec3(waveUV, 0)).xyz;
	disp.xz *= u_ChoppyScale;

	vec2 fftSlope = texture(u_FFTWaveDataMap, vec3(waveUV, 1)).xy;
	vec3 waveNormal = normalize(vec3(fftSlope.x, 1.0f, fftSlope.y));

	vec3 lightDir = normalize(u_SunDirection);

	const float airRefrIndex = 1.0f;
	const float waterRefrIndex = 1.33f;
	float ratio = airRefrIndex / waterRefrIndex;
	vec3 refr = refract(- lightDir, waveNormal, ratio);
	vec3 refrLight = refract(- lightDir, vec3(0.0f, 1.0f, 0.0f), ratio);

	vec3 oldPos, newPos;

	float planeDistance = u_BottomPlaneDistance + u_PlaneDistanceOffset;

	calcIntersectionPos(worldPos, refrLight, planeDistance, oldPos);
	calcIntersectionPos(worldPos + vec3(disp.x, 0.0f, disp.z), refr, planeDistance, newPos);

	v_oldPos = oldPos;
	v_newPos = newPos;

	gl_Position = u_WorldToClipMatrix * vec4(newPos, 1.0f);
}