/* Author: BAIRAC MIHAI

The sub surface scattering shader code is based on the nVidia Direct3D SDK11 Island11 sample code
Code: https://developer.nvidia.com/dx11-samples
License: download the package and check the License.pdf file

The perlin noise sampling algorithm is based on the nVidia Direct3D SDK11 OceanCS sample code
Code: https://developer.nvidia.com/dx11-samples
License: download the package and check the License.pdf file

Based on Eric Bruneton's work:

The precomputed scattering algorithms are taken and adapted based on:
Precomputed Atmospheric Scattering effect
Code + paper: http://www-evasion.imag.fr/people/Eric.Bruneton/
License: check the code

*/

uniform float u_HDRExposure;
uniform float u_CrrTime;

uniform sampler2D u_ReflectionMap;
uniform sampler2D u_RefractionMap;
uniform float u_ReflectionDistortFactor;
uniform float u_RefractionDistortFactor;

uniform vec3 u_WaterColor;
uniform vec3 u_WaterRefrColor;
uniform vec3 u_UnderWaterColor;

uniform float u_MaxFadeAltitude;

uniform vec3 u_CameraPosition;

uniform bool u_IsUnderWater;

// Precomputed Sky data used to compute the sun radiance reflection on water
struct PrecomputedScatteringData
{
	sampler2D TransmittanceMap;
	float EarthRadius;
	float SunIntensity;
	float PI;
	vec3 Rgtl;
};
uniform PrecomputedScatteringData u_PrecomputedScatteringData;

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

#ifdef WAVES_FOAM
struct WavesFoamData 
{
	sampler2D Map;
	float ScaleFactor;
};
uniform WavesFoamData u_WavesFoamData;
#endif // WAVES_FOAM

#ifdef WAVES_SSS
struct WavesSSSData 
{
	vec3 Color;
	float Scale;
	float Power;
	float WaveHeightScale;
	float MaxAllowedValue;
};
uniform WavesSSSData u_WavesSSSData;
#endif // WAVES_SSS

#ifdef UNDERWATER_FOG_SURFACE
struct UnderWaterFog 
{
	vec3 Color;
	float Density;
};
uniform UnderWaterFog u_UnderWaterFog;
#endif // UNDERWATER_FOG_SURFACE

// Sun data
struct SunData
{
	vec3 Direction;
};
uniform SunData u_SunData;


#ifdef BOAT_FOAM
struct BoatFoamData 
{
	sampler2D Map;
	float Scale;
};
uniform BoatFoamData u_BoatFoamData;
#endif // BOAT_FOAM

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

#ifdef BOAT_PROPELLER_WASH
struct BoatPropellerWashData 
{
	sampler2D Map;
	float DistortFactor;
};
uniform BoatPropellerWashData u_BoatPropellerWashData;
#endif // BOAT_PROPELLER_WASH



/// Interpolated inputs across mesh
in vec3 v_worldDispPos;
in vec3 v_worldPos;
in vec2 v_scaledUV;
in float v_blendFactor;
in float v_attFactor;
in float v_fogCoord;
in vec4 v_clipPos;

#ifdef BOAT_KELVIN_WAKE
in vec2 v_wakeBaseUV;
#endif // BOAT_KELVIN_WAKE
///

out vec4 fragColor;


/////////////// SKY /////////////////

vec2 getTransmittanceUV(float r, float mu)
{
	vec3 R = u_PrecomputedScatteringData.Rgtl;
    float uR = sqrt((r - R.x) / (R.y - R.x));
    float uMu = atan((mu + 0.15f) / (1.0f + 0.15f) * tan(1.5f)) / 1.5f;

    return vec2(uMu, uR);
}


// transmittance(=transparency) of atmosphere for infinite ray (r,mu)
// (mu=cos(view zenith angle)), intersections with ground ignored
vec3 transmittance(float r, float mu)
{
    vec2 uv = getTransmittanceUV(r, mu);
    return texture2D(u_PrecomputedScatteringData.TransmittanceMap, uv).rgb;
}

