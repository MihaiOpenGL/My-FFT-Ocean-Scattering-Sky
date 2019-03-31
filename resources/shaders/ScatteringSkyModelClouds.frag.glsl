/* Author: BAIRAC MIHAI

Clouds generated using simplex noise algorithm
Simplex noise code taken from: 
https://github.com/ashima/webgl-noise/blob/master/src/noise2D.glsl
License: MIT License


Fractal code to sum up octaves taken from:
https://github.com/Auburns/FastNoise/blob/master/FastNoise.cpp
License: MIT License

*/

uniform float u_HDRExposure;
uniform bool u_ApplyHDR;

uniform float u_SunDirY;

struct CloudsData
{
	int Octaves;
	float Lacunarity;
	float Gain;
	float ScaleFactor;
};
uniform CloudsData u_CloudsData;

/// Interpolated inputs across mesh
in vec2 v_uv;
///

out vec4 fragColor;


////////////////////////////////////////////

vec3 mod289 (vec3 x)
{
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec2 mod289 (vec2 x)
{
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec3 permute (vec3 x)
{
  return mod289(((x*34.0)+1.0)*x);
}

float snoise (vec2 v)
{
  const vec4 C = vec4(0.211324865405187,  // (3.0-sqrt(3.0))/6.0
                      0.366025403784439,  // 0.5*(sqrt(3.0)-1.0)
                     -0.577350269189626,  // -1.0 + 2.0 * C.x
                      0.024390243902439); // 1.0 / 41.0
// First corner
  vec2 i  = floor(v + dot(v, C.yy) );
  vec2 x0 = v -   i + dot(i, C.xx);

// Other corners
  vec2 i1;
  //i1.x = step( x0.y, x0.x ); // x0.x > x0.y ? 1.0 : 0.0
  //i1.y = 1.0 - i1.x;
  i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
  // x0 = x0 - 0.0 + 0.0 * C.xx ;
  // x1 = x0 - i1 + 1.0 * C.xx ;
  // x2 = x0 - 1.0 + 2.0 * C.xx ;
  vec4 x12 = x0.xyxy + C.xxzz;
  x12.xy -= i1;

// Permutations
  i = mod289(i); // Avoid truncation effects in permutation
  vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))
		+ i.x + vec3(0.0, i1.x, 1.0 ));

  vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy), dot(x12.zw,x12.zw)), 0.0);
  m = m*m ;
  m = m*m ;

// Gradients: 41 points uniformly over a line, mapped onto a diamond.
// The ring size 17*17 = 289 is close to a multiple of 41 (41*7 = 287)

  vec3 x = 2.0 * fract(p * C.www) - 1.0;
  vec3 h = abs(x) - 0.5;
  vec3 ox = floor(x + 0.5);
  vec3 a0 = x - ox;

// Normalise gradients implicitly by scaling m
// Approximation of: m *= inversesqrt( a0*a0 + h*h );
  m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );

// Compute final noise value at P
  vec3 g;
  g.x  = a0.x  * x0.x  + h.x  * x0.y;
  g.yz = a0.yz * x12.xz + h.yz * x12.yw;
  return 130.0 * dot(m, g);
}

//////////////////////////

float fractalFBM (vec2 uv)
{
	float sum = snoise(uv);
	float amp = 1.0f;
	float ampFractal = 1.0f;

	for (int i = 0; i < u_CloudsData.Octaves; ++ i)
	{
		uv *= u_CloudsData.Lacunarity;
		ampFractal += amp;
		amp *= u_CloudsData.Gain;

		sum  += snoise(uv) * amp;
	}

	sum /= ampFractal;

	return sum;
}

float fractalBillow (vec2 uv)
{
	float sum = abs(snoise(uv)) * 2.0f - 1.0f;
	float amp = 1.0f;
	float ampFractal = 1.0f;

	for (int i = 0; i < u_CloudsData.Octaves; ++ i)
	{
		uv *= u_CloudsData.Lacunarity;
		ampFractal += amp;
		amp *= u_CloudsData.Gain;

		sum  += (abs(snoise(uv)) * 2.0f - 1.0f) * amp;
	}

	sum /= ampFractal;

	return sum;
}

float fractalRigidMulti (vec2 uv)
{
	float sum = 1.0f - abs(snoise(uv));
	float amp = 1.0f;
	float ampFractal = 1.0f;

	for (int i = 0; i < u_CloudsData.Octaves; ++ i)
	{
		uv *= u_CloudsData.Lacunarity;
		ampFractal += amp;
		amp *= u_CloudsData.Gain;

		sum  += (1.0f - abs(snoise(uv))) * amp;
	}

	sum /= ampFractal;

	return sum;
}
////////////////////////////////////////////

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


void main (void)
{
	vec3 finalColor = vec3(0.0f);

	// the clouds surface color must get darker and darker as sun goes away!!!
	float lightQuantity = computeLightQuantityFactor(u_SunDirY);

	float value = fractalFBM(v_uv * u_CloudsData.ScaleFactor);
	//float value = fractalBillow(v_uv);
	//float value = fractalRigidMulti(v_uv);

	finalColor = vec3((value + 0.3f) * lightQuantity);

	// to better observe the clouds
	//finalColor *= vec3(0.0f, 1.0f, 0.0f);//DEBUG

#ifdef HDR
	if (u_ApplyHDR)
	{
		finalColor = hdr(finalColor);
	}
#endif // HDR

	finalColor *= lightQuantity;

	fragColor = vec4(finalColor, value * 3.0f);
}	
