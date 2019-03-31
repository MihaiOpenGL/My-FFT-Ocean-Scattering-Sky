/* Author: BAIRAC MIHAI

 Caustics code uses a small portion of the below work
 code: http://madebyevan.com/webgl-water/renderer.js
 paper: https://medium.com/@evanwallace/rendering-realtime-caustics-in-webgl-2a99a29a0b2c

 License: MIT License

*/

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

// Projector data
uniform mat4 u_BottomProjectingMatrix;
uniform float u_BottomPlaneDistance;
uniform float u_PlaneDistanceOffset;
// Matrices
uniform mat4 u_WorldToClipMatrix;
uniform mat4 u_ClipToCameraMatrix;
uniform mat4 u_CameraToWorldMatrix;

uniform vec3 u_CameraPosition;

uniform sampler2DArray u_FFTWaveDataMap;

uniform float u_PatchSize;
uniform float u_ChoppyScale;

uniform vec3 u_SunDirection;

out vec3 v_oldPos;
out vec3 v_newPos;


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

    float t = - (u_CameraPosition.y + u_BottomPlaneDistance) / worldDir.y;

	if (t <= 0.0f) // no intersection
	{
		pos_world = vec3(0.0f);
		return false;
	}

	pos_world = u_CameraPosition + t * worldDir;

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

	for (int i = 0; i < gl_in.length(); i ++)
	{
		if (projectScreenPosToWorld(gl_in[i].gl_Position.xyz, worldPos))
		{
			///////////////////
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

			EmitVertex();
		}
	}

	EndPrimitive();
}