// transmittance(=transparency) of atmosphere for infinite ray (r,mu)
// (mu=cos(view zenith angle)), or zero if ray intersects ground
vec3 transmittanceWithShadow(float r, float mu)
{
	vec3 R = u_PrecomputedScatteringData.Rgtl;
    return mu < -sqrt(1.0f - (R.x / r) * (R.x / r)) ? vec3(0.0f) : transmittance(r, mu);
}

// incident sun light at given position (radiance)
// r=length(x)
// muS=dot(x,s) / r
vec3 sunRadiance(float r, float muS)
{
    return transmittanceWithShadow(r, muS) * u_PrecomputedScatteringData.SunIntensity;
}

vec3 computeSunRadiance(vec3 worldP, vec3 worldS)
{
    vec3 worldV = normalize(worldP); // vertical vector
    float r = length(worldP);
    float muS = dot(worldV, worldS);

    return sunRadiance(r, muS);
}

/////////////// OCEAN ////////////////

float meanFresnel(float cosThetaV, float sigmaV) 
{
	return pow(1.0f - cosThetaV, 5.0f * exp(-2.69f * sigmaV)) / (1.0f + 22.7f * pow(sigmaV, 1.5f));
}

// V, N in world space
float meanFresnel(vec3 V, vec3 N, vec2 sigmaSq)
{
    vec2 v = V.xy; // view direction in wind space
    vec2 t = v * v / (1.0f - V.z * V.z); // cos^2 and sin^2 of view direction
    float sigmaV2 = dot(t, sigmaSq); // slope variance in view direction
    return meanFresnel(dot(V, N), sqrt(sigmaV2));
}

void sunRadianceSky(vec3 worldP, vec3 worldS, out vec3 sunL)
{
    vec3 worldV = normalize(worldP); // vertical vector
    float r = length(worldP);
    float muS = dot(worldV, worldS);
    sunL = sunRadiance(r, muS);
}

// assumes x>0
float erfc(float x)
{
	return 2.0f * exp(-x * x) / (2.319f * x + sqrt(4.0f + 1.52f * x * x));
}

float Lambda(float cosTheta, float sigmaSq)
{
	float v = cosTheta / sqrt((1.0 - cosTheta * cosTheta) * (2.0f * sigmaSq));
    return max(0.0, (exp(-v * v) - v * sqrt(u_PrecomputedScatteringData.PI) * erfc(v)) / (2.0f * v * sqrt(u_PrecomputedScatteringData.PI)));
	//return (exp(-v * v)) / (2.0f * v * sqrt(u_PrecomputedScatteringData.PI)); // approximate, faster formula
}

// L, V, N, Tx, Ty in world space
float reflectedSunRadiance(vec3 L, vec3 V, vec3 N, vec3 Tx, vec3 Ty, vec2 sigmaSq)
{
	float reflSunRadiance = 0.0f;

	// because the sky is constructed in a specific way, the lightDir is rotated with pi / 2 radians on OX axis
	// we must rotate it back with - pi / 2 radians

	vec3 RL;
#if 1
	//equivalent with u_SunDirection rotated at - pi/2 radians
	RL = vec3(L.x, L.z, - L.y);
#else
	float sn = sin(- u_PrecomputedScatteringData.PI * 0.5f);
	float cn = cos(- u_PrecomputedScatteringData.PI * 0.5f);

	// rotate vector around OX axis
	RL.x = L.x;
	RL.y = L.y * cn - L.z * sn;
	RL.z = L.y * sn + L.z * cn;
#endif

	////////////////////////////////////
	vec3 H = normalize(RL + V);
	float zetax = dot(H, Tx) / dot(H, N);
	float zetay = dot(H, Ty) / dot(H, N);

	float zL = dot(RL, N); // cos of source zenith angle
	float zV = dot(V, N); // cos of receiver zenith angle
	float zH = dot(H, N); // cos of facet normal zenith angle
	float zH2 = zH * zH;

	float p = exp(-0.5 * (zetax * zetax / sigmaSq.x + zetay * zetay / sigmaSq.y)) / (2.0f * u_PrecomputedScatteringData.PI * sqrt(sigmaSq.x * sigmaSq.y));

	float tanV = atan(dot(V, Ty), dot(V, Tx));
	float cosV2 = 1.0f / (1.0f + tanV * tanV);
	float sigmaV2 = sigmaSq.x * cosV2 + sigmaSq.y * (1.0f - cosV2);

	float tanL = atan(dot(RL, Ty), dot(RL, Tx));
	float cosL2 = 1.0f / (1.0f + tanL * tanL);
	float sigmaL2 = sigmaSq.x * cosL2 + sigmaSq.y * (1.0f - cosL2);

	float fresnel = 0.02f + 0.98f * pow(1.0f - dot(V, H), 5.0f);

	zL = max(zL, 0.01f);
	zV = max(zV, 0.01f);

	reflSunRadiance = fresnel * p / ((1.0f + Lambda(zL, sigmaL2) + Lambda(zV, sigmaV2)) * zV * zH2 * zH2 * 4.0f);

	return reflSunRadiance;
}

