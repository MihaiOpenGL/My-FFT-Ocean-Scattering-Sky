/* Author: BAIRAC MIHAI 

Filtering functions:
bilinearFilteringCaustics()
bicubicTriangularFilteringCaustics()

are based on code from http://www.codeproject.com/Articles/236394/Bi-Cubic-and-Bi-Linear-Interpolation-with-GLSL
License: Code Project Open License

*/

uniform float u_HDRExposure;

uniform sampler2D u_PerlinDisplacementMap;

struct SandData
{
	sampler2D DiffuseMap;
	float Scale;
};
uniform SandData u_SandData;

uniform vec3 u_SunDirection;

uniform float u_TileScale;

#ifdef UNDERWATER_CAUSTICS
struct CausticsData
{	
	sampler2D Map;
	float Scale;
};
uniform CausticsData u_CausticsData;
#endif // UNDERWATER_CAUSTICS

#ifdef UNDERWATER_FOG_BOTTOM
struct FogData 
{
	vec3 Color;
	float Density;
};
uniform FogData u_FogData;
#endif // UNDERWATER_FOG_BOTTOM

in vec2 v_baseUV;
in vec2 v_causUV;
in float v_fogCoord;


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

vec3 bilinearFilteringCaustics (vec2 uv)
{
	vec3 causticsComponent = vec3(0.0f);

#ifdef UNDERWATER_CAUSTICS
	int texSize = textureSize(u_CausticsData.Map, 0).x;
	float offset = 1.0f / texSize;

	vec3 p0q0 = texture(u_CausticsData.Map, uv).rgb;
    vec3 p1q0 = texture(u_CausticsData.Map, uv + vec2(offset, 0)).rgb;

    vec3 p0q1 = texture(u_CausticsData.Map, uv + vec2(0, offset)).rgb;
    vec3 p1q1 = texture(u_CausticsData.Map, uv + vec2(offset , offset)).rgb;

    float a = fract( uv.x * texSize ); // Get Interpolation factor for X direction.
					// Fraction near to valid data.

    vec3 pInterp_q0 = mix( p0q0, p1q0, a ); // Interpolates top row in X direction.
    vec3 pInterp_q1 = mix( p0q1, p1q1, a ); // Interpolates bottom row in X direction.

    float b = fract( uv.y * texSize );// Get Interpolation factor for Y direction.

    causticsComponent =  mix( pInterp_q0, pInterp_q1, b ); // Interpolate in Y direction.
#endif // UNDERWATER_CAUSTICS

	return causticsComponent;
}

float Triangular (float f)
{
	f = f / 2.0f;
	if (f < 0.0f)
	{
		return ( f + 1.0f );
	}
	else
	{
		return ( 1.0f - f );
	}
	return 0.0f;
}

vec3 bicubicTriangularFilteringCaustics (vec2 uv)
{
vec3 causticsComponent = vec3(0.0f);

#ifdef UNDERWATER_CAUSTICS
	int texSize = textureSize(u_CausticsData.Map, 0).x;
	float offset = 1.0f / texSize;

    vec3 nSum = vec3( 0.0f );
    vec3 nDenom = vec3( 0.0f );
    float a = fract( uv.x * texSize ); // get the decimal part
    float b = fract( uv.y * texSize ); // get the decimal part

    for( int m = -1; m <=2; m++ )
    {
        for( int n =-1; n<= 2; n++)
        {
			vec3 vecData = texture(u_CausticsData.Map, uv + vec2(offset * float( m ), offset * float( n ))).rgb;
			float f  = Triangular( float( m ) - a );
			vec3 vecCooef1 = vec3( f,f,f );
			float f1 = Triangular( -( float( n ) - b ) );
			vec3 vecCoeef2 = vec3( f1, f1, f1 );
            nSum = nSum + ( vecData * vecCoeef2 * vecCooef1 );
            nDenom = nDenom + (( vecCoeef2 * vecCooef1 ));
        }
    }

    causticsComponent = nSum / nDenom;
#endif // UNDERWATER_CAUSTICS

	return causticsComponent;
}

vec3 computeUnderWaterCausticsEffect (void)
{
	vec3 underWaterCausticsComponent = vec3(0.0f);

#ifdef UNDERWATER_CAUSTICS
//	underWaterCausticsComponent = texture(u_CausticsData.Map, v_causUV * u_CausticsData.Scale * u_TileScale).rgb;
//	underWaterCausticsComponent = bicubicTriangularFilteringCaustics(v_causUV * u_CausticsData.Scale * u_TileScale);

	underWaterCausticsComponent = bilinearFilteringCaustics(v_causUV * u_CausticsData.Scale * u_TileScale);	
#endif // UNDERWATER_CAUSTICS

	return underWaterCausticsComponent;
}

vec3 computeUnderWaterFogEffect (vec3 finalColor)
{
	vec3 underWaterFogComponent = vec3(0.0f);

#ifdef UNDERWATER_FOG_BOTTOM
	// exp fog
	float fogFactor = exp(- u_FogData.Density * v_fogCoord);
	fogFactor = 1.0f - clamp(fogFactor, 0.0f, 1.0f);

	underWaterFogComponent = mix(finalColor, u_FogData.Color, fogFactor);
#endif // UNDERWATER_FOG_BOTTOM

	return underWaterFogComponent;
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
	vec3 sandColor = texture (u_SandData.DiffuseMap, v_baseUV * u_SandData.Scale).rgb;

	// better normals when having displacement!
	vec2 slopes = texture(u_PerlinDisplacementMap, v_baseUV).xy;
	vec3 sandNormal = vec3(slopes.x, 1.0f, slopes.y);

	vec3 lightDir = normalize(u_SunDirection); 

	vec3 lightColor = vec3(1.0f);

	// ambient component - approximation
	float ambientStrength = 0.01f;
    vec3 ambient = ambientStrength * lightColor;

	////// diffuse component
	float diff = max(dot(sandNormal, lightDir), 0.0f);
	vec3 diffuse = diff * lightColor;

	// final color
    vec3 finalColor = (ambient + diffuse) * sandColor;


#ifdef UNDERWATER_CAUSTICS
	finalColor += computeUnderWaterCausticsEffect();
#endif // UNDERWATER_CAUSTICS

#ifdef UNDERWATER_FOG_BOTTOM
	finalColor = computeUnderWaterFogEffect(finalColor);
#endif // UNDERWATER_FOG_BOTTOM

#ifdef HDR
	finalColor = hdr(finalColor);
#endif // HDR

	// the underwater ocean bottom color must get darker and darker as sun goes away!!!
	finalColor *= computeLightQuantityFactor(lightDir.y);

	fragColor = vec4(finalColor, 1.0f);
}