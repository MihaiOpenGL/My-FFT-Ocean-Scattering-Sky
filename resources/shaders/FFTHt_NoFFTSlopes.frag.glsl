/* Author: BAIRAC MIHAI */

uniform sampler2D u_FFTInitDataMap;
uniform float u_FFTSize;
uniform float u_Time;

uniform float u_PatchSize;

/// Interpolated inputs across mesh
in vec2 v_uv;
///

out vec4 fragColor0; // DY
out vec4 fragColor1; // DX, DZ

const float PI = 3.141592657f;


vec2 complex_mult_complex (vec2 c1, vec2 c2)
{
	// (x, yi) * (u, vi) = (xu - yv), (xv + yu)i

	return vec2(c1.x * c2.x - c1.y * c2.y, c1.x * c2.y + c1.y * c2.x);
}

vec2 calcHt (vec2 s0, vec2 s0c, float omega)
{
	float sn = sin(omega * u_Time);
	float cs = cos(omega * u_Time);

	vec2 ht = complex_mult_complex(s0, vec2(cs, sn)) + complex_mult_complex(s0c, vec2(cs, - sn));

	return ht;
}

vec4 calcHtDxDz (vec2 ht, vec2 K, float inv_k)
{
	vec4 ht_dxdz;
	ht_dxdz.xy = complex_mult_complex(ht, vec2(0.0f, - K.x * inv_k));
	ht_dxdz.zw = complex_mult_complex(ht, vec2(0.0f, - K.y * inv_k));

	return ht_dxdz;
}

void main (void)
{
	vec2 index = v_uv * u_FFTSize;
	vec2 uv_neg = vec2(1.0f) - v_uv;

	vec2 h0 = texture(u_FFTInitDataMap, v_uv).xy;
	vec2 conH0 = texture(u_FFTInitDataMap, uv_neg).xy;
	float omega = texture(u_FFTInitDataMap, v_uv).z;

	vec2 K = PI * (2.0f * index - u_FFTSize) / u_PatchSize;

	float k = length(K);
	float inv_k = (k == 0.0f ? 0.0f : 1.0f / k);

	vec2 ht = calcHt(h0, conH0, omega);

	fragColor0.xy = ht;
	fragColor1 = calcHtDxDz(ht, K, inv_k);
}	
