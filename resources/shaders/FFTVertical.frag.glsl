/* Author: BAIRAC MIHAI */

uniform sampler2D u_ButterflyMap;
uniform sampler2DArray u_PingPongMap; // 2 complex inputs (= 4 values) per layer

uniform float u_FFTSize;
uniform float u_Step;
uniform bool u_IsLastStep;

/// Interpolated inputs across mesh
in vec2 v_uv;
///

out vec4 fragColor0; // DY
out vec4 fragColor1; // DX, DZ
out vec4 fragColor2; // SX, SZ

vec2 complex_mult_complex (vec2 c1, vec2 c2)
{
	// (x, yi) * (u, vi) = (xu - yv), (xv + yu)i

	return vec2(c1.x * c2.x - c1.y * c2.y, c1.x * c2.y + c1.y * c2.x);
}

vec4 computeIFFT (vec4 indexAndWeight, float layer)
{
	vec2 sourceA = texture(u_PingPongMap, vec3(v_uv.x, indexAndWeight.x, layer)).xy;
	vec2 sourceB = texture(u_PingPongMap, vec3(v_uv.x, indexAndWeight.y, layer)).xy;

	vec2 weightedB = complex_mult_complex(indexAndWeight.zw, sourceB);

	vec2 complex = sourceA + weightedB;

	return vec4(complex, 0.0f, 0.0f);
}

vec4 compute2IFFT (vec4 indexAndWeight, float layer)
{
	vec4 sourceA = texture(u_PingPongMap, vec3(v_uv.x, indexAndWeight.x, layer));
	vec4 sourceB = texture(u_PingPongMap, vec3(v_uv.x, indexAndWeight.y, layer));

	vec4 weightedB;
	weightedB.xy = complex_mult_complex(indexAndWeight.zw, sourceB.xy);
	weightedB.zw = complex_mult_complex(indexAndWeight.zw, sourceB.zw);

	vec4 complex2 = sourceA + weightedB;

	return complex2;
}

void main (void)
{
	vec4 indexAndWeight = texture(u_ButterflyMap, vec2(v_uv.y, u_Step));

	vec4 dy = computeIFFT(indexAndWeight, 0);
	vec4 dxdz = compute2IFFT(indexAndWeight, 1);
	vec4 sxsz = compute2IFFT(indexAndWeight, 2);

	if (u_IsLastStep)
	{
		// the texture is of size u_FFTSize x u_FFTSize
		vec2 index = v_uv * u_FFTSize;
		int sign_correction = (mod((index.x + index.y), 2.0f) == 1.0f) ? -1 : 1;

		vec3 displacement = vec3( - dxdz.x, dy.x, - dxdz.z) * sign_correction;
		vec2 slopes = vec2( - sxsz.x, - sxsz.z) * sign_correction;

		fragColor0 = vec4(displacement, 0.0f);
		fragColor1 = vec4(slopes, 0.0f, 0.0f);
	}
	else
	{
		fragColor0 = dy;
		fragColor1 = dxdz;
		fragColor2 = sxsz;
	}
}	