/////////////////////////////////////////////////////

vec3 computeSunRefection(vec3 lightDir, vec3 viewDir, vec3 normalDir, float lightQuantity)
{
	vec3 sunReflectionComponent = vec3(0.0f);

	if (lightDir.z > 0.0f && u_CameraPosition.y > 0.0f)
	{
		vec3 Lsun;
		sunRadianceSky(u_CameraPosition + vec3(0.0f, 0.0f, u_PrecomputedScatteringData.EarthRadius), lightDir, Lsun);

		vec2 sigmaSq = vec2(0.02f);
	
		vec3 Ty = normalize(vec3(0.0f, normalDir.z, - normalDir.y));
		vec3 Tx = cross(Ty, normalDir);

		sunReflectionComponent = reflectedSunRadiance(lightDir, viewDir, normalDir, Tx, Ty, sigmaSq) * Lsun * lightQuantity * 0.7f;
	}

	return sunReflectionComponent;
}

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


vec2 computePerlinSlopes (void)
{
	vec2 perlinSlopes = vec2(0.0f);

	vec3 perlinGradients = u_FFTOceanPatchData.WindSpeed * u_PerlinNoiseData.Gradients;

	vec2 perlinUV_1 = v_blendFactor < 1.0f ? v_scaledUV * u_PerlinNoiseData.Octaves.x + u_PerlinNoiseData.Movement : vec2(0.0f);
	vec2 perlinUV_2 = v_blendFactor < 1.0f ? v_scaledUV * u_PerlinNoiseData.Octaves.y + u_PerlinNoiseData.Movement : vec2(0.0f);
	vec2 perlinUV_3 = v_blendFactor < 1.0f ? v_scaledUV * u_PerlinNoiseData.Octaves.z + u_PerlinNoiseData.Movement : vec2(0.0f);

	perlinSlopes += texture(u_PerlinNoiseData.DisplacementMap, perlinUV_1).xy * perlinGradients.x;
	perlinSlopes += texture(u_PerlinNoiseData.DisplacementMap, perlinUV_2).xy * perlinGradients.y;
	perlinSlopes += texture(u_PerlinNoiseData.DisplacementMap, perlinUV_3).xy * perlinGradients.z;

	return perlinSlopes;
}

vec2 computeFFTSlopes (void)
{
	vec2 fftSlopes = vec2(0.0f);

	vec2 fftUV = v_blendFactor > 0.0f ? v_scaledUV : vec2(0.0f);

	if (u_FFTOceanPatchData.UseFFTSlopes)
	{
		fftSlopes = texture(u_FFTOceanPatchData.FFTWaveDataMap, vec3(fftUV, 1)).xy;
	}
	else
	{
		// NOTE! Not so realistic, but ok when computing FFT on CPU !
		fftSlopes = texture(u_FFTOceanPatchData.NormalGradientFoldingMap, fftUV).xy;
	}

	return fftSlopes;
}

