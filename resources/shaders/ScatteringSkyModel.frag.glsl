/* Author: BAIRAC MIHAI

Atmospheric scattring code based and adapted from
Mie and Rayleight code based on this work
code: https://www.gamedev.net/forums/topic/461747-atmospheric-scattering-sean-oneill---gpu-gems2/
paper: https://developer.nvidia.com/gpugems/GPUGems2/gpugems2_chapter16.html

License: GameDev.net Open License

*/

uniform float u_HDRExposure;
uniform bool u_ApplyHDR;
uniform bool u_IsReflMode;
uniform bool u_IsUnderWater;

#ifdef UNDERWATER_FOG
uniform vec3 u_UnderWaterFogColor;
#endif // UNDERWATER_FOG

uniform vec3 u_UnderWaterColor;

uniform vec3 u_SunDirection;
uniform float u_g;
uniform float u_yOffset;

/// Interpolated inputs across mesh
in float v_fragY;
in vec3 v_RayleighColor;
in vec3 v_MieColor;
in vec3 v_Direction;
///

out vec4 fragColor;


#ifdef HDR
// Exposure tone mapping + gamma correction
vec3 hdr (vec3 L) 
{
    L = L * u_HDRExposure;
    L.r = L.r < 1.413f ? pow(L.r * 0.38317f, 1.0f / 2.2f) : 1.0f - exp(-L.r);
    L.g = L.g < 1.413f ? pow(L.g * 0.38317f, 1.0f / 2.2f) : 1.0f - exp(-L.g);
    L.b = L.b < 1.413f ? pow(L.b * 0.38317f, 1.0f / 2.2f) : 1.0f - exp(-L.b);
    return L;
}
#endif // HDR

float computeRayleighPhase (float cos_angle2)
{
	float rayleighPhase = 0.75f * (1.0f + cos_angle2);

	return rayleighPhase;
}

float computeMiePhase (float cos_angle)
{
	float g2 = u_g * u_g;

	float miePhase = 0.2f * ((1.0f - g2) / (2.0f + g2)) * (1.0f + cos_angle * cos_angle) / pow(1.0f + g2 - 2.0f * u_g * cos_angle, 1.5f);

	return miePhase;
}

vec3 computeSkySunColor (void)
{
	float cos_angle = dot(u_SunDirection, v_Direction) / length(v_Direction);
	float cos_angle2 = cos_angle * cos_angle;

	float rayleighPhase = computeRayleighPhase(cos_angle2);
	float miePhase = computeMiePhase(cos_angle);

	vec3 skySunColor = u_IsReflMode ? 0.5f * v_RayleighColor : rayleighPhase * v_RayleighColor + miePhase * v_MieColor;

	return skySunColor;
}


void main (void)
{
	vec3 finalColor = computeSkySunColor();

	if (v_fragY <= u_yOffset)
	{
#ifdef UNDERWATER_FOG
		finalColor = u_UnderWaterFogColor;
#else
		finalColor = u_UnderWaterColor;
#endif // UNDERWATER_FOG
	}

#ifdef HDR
if (u_ApplyHDR)
{
	finalColor = hdr(finalColor);
}
#endif // HDR

	fragColor = vec4(finalColor, 1.0f);
}	
