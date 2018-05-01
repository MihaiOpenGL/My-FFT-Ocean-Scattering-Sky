/* Author: BAIRAC MIHAI */

uniform sampler2D u_ButterflyMap;
uniform sampler2DArray u_PingPongMap;

uniform float u_Step;

/// Interpolated inputs across mesh 
in vec2 v_uv;
///

out vec4 fragColor0; // DY
out vec4 fragColor1; // DX, DZ

vec2 complex_mult_complex (vec2 c1, vec2 c2)
{
	// (x, yi) * (u, vi) = (xu - yv), (xv + yu)i

	return vec2(c1.x * c2.x - c1.y * c2.y, c1.x * c2.y + c1.y * c2.x);
}

vec4 computeIFFT (vec4 indexAndWeight, float layer)
{
	vec2 sourceA = texture(u_PingPongMap, vec3(indexAndWeight.x, v_uv.y, layer)).xy;
	vec2 sourceB = texture(u_PingPongMap, vec3(indexAndWeight.y, v_uv.y, layer)).xy;

	vec2 weightedB = complex_mult_complex(indexAndWeight.zw, sourceB);

	vec2 complex = sourceA + weightedB;

	return vec4(complex, 0.0f, 0.0f);
}

vec4 compute2IFFT (vec4 indexAndWeight, float layer)
{
	vec4 sourceA = texture(u_PingPongMap, vec3(indexAndWeight.x, v_uv.y, layer));
	vec4 sourceB = texture(u_PingPongMap, vec3(indexAndWeight.y, v_uv.y, layer));

	vec4 weightedB;
	weightedB.xy = complex_mult_complex(indexAndWeight.zw, sourceB.xy);
	weightedB.zw = complex_mult_complex(indexAndWeight.zw, sourceB.zw);

	vec4 complex2 = sourceA + weightedB;

	return complex2;
}

void main (void)
{
	vec4 indexAndWeight = texture(u_ButterflyMap, vec2(v_uv.x, u_Step));

	vec4 dy = computeIFFT(indexAndWeight, 0);
	vec4 dxdz = compute2IFFT(indexAndWeight, 1);

	// gather all the results
	fragColor0 = dy;
	fragColor1 = dxdz;
}	
