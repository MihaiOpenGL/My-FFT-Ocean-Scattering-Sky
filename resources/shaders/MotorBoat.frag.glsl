/* Author: BAIRAC MIHAI */

uniform float u_HDRExposure;
uniform bool u_ApplyHDR;

uniform sampler2D u_BoatDiffMap;
uniform sampler2D u_BoatNormalMap;

uniform vec3 u_SunDirection;

/// Interpolated inputs across mesh
in vec3 v_normal; ///
in vec2 v_uv;
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
	vec3 boatColor = texture(u_BoatDiffMap, v_uv).rgb;

	vec3 boatNormal = v_normal;

	// TODO - Incorrect lighting when sun is dynamic!!!

	vec3 lightDir = normalize(u_SunDirection);
	vec3 lightColor = vec3(1.0f);

	////// ambient component
	float ambientStrength = 0.2f;
    vec3 ambient = ambientStrength * lightColor;

	////// diffuse component 
	float diff = max(dot(boatNormal, lightDir), 0.0f);
	vec3 diffuse = diff * lightColor;

    vec3 finalColor = (ambient + diffuse) * boatColor;

#ifdef HDR
if (u_ApplyHDR)
{
	finalColor = hdr(finalColor);
}
#endif // HDR

	// the boat surface color must get darker and darker as sun goes away!!!
	finalColor *= computeLightQuantityFactor(lightDir.y);

	fragColor = vec4(finalColor, 1.0f);
}	
