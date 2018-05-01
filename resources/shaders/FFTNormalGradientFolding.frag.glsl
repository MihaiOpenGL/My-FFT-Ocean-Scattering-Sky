/* Author: BAIRAC MIHAI

The gradient and Jacobian computation is based on the nVidia Direct3D SDK11 OceanCS sample code
Code: https://developer.nvidia.com/dx11-samples
License: download the package and check the License.pdf file

*/

uniform sampler2DArray u_FFTWaveDataMap;

uniform float u_FFTSize;
uniform float u_ChoppyScale;
uniform float u_CoverageFactor;

/// Interpolated inputs across mesh 
in vec2 v_uv;
///

out vec4 fragColor;

vec2 computeGradientJacobian (out float J)
{
	// Sample neighbour texels
	float offset = 1.0f / u_FFTSize;

	vec2 left = vec2(v_uv.x - offset, v_uv.y);
	vec2 right = vec2(v_uv.x + offset, v_uv.y);
	vec2 back = vec2(v_uv.x, v_uv.y - offset);
	vec2 front = vec2(v_uv.x, v_uv.y + offset);

	vec3 displace_left = texture(u_FFTWaveDataMap, vec3(left, 0)).xyz;
	vec3 displace_right = texture(u_FFTWaveDataMap, vec3(right, 0)).xyz;
	vec3 displace_back = texture(u_FFTWaveDataMap, vec3(back, 0)).xyz;
	vec3 displace_front = texture(u_FFTWaveDataMap, vec3(front, 0)).xyz;
	
	// Do not store the actual normal value. Using gradient instead, which preserves two differential values.
	vec2 gradient = vec2(-(displace_right.y - displace_left.y), -(displace_front.y - displace_back.y));

	// Ecuation (30) from Jerry Tessendorf's paper

	// Calculate Jacobian corelation from the partial differential of height field
	vec2 Dx = (displace_right.xz - displace_left.xz) * u_ChoppyScale;
	vec2 Dz = (displace_front.xz - displace_back.xz) * u_ChoppyScale;
	J = (1.0f + Dx.x) * (1.0f + Dz.y) - Dx.y * Dz.x;

	return gradient;
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
	float J = 0.0f;
	vec2 gradient = computeGradientJacobian(J);

	float fold = computeFoldFactor(J);

	fragColor = vec4(gradient, fold, 0.0f);
}	