vec2 computeBoatKelvinWakeSlopes (void)
{
	vec2 wakeSlopes = vec2(0.0f);

#ifdef BOAT_KELVIN_WAKE
	ivec2 texSize = textureSize(u_BoatKelvinWakeData.DispNormMap, 0);

	// center them to boat position
	vec2 wakeNormUV = v_wakeBaseUV;
	wakeNormUV += vec2(texSize.x, texSize.y * 0.4f);
	wakeNormUV /= (texSize * u_BoatKelvinWakeData.Scale);

	///////////
	vec4 data = texture(u_BoatKelvinWakeData.DispNormMap, wakeNormUV);

	float alpha = (data.a < 0.02f ? 0.0f : data.a); // to eliminate artifacts

	vec3 wakeNorm = data.xyz * alpha;

	// [0, 1] -> [-1, 1]
	wakeNorm = normalize(wakeNorm * 2.0f - 1.0f);


	wakeNorm.xz *= min(1.0f, u_BoatKelvinWakeData.Amplitude);
    wakeNorm = normalize(wakeNorm);

	wakeSlopes = vec2(wakeNorm.x / wakeNorm.y, wakeNorm.z / wakeNorm.y);
#endif // BOAT_KELVIN_WAKE

	return wakeSlopes;
}

vec3 computeUnderWaterFogEffect(vec3 finalColor)
{
	vec3 underWaterFogComponent = vec3(0.0f);

#ifdef UNDERWATER_FOG_SURFACE
	// linear fog
//	const float fogStart = 100.0f;
//	const float fogEnd = 1000.0f;
//	float fogFactor = (fogEnd - v_fogCoord) / (fogEnd - fogStart);

	// exp fog
	float fogFactor = exp(- u_UnderWaterFog.Density * v_fogCoord);

	// exp2 fog
//	const float fogDensity = 0.001f;
//	const float gradient = 2.0f;
//	float fogFactor = exp(- pow(fogDensity * v_fogCoord, gradient));

	fogFactor = 1.0f - clamp(fogFactor, 0.0f, 1.0f);

	underWaterFogComponent = mix(finalColor, u_UnderWaterFog.Color, fogFactor);
#endif // UNDERWATER_FOG_SURFACE

	return underWaterFogComponent;
}

vec3 computeWavesFoamEffect (float fadeAltitude)
{
	vec3 foamComponent = vec3(0.0f);

#ifdef WAVES_FOAM
	float fold = texture(u_FFTOceanPatchData.NormalGradientFoldingMap, v_scaledUV).z;

	float foamFactor = fold * fadeAltitude * v_blendFactor;
	vec3 foamColor = texture(u_WavesFoamData.Map, v_scaledUV * u_WavesFoamData.ScaleFactor).rgb;

	foamComponent = foamColor * foamFactor;
#endif // WAVES_FOAM

	return foamComponent;
}

vec3 computeSubSurfaceScatteringEffect (vec3 normalDir, vec3 lightDir, vec3 viewDir, float fadeAltitude)
{
	vec3 scatterComponent = vec3(0.0f);

#ifdef WAVES_SSS
	if (lightDir.y > 0)
	{
		vec3 incidentDir = - viewDir;

		//// add sub surface scattering effect

		// simulating scattering/double refraction: light hits the side of wave, travels some distance in water, and leaves wave on the other side
		// it's difficult to do it physically correct without photon mapping/ray tracing, so using simple but plausible emulation below

		// only the crests of water waves generate double refracted light
		float scatterFactor = u_WavesSSSData.Scale * max(0.0f, v_worldDispPos.y * u_WavesSSSData.WaveHeightScale) *

		// the waves that lie between camera and light projection on water plane generate maximal amount of double refracted light 
		pow(max(0.0f, dot(lightDir, incidentDir)), u_WavesSSSData.Power) *
	
		// the slopes of waves that are oriented back to light generate maximal amount of double refracted light 
		pow(max(0.0f, 1.0f - dot(lightDir, normalDir)), u_WavesSSSData.Power);

		// water crests gather more light than lobes, so more light is scattered under the crests
		scatterFactor += max(0.0f, v_worldDispPos.y * u_WavesSSSData.WaveHeightScale) *

		// the scattered light is best seen if observing direction is normal to slope surface
		max(0.0f, dot(viewDir, normalDir));

		// also dependent on sun vertical direction
		// if the sun is nearly set the intensity of scattering should be really small
		scatterFactor *= lightDir.y *

		// also make it dependent on fade and blending factors
		fadeAltitude * v_blendFactor;

		scatterComponent = clamp(scatterFactor, 0.0f, u_WavesSSSData.MaxAllowedValue) * u_WavesSSSData.Color * u_WaterColor;
	}
#endif // WAVES_SSS

	return scatterComponent;
}

