/* Author: BAIRAC MIHAI

The perlin noise sampling algorithm is based on the nVidia Direct3D SDK11 OceanCS sample code
Code: https://developer.nvidia.com/dx11-samples
License: download the package and check the License.pdf file

*/

// Projector data
uniform mat4 u_ProjectingMatrix;
uniform float u_PlaneDistance;

// Matrices
uniform mat4 u_WorldToCameraMatrix;
uniform mat4 u_WorldToClipMatrix;

uniform vec3 u_CameraPosition;

uniform float u_CrrTime;

// FFT Ocean Patch data
struct FFTOceanPatchData 
{
	sampler2DArray FFTWaveDataMap;
	sampler2D NormalGradientFoldingMap;

	float PatchSize;
	float WaveAmplitude;
	float WindSpeed;
	float WindSpeedMixLimit;
	float ChoppyScale;
	float TileScale;
	bool UseFFTSlopes;
};
uniform FFTOceanPatchData u_FFTOceanPatchData;

// Perlin Noise Data
struct PerlinNoiseData
{	
	sampler2D DisplacementMap;
	vec2 Movement;
	vec3 Octaves;
	vec3 Amplitudes;
	vec3 Gradients;
};
uniform PerlinNoiseData u_PerlinNoiseData;

struct WaveBlending 
{
	float Begin;
	float End;
};
uniform WaveBlending u_WaveBlending;

#ifdef BOAT_KELVIN_WAKE
struct BoatKelvinWakeData 
{
	sampler2D DispNormMap;
	sampler2D FoamMap;
	vec3 BoatPosition;
	vec3 WakePosition;
	float Amplitude;
	float FoamAmount;
	float Scale;
};
uniform BoatKelvinWakeData u_BoatKelvinWakeData;
#endif // BOAT_KELVIN_WAKE

in vec2 a_uv;

out vec2 v_scaledUV;
out vec3 v_worldDispPos;
out vec3 v_worldPos;
out float v_blendFactor;
out float v_fogCoord;
out vec4 v_clipPos; //for local reflections + refractions

#ifdef BOAT_KELVIN_WAKE
out vec2 v_wakeBaseUV;
#endif // BOAT_KELVIN_WAKE


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
	// default epsilon = 1e-6f;

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
	if (equalAbs(origin.w, 0.0f, 1e-6f)) origin.w = 1e-6f;
	if (equalAbs(direction.w, 0.0f, 1e-6f)) direction.w = 1e-6f;
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

float calculateWaveDisplacementAttenuation (float d, float dmin, float dmax)
{
	//source:
	// http://stackoverflow.com/questions/33508269/projected-grid-water-horizon-detail
	// http://computergraphics.stackexchange.com/questions/1681/projected-grid-water-horizon-detail

    // Quadratic curve that is 1 at dmin and 0 at dmax
    // Constant 1 for less than dmin, constant 0 for more than dmax

    float att = d > dmax ? 0.0f: clamp(0.0f, 1.0f, (1.0f / ((dmin - dmax) * (dmin - dmax))) * ((d - dmax) * (d - dmax)));

	att = clamp(att, 0.0f, 1.0f);

	return att;
}

vec3 computePerlinDisplacement (vec2 scaledUV)
{
	vec3 perlinDisp = vec3(0.0f);

	vec3 perlinAmplitudes = u_FFTOceanPatchData.WindSpeed * u_PerlinNoiseData.Amplitudes;

	if (v_blendFactor < 1.0f)
	{
		vec2 perlinUV_1 = scaledUV * u_PerlinNoiseData.Octaves.x + u_PerlinNoiseData.Movement;
		vec2 perlinUV_2 = scaledUV * u_PerlinNoiseData.Octaves.y + u_PerlinNoiseData.Movement;
		vec2 perlinUV_3 = scaledUV * u_PerlinNoiseData.Octaves.z + u_PerlinNoiseData.Movement;

		perlinDisp.y += texture(u_PerlinNoiseData.DisplacementMap, perlinUV_1).w * perlinAmplitudes.x;
		perlinDisp.y += texture(u_PerlinNoiseData.DisplacementMap, perlinUV_2).w * perlinAmplitudes.y;
		perlinDisp.y += texture(u_PerlinNoiseData.DisplacementMap, perlinUV_3).w * perlinAmplitudes.z;
	}

	return perlinDisp;
}

