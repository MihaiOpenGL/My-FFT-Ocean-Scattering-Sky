/* Author: BAIRAC MIHAI

 Caustics code uses a small portion of the below work
 code: http://madebyevan.com/webgl-water/renderer.js
 paper: https://medium.com/@evanwallace/rendering-realtime-caustics-in-webgl-2a99a29a0b2c

 License: MIT License

*/

#ifdef UNDERWATER_CAUSTICS
struct CausticsData
{
	vec3 Color;
	float Intensity;
};
uniform CausticsData u_CausticsData;
#endif // UNDERWATER_CAUSTICS

in vec3 v_oldPos;
in vec3 v_newPos;

out vec4 fragColor;

// NOTE! No LDR to HDR conversion for caustics, because the texture will be used in ocean bottom shader were we already have HDR applied!

void main (void)
{
	vec3 finalColor = vec3(0.0f);
	
#ifdef UNDERWATER_CAUSTICS
	float oldArea = length(dFdx(v_oldPos)) * length(dFdy(v_oldPos));
    float newArea = length(dFdx(v_newPos)) * length(dFdy(v_newPos));

	float causticsFactor = oldArea / newArea;

    finalColor = u_CausticsData.Color * causticsFactor * u_CausticsData.Intensity;
#endif // UNDERWATER_CAUSTICS

	fragColor = vec4(finalColor, 1.0f);
}	