vec3 computeBoatFoamEffect (void)
{
	vec3 boatFoamComponent = vec3(0.0f);

#ifdef BOAT_FOAM
	ivec2 texSize = textureSize(u_BoatFoamData.Map, 0);

	vec2 boatFoamUV = v_wakeBaseUV;
	boatFoamUV += vec2(texSize.x * 1.0f / u_BoatFoamData.Scale, texSize.y * 0.4f) / u_BoatFoamData.Scale;
	boatFoamUV /= texSize;

	vec4 foamColor = texture(u_BoatFoamData.Map, boatFoamUV * u_BoatFoamData.Scale);

	boatFoamComponent = foamColor.rgb * foamColor.a * 1.0f / u_BoatFoamData.Scale;
#endif // BOAT_FOAM

	return boatFoamComponent;
}

vec3 computeBoatKelvinWakeEffectFoam (vec2 wakeSlope)
{
	vec3 bowWakeComponent = vec3(0.0f);

#ifdef BOAT_KELVIN_WAKE // BOW WAKE
	ivec2 texSize = textureSize(u_BoatKelvinWakeData.FoamMap, 0);

	////////// BOW WAKE FOAM //////////
	// center them to ship position
	vec2 bowWakeFoamUV = v_wakeBaseUV;
	bowWakeFoamUV += vec2(texSize.x, texSize.y * 0.4f) / u_BoatKelvinWakeData.Scale;
	bowWakeFoamUV /= (texSize * u_BoatKelvinWakeData.Scale);

	vec4 bowWakeColor = texture(u_BoatKelvinWakeData.FoamMap, bowWakeFoamUV * u_BoatKelvinWakeData.Scale);
	float bowAlpha = (bowWakeColor.a < 0.02f ? 0.0f : bowWakeColor.a * 2.0f); // to eliminate artifact lines from 'clamp to edge' wrap type

	float bowWakeFactor = u_BoatKelvinWakeData.FoamAmount * length(wakeSlope) * bowAlpha;

	bowWakeComponent = bowWakeFactor * bowWakeColor.rgb;
	bowWakeComponent = min(bowWakeComponent, 1.0f);
#endif // BOAT_KELVIN_WAKE

	return bowWakeComponent;
}