vec3 computeFFTDisplacement (vec3 worldPos, vec2 scaledUV)
{
	vec3 fftDisp = vec3(0.0f);

	if (v_blendFactor > 0.0f)
	{
#ifdef USE_GRID_CORNERS_SURFACE
		fftDisp = texture(u_FFTOceanPatchData.FFTWaveDataMap, vec3(scaledUV, 0)).xyz;
#else

		/// working ok with this setup
		// the texture is of size PatchSize x PatchSize
		float offset = 1.0f / u_FFTOceanPatchData.PatchSize;

		vec3 worldPos_dx, worldPos_dy;

		projectPosToWorld(vec2(a_uv.x + offset, a_uv.y), worldPos_dx);
		projectPosToWorld(vec2(a_uv.x, a_uv.y + offset), worldPos_dy);

		vec2 dudx = abs(worldPos_dx.xz - worldPos.xz) * 2.0f / u_FFTOceanPatchData.PatchSize;
		vec2 dudy = abs(worldPos_dy.xz - worldPos.xz) * 2.0f / u_FFTOceanPatchData.PatchSize;

		vec2 scaledDUDX = dudx * u_FFTOceanPatchData.TileScale;
		vec2 scaledDUDY = dudy * u_FFTOceanPatchData.TileScale;

		fftDisp = textureGrad(u_FFTOceanPatchData.FFTWaveDataMap, vec3(scaledUV, 0), scaledDUDX, scaledDUDY).xyz;
		//fftDisp = texture(u_FFTOceanPatchData.FFTWaveDataMap, vec3(scaledUV, 0)).xyz;
#endif // USE_GRID_CORNERS_SURFACE
		fftDisp.xz *= u_FFTOceanPatchData.ChoppyScale; ////
	}

	return fftDisp;
}

float computeBoatKelvinWakeDisplacement (vec3 worldPos)
{
	float bowWakeDisp = 0.0f;

#ifdef BOAT_KELVIN_WAKE // inspired from SunDog Triton Demo ocean shaders
	ivec2 texSize = textureSize(u_BoatKelvinWakeData.DispNormMap, 0);

	// compute the position
	vec3 Pos = worldPos - u_BoatKelvinWakeData.BoatPosition;
	
	vec3 P = u_BoatKelvinWakeData.WakePosition - u_BoatKelvinWakeData.BoatPosition;

	// compute the frame to rotate on
	vec3 Forward = normalize(P);
    vec3 Up = vec3(0.0f, 1.0f, 0.0f);
    vec3 Right = normalize(cross(Up, Forward));

	// compute the UVs
	vec2 wakeBaseUV;
	wakeBaseUV.x = dot(Pos.xz, Right.xz);
	wakeBaseUV.y = dot(Pos.xz, Forward.xz);

	v_wakeBaseUV = wakeBaseUV;

	//////// BOW WAKE - in front of the boat ////////
	// center them to boat position
	vec2 bowWakeDispUV = wakeBaseUV;

	bowWakeDispUV += vec2(texSize.x, texSize.y * 0.4f);
	bowWakeDispUV /= (texSize * u_BoatKelvinWakeData.Scale);

	// alpha channel holds the displacement!
	vec4 data = texture(u_BoatKelvinWakeData.DispNormMap, bowWakeDispUV);

	float wakeDisp = data.w;
	wakeDisp = (wakeDisp < 0.02f ? 0.0f : wakeDisp); // to eliminate artifacts

	bowWakeDisp = wakeDisp * u_BoatKelvinWakeData.Amplitude;
#endif // BOAT_KELVIN_WAKE

	return bowWakeDisp;
}

