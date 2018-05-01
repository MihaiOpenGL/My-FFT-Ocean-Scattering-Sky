/* Author: BAIRAC MIHAI

The sub surface scattering shader code is based on the nVidia Direct3D SDK11 Island11 sample code
Code: https://developer.nvidia.com/dx11-samples
License: download the package and check the License.pdf file

The perlin noise sampling algorithm is based on the nVidia Direct3D SDK11 OceanCS sample code
Code: https://developer.nvidia.com/dx11-samples
License: download the package and check the License.pdf file

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
	float Shininess;
	float Strength;
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

vec3 computeReflectedSunRadiance (vec3 lightDir, vec3 viewDir, vec3 normalDir)
{
	vec3 reflSunComponent = vec3(0.0f);

	if (lightDir.y > 0.0f && u_CameraPosition.y > 0.0f)
	{
		// phong specular - looks unrealistic
		//float cosSpec = max(dot(refl, lightDir), 0.0f);
		//float sunSpot = pow(cosSpec, u_SunData.Shininess) * u_SunData.Strength;

		// blinn-phong specular - looks much better
		vec3 halfwayDir = normalize(lightDir + viewDir);
		float cosSpec = max(dot(normalDir, halfwayDir), 0.0f);
		float sunSpot = pow(cosSpec, u_SunData.Shininess) * u_SunData.Strength;

		float x = lightDir.y;
		float y = max(1.0f - 0.07f / x, 0.0f);
		vec3 sunColor = vec3(1.0f, y + 0.3f, y * y); //mostly white to orange at the end

		reflSunComponent += sunColor * sunSpot;
	}

	return reflSunComponent;
}

vec3 computeUnderWaterFogEffect (vec3 finalColor)
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
	float lightQuantity = computeLightQuantityFactor(lightDir.y);

	finalColor *= lightQuantity;

	if (! u_IsUnderWater)
	{
		finalColor += computeReflectedSunRadiance(lightDir, viewDir, normalDir) * lightQuantity * 0.7f;
	}

	fragColor = vec4(finalColor, 1.0f);
}