vec3 computeBoatSternWakeEffectFoam (vec2 sternUV)
{
	vec3 washComponent = vec3(0.0f);

#ifdef BOAT_PROPELLER_WASH // STERN WAKE
	vec3 washColor = texture(u_BoatPropellerWashData.Map, sternUV).rgb;

	washComponent = washColor;
#endif // BOAT_PROPELLER_WASH

	return washComponent;
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
	vec3 viewDir = normalize(u_CameraPosition - v_worldDispPos); // from pixel to camera

	// sun is considered a directional light source
	vec3 lightDir = normalize(u_SunData.Direction); // from the sun ???

	vec3 normalDir = vec3(0.0f);
	vec2 slopes = vec2(0.0f);

	vec2 fftSlopes = vec2(0.0f);
	vec2 perlinSlopes = vec2(0.0f);

	/////// PERLIN SLOPE ///////
	perlinSlopes = computePerlinSlopes();

	////// FFT SLOPE ////////
	fftSlopes = computeFFTSlopes();

#ifdef BOAT_KELVIN_WAKE
	vec2 wakeSlopes = computeBoatKelvinWakeSlopes();
#endif // BOAT_KELVIN_WAKE

	slopes = mix(perlinSlopes, fftSlopes, v_blendFactor);

	// Normal
	normalDir = normalize(vec3(slopes.x, 1.0f, slopes.y));

	////////////////////////////////////////////////////
	
	vec3 incidentDir = - viewDir; // from camera to pixel


	// Fresnel approximation
	const float r = (1.2f - 1.0f)/(1.2f + 1.0f);		
	float fresnelFactor = max(0.0f, min(1.0f, r + (1.0f - r) * pow(1.0f - dot(normalDir, viewDir), 5.0f)));

	// coordinates for local reflection and refraction
	vec2 coord = (v_clipPos.xy / v_clipPos.w) * 0.5f + 0.5f;

	vec3 finalColor = vec3(0.0f);

	if (u_IsUnderWater)
	{
		//// refraction
		vec2 refrOffset = normalDir.xz * u_RefractionDistortFactor;
		vec3 refraction = texture(u_RefractionMap, coord + refrOffset).rgb;

		finalColor = mix(refraction, u_UnderWaterColor, 0.7f);

		/////////////// UNDER WATER EFFECTS //////

#ifdef UNDERWATER_FOG_SURFACE
		finalColor = computeUnderWaterFogEffect(finalColor);
#endif // UNDERWATER_FOG_SURFACE

#ifdef HDR
		finalColor = hdr(finalColor);
#endif // HDR
	}
	else
	{	
		//// reflection
		vec2 reflOffset = normalDir.xz * u_ReflectionDistortFactor; 
		vec3 reflection = texture(u_ReflectionMap, coord + reflOffset).rgb;


		// much better visually! - reduced the reflection color by mixing it with another flat color
		float mixFactor = clamp(u_FFTOceanPatchData.WindSpeed / u_FFTOceanPatchData.WindSpeedMixLimit, 0.0f, 1.0f) * fresnelFactor;
		finalColor = mix(u_WaterRefrColor, mix(reflection, u_WaterColor, mixFactor), fresnelFactor);

		float fadeAltitude = (u_MaxFadeAltitude - u_CameraPosition.y) / u_MaxFadeAltitude;
		fadeAltitude = fadeAltitude < 0.0f ? 0.0f : fadeAltitude;

	////////// ABOVE WATER EFFECTS ///////////

#ifdef WAVES_FOAM
		finalColor += computeWavesFoamEffect(fadeAltitude);
#endif // WAVES_FOAM

#ifdef WAVES_SSS
		finalColor += computeSubSurfaceScatteringEffect(normalDir, lightDir, viewDir, fadeAltitude);
#endif // WAVES_SSS

#ifdef BOAT_FOAM
		finalColor += computeBoatFoamEffect();
#endif // BOAT_FOAM

#ifdef BOAT_KELVIN_WAKE
		finalColor += computeBoatKelvinWakeEffectFoam(wakeSlopes);
#endif // BOAT_KELVIN_WAKE

#ifdef BOAT_PROPELLER_WASH // STERN WAKE
		vec2 sternUV = coord + normalDir.xz * u_BoatPropellerWashData.DistortFactor;
	
		finalColor += computeBoatSternWakeEffectFoam(sternUV);
#endif // BOAT_PROPELLER_WASH

#ifdef HDR
		finalColor = hdr(finalColor);
#endif // HDR
	}

	// the ocean surface color must get darker and darker as sun goes away!!!
	float lightQuantity = computeLightQuantityFactor(lightDir.z);

	finalColor *= lightQuantity;

	//// sun reflection on water
	finalColor.rgb += computeSunRefection(lightDir, viewDir, normalDir, lightQuantity);
	/////

	fragColor = vec4(finalColor, 1.0f);
}