void main (void)
{
	vec3 worldPos = vec3(0.0f);

#ifdef USE_GRID_CORNERS_SURFACE
	// by computing the grid vertex positions on CPU based on the 4 edges of the grid we avoid interpolation and float precision errors
	vec4 result = mix(mix(u_ProjectingMatrix[0], u_ProjectingMatrix[1], a_uv.x), mix(u_ProjectingMatrix[2], u_ProjectingMatrix[3], a_uv.x), a_uv.y);

	if (result.w < 1e-6f) result.w = 1e-6f;
	float divide = 1.0f / result.w;

	result.xz *= divide;

	worldPos = result.xyz;
#else
	projectPosToWorld(a_uv, worldPos);
#endif // USE_GRID_CORNERS_SURFACE
	
	/////////////////////////////////////////////////////////////////

	// patch size is different from grid size !!!
	vec2 uv = worldPos.xz / u_FFTOceanPatchData.PatchSize;

	// NOTE! uncommenting this code we can see patches
	//uv = max(floor(uv), 1.0f) + fract(uv);

	// when making texture lookups in fragment shader it has access to per-attribute gradients (partial derivates)
	// for the primitive currently being shaded, and it uses this information to determine which LOD to fetch neighboring texels from during filtering.
	// the vertex sahder and geometry shader stages don't have access to such info, so in those stages the LOD can't be determinated correctly automatically/implicitly.
	// To allow this the textureLod() and textureGrad() where introduces. The LOD is sampled from mipmaps. If there is no mipmaps then the base lod (the original texture) will be used.
	// Because hardware is limited it is good to use these functions, even if one new hardware the use of them may not be necesarry, you never know on what hardware the program may be runned on :)
	// dFdx() and dFdy() (partial derivatives) are available only in fragment shader for reasons explained above.
	// https://www.opengl.org/registry/specs/EXT/geometry_shader4.txt
	// http://stackoverflow.com/questions/28983192/why-no-access-to-texture-lod-in-fragment-shader
	// https://rendermeapangolin.wordpress.com/2015/05/27/opengl-texture-lod/
	// https://rendermeapangolin.wordpress.com/2015/05/26/screen-space-grid/

	vec2 scaledUV = uv * u_FFTOceanPatchData.TileScale;

	//////////////////
	float dist = length(u_CameraPosition.xz - worldPos.xz);

	// better and smoother blending !!!
	// make blend factor the same with falloff factor
	//v_blendFactor = calculateWaveDisplacementAttenuation(dist, u_WaveBlending.Begin, u_WaveBlending.End);
	v_blendFactor = clamp((u_WaveBlending.End - dist) / (u_WaveBlending.End - u_WaveBlending.Begin), 0.0f, 1.0f);

	// FOR DEBUGGING
	//v_blendFactor = 1.0f - v_blendFactor; //inverse displacement
	//v_blendFactor = 0.0f; //only perlin noise
	//v_blendFactor = 1.0f; //only fft

	vec3 disp = vec3(0.0f);
	vec3 perlinDisp = vec3(0.0f);
	vec3 fftDisp = vec3(0.0f);

	//////// PERLIN NOISE displacement //////////
	perlinDisp = computePerlinDisplacement(scaledUV);

	///////// FFT displacement ////////////
	fftDisp = computeFFTDisplacement(worldPos, scaledUV);

	//disp = perlinDisp;
	//disp = fftDisp;
	disp = mix(perlinDisp, fftDisp, v_blendFactor);


#ifdef BOAT_KELVIN_WAKE
	disp.y += computeBoatKelvinWakeDisplacement(worldPos);
#endif // BOAT_KELVIN_WAKE

	v_scaledUV = scaledUV;
	v_worldPos = worldPos;
	v_worldDispPos = v_worldPos + disp;

	if (u_CameraPosition.y <= 0.0f)
	{
		vec4 v = u_WorldToCameraMatrix * vec4(v_worldPos, 1.0f);
		v_fogCoord = abs(v.z / v.w);
	}

	////////////////////////////

	gl_Position = v_clipPos = u_WorldToClipMatrix * vec4(v_worldDispPos, 1.0f);
}
