/* Author: BAIRAC MIHAI */

uniform float u_HDRExposure;
uniform bool u_ApplyHDR;

uniform samplerCube u_CubeMap;
uniform bool u_IsUnderWater;

struct SunData
{
	float Shininess;
	float Strength;
	float SunFactor;
	vec3 Direction;
};
uniform SunData u_SunData;

#ifdef UNDERWATER_FOG
uniform vec3 u_UnderWaterFogColor;
#endif // UNDERWATER_FOG

uniform vec3 u_UnderWaterColor;	

/// Interpolated inputs across mesh
in vec3 v_uv;
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


vec3 computeSunColor (float sunDirY)
{
	float x = sunDirY;
	float y = max(1.0f - 0.07f / x, 0.0f);
	vec3 sunColor = vec3(1.0f, y + 0.5f, y * y); //mostly white to orange at the end

	return sunColor;
}

float computeLightQuantityFactor (float lightDirY)
{
	// [0, 1] -> [many values near 0, 1]
	// https://www.wolframalpha.com/input/?i=1-0.01%2Fx,+x+%3D+%5B0,+1%5D
	// float y = 1.0f - 0.01f / x;

	// the object color must get darker and darker as sun goes away!!!

	float x = lightDirY;
	float y = x >= 0.0f ? max(1.0f - 0.006f / x, 0.4f) : (0.4f + x);
	
	return y;
}

void main (void)
{
	vec3 finalColor = vec3(0.0f);

	vec3 lightDir = normalize(u_SunData.Direction);

	if (u_IsUnderWater)
	{
#ifdef UNDERWATER_FOG
		finalColor = u_UnderWaterFogColor;
#else
		finalColor = u_UnderWaterColor;
#endif // UNDERWATER_FOG
	}
	else
	{
		vec3 reflDir = normalize(v_uv);

		//  rendering the sun
		float cosSpec = max(dot(reflDir, lightDir), 0.0f);
		float sunSpot = pow(cosSpec, u_SunData.Shininess * u_SunData.SunFactor) * u_SunData.Strength;

		vec3 sunColor = computeSunColor(lightDir.y);

		vec3 sunComponent = sunColor * sunSpot;

		// sky color
		vec3 aboveSkyColor = texture(u_CubeMap, v_uv).rgb;

		finalColor = aboveSkyColor + sunComponent;
	}

#ifdef HDR
if (u_ApplyHDR)
{
	finalColor = hdr(finalColor);
}
#endif // HDR

	// the sky color must get darker and darker as sun goes away!!!
	finalColor *= computeLightQuantityFactor(lightDir.y);

	fragColor = vec4(finalColor, 1.0f);
}	
