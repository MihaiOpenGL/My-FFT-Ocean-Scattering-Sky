/* Author: BAIRAC MIHAI */

layout (binding = 0, rgba16f) uniform image2D u_imageIn; // H0
layout (binding = 1, rgba16f) uniform image2DArray u_imageOut; //Ht

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;


uniform float u_FFTSize;
uniform float u_PatchSize;
uniform float u_Time;

const float PI = 3.141592657f;

vec2 complex_mult_complex (vec2 c1, vec2 c2)
{
	// (x, yi) * (u, vi) = (xu - yv), (xv + yu)i

	return vec2(c1.x * c2.x - c1.y * c2.y, c1.x * c2.y + c1.y * c2.x);
}

vec2 calcHt (vec2 s0, vec2 s0c, float omega)
{
	float sn = sin(omega * u_Time);
	float cs = cos(omega * u_Time);

	vec2 ht = complex_mult_complex(s0, vec2(cs, sn)) + complex_mult_complex(s0c, vec2(cs, - sn));

	return ht;
}

vec4 calcHtDxDz (vec2 ht, vec2 K, float inv_k)
{
	vec4 ht_dxdz;
	ht_dxdz.xy = complex_mult_complex(ht, vec2(0.0f, - K.x * inv_k));
	ht_dxdz.zw = complex_mult_complex(ht, vec2(0.0f, - K.y * inv_k));

	return ht_dxdz;
}

vec4 calcHtSxSz (vec2 ht, vec2 K)
{
	vec4 ht_sxsz;
	ht_sxsz.xy = complex_mult_complex(ht, vec2(0.0f, K.x));
	ht_sxsz.zw = complex_mult_complex(ht, vec2(0.0f, K.y));

	return ht_sxsz;
}

void main (void)
{
	// NOTE! imageLoad() and imageStore() coord are integer values above 0 
	ivec2 storePos = ivec2(gl_GlobalInvocationID);
	ivec2 storePos_neg = ivec2(u_FFTSize - 1 - storePos.x, u_FFTSize - 1 - storePos.y);

	vec2 h0 = imageLoad(u_imageIn, storePos).xy;
	vec2 conH0 = imageLoad(u_imageIn, storePos_neg).xy;
	float omega = imageLoad(u_imageIn, storePos).z;

	vec2 index = vec2(gl_GlobalInvocationID);
	vec2 K = PI * (2.0f * index - u_FFTSize) / u_PatchSize;

	float k = length(K);
	float inv_k = (k == 0.0f ? 0.0f : 1.0f / k);

	vec2 ht = calcHt(h0, conH0, omega);

	vec4 storeData;

	/////// store computed data ///////
	// DY
	storeData.xy = ht;
	imageStore(u_imageOut, ivec3(storePos, 0), storeData);

	// DX, DZ
	storeData = calcHtDxDz(ht, K, inv_k);
	imageStore(u_imageOut, ivec3(storePos, 1), storeData);

	// SX, SZ
	storeData = calcHtSxSz(ht, K);
	imageStore(u_imageOut, ivec3(storePos, 2), storeData);
}	
