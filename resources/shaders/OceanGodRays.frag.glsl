/* Author: BAIRAC MIHAI 

God Rays code is based on:
code + paper: http://fabiensanglard.net/lightScattering/
which is based on: https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch13.html

License: check the package

*/


#ifdef UNDERWATER_GODRAYS
struct GodRaysData 
{
	int NumberOfSamples;
	sampler2D OccluderMap;
	float Exposure;
	float Decay;
	float Density;
	float Weight;
	vec2 LightDirectionOnScreen;
};
uniform GodRaysData u_GodRaysData;
#endif // UNDERWATER_GODRAYS

in vec2 v_uv;

out vec4 fragColor;


void main (void)
{
	vec3 finalColor = vec3(0.0f);

#ifdef UNDERWATER_GODRAYS
	vec2 deltaTextCoord = vec2(v_uv - u_GodRaysData.LightDirectionOnScreen);
	deltaTextCoord *= 1.0f /  float(u_GodRaysData.NumberOfSamples) * u_GodRaysData.Density;
	float illuminationDecay = 1.0f;
	
	vec2 uv = v_uv;
	for (int i = 0; i < u_GodRaysData.NumberOfSamples ; i++)
	{
			uv -= deltaTextCoord;
			vec3 texel = texture(u_GodRaysData.OccluderMap, uv).rgb;
			
			texel *= illuminationDecay * u_GodRaysData.Weight;
			
			finalColor += texel;
			
			illuminationDecay *= u_GodRaysData.Decay;
	}
	
	finalColor *= u_GodRaysData.Exposure;
#endif // UNDERWATER_GODRAYS

	// No need to HDR this because we blend this effect with the hdr corrected color that is already in framebuffer!

	fragColor = vec4(finalColor, 1.0f);
}	
