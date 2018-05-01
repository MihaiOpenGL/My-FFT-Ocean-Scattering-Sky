/* Author: BAIRAC MIHAI

The gradient and Jacobian computation is based on the nVidia Direct3D SDK11 OceanCS sample code
Code: https://developer.nvidia.com/dx11-samples
License: download the package and check the License.pdf file

*/

layout (binding = 0, rgba16f) uniform image2DArray u_imageFFTIn; 
layout (binding = 1, rgba16f) uniform image2D u_imageFFTOut;

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

uniform int u_FFTSize;
uniform float u_ChoppyScale;
uniform float u_CoverageFactor;


vec3 loadData (ivec2 pos)
{
	ivec2 cpos = clamp(pos, 0, u_FFTSize - 1);

	return imageLoad(u_imageFFTIn, ivec3(cpos, 0)).xyz;
}

void storeData (ivec2 storePos, vec3 data)
{
	imageStore(u_imageFFTOut, storePos, vec4(data, 0.0f));
}

vec3 computeGradientJacobian (ivec2 storePos)
{
	// Sample neighbour texels
	const int offset = 1;

	ivec2 left = ivec2(storePos.x - offset, storePos.y);
	ivec2 right = ivec2(storePos.x + offset, storePos.y);
	ivec2 back = ivec2(storePos.x, storePos.y - offset);
	ivec2 front = ivec2(storePos.x, storePos.y + offset);

	vec3 displace_left = loadData(left);
	vec3 displace_right = loadData(right);
	vec3 displace_back = loadData(back);
	vec3 displace_front = loadData(front);
	
	// Do not store the actual normal value. Using gradient instead, which preserves two differential values.
	vec2 gradient = vec2(-(displace_right.y - displace_left.y), -(displace_front.y - displace_back.y));

	// Ecuation (30) from Jerry Tessendorf's paper

	// Calculate Jacobian correlation from the partial differential of height field
	vec2 Dx = (displace_right.xz - displace_left.xz) * u_ChoppyScale;
	vec2 Dz = (displace_front.xz - displace_back.xz) * u_ChoppyScale;
	float J = (1.0f + Dx.x) * (1.0f + Dz.y) - Dx.y * Dz.x;

	return vec3(gradient, J);
}

float computeFoldFactor (float J)
{
	// Practical subsurface scale calculation: max[0, (1 - J) + Amplitude * (2 * Coverage - 1)].
	//float fold = max(0.0f, (1.0f - J) + wave_amplitude * (2.0f * coverage - 1.0f));

	// if coverage is smaller than 1.0f then it's bigger, if it's bigger than 1.0f, it's smaller
	float fold = max(0.0f, 1.0f - J * u_CoverageFactor);

	return fold;
}

void main (void)
{
	ivec2 storePos = ivec2(gl_GlobalInvocationID);

	vec3 gradient_J = computeGradientJacobian(storePos);

	float fold = computeFoldFactor(gradient_J.z);

	// store computed data
	storeData(storePos, vec3(gradient_J.xy, fold));
}	
