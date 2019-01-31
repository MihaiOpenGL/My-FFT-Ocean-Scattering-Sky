/* Author: BAIRAC MIHAI

Based on Eric Bruneton's work:

The precomputed scattering algorithms and glsl functions are taken and adapted based on
Precomputed Atmospheric Scattering effect
Code + paper: http://www-evasion.imag.fr/people/Eric.Bruneton/
License: check the code

*/

struct PrecomputedScatteringData
{
	sampler2D TransmittanceMap;
	sampler2D IrradianceMap;
	sampler3D InscatterMap;
	float EarthRadius;
	float SunIntensity;
	float MieG;
	float PI;
	vec3 Rgtl;
	vec3 BetaR;
};

uniform PrecomputedScatteringData u_PrecomputedScatteringData;

uniform vec3 u_SunDirection;
uniform vec3 u_CameraPosition;

uniform bool u_IsReflMode;

uniform float u_HDRExposure;
uniform bool u_ApplyHDR;

/// Interpolated inputs across mesh 
in vec3 v_viewDir;
in vec2 v_UV;
///

out vec4 fragColor;

///////////////////////////////////////

// ----------------------------------------------------------------------------
// PARAMETERIZATION OPTIONS
// ----------------------------------------------------------------------------

const int RES_R = 32;
const int RES_MU = 128;
const int RES_MU_S = 32;
const int RES_NU = 8;

///////////////// SKY ////////////////////

vec2 getTransmittanceUV (float r, float mu)
{
	vec3 R = u_PrecomputedScatteringData.Rgtl;
    float uR = sqrt((r - R.x) / (R.y - R.x));
    float uMu = atan((mu + 0.15f) / (1.0f + 0.15f) * tan(1.5f)) / 1.5f;

    return vec2(uMu, uR);
}

vec2 getIrradianceUV (float r, float muS)
{
	vec3 R = u_PrecomputedScatteringData.Rgtl;
    float uR = (r - R.x) / (R.y - R.x);
    float uMuS = (muS + 0.2f) / (1.0f + 0.2f);
    return vec2(uMuS, uR);
}

// transmittance(=transparency) of atmosphere for infinite ray (r,mu)
// (mu=cos(view zenith angle)), intersections with ground ignored
vec3 transmittance (float r, float mu)
{
    vec2 uv = getTransmittanceUV(r, mu);

	return texture(u_PrecomputedScatteringData.TransmittanceMap, uv).rgb;
}

vec3 irradiance (float r, float muS)
{
    vec2 uv = getIrradianceUV(r, muS);

	return texture2D(u_PrecomputedScatteringData.IrradianceMap, uv).rgb;
}

// incident sky light at given position, integrated over the hemisphere (irradiance)
// r=length(x)
// muS=dot(x,s) / r
vec3 skyIrradiance (float r, float muS)
{
    return irradiance(r, muS) * u_PrecomputedScatteringData.SunIntensity;
}

// approximated single Mie scattering (cf. approximate Cm in paragraph "Angular precision")
vec3 getMie (vec4 rayMie)
{ // rayMie.rgb=C*, rayMie.w=Cm,r
	vec3 betaR = u_PrecomputedScatteringData.BetaR;
    return rayMie.rgb * rayMie.w / max(rayMie.r, 1e-4f) * (betaR.x / betaR);
}

// Rayleigh phase function
float phaseFunctionR (float mu)
{
    return (3.0f / (16.0f * u_PrecomputedScatteringData.PI)) * (1.0f + mu * mu);
}

// Mie phase function
float phaseFunctionM (float mu)
{
	float mieG2 = u_PrecomputedScatteringData.MieG * u_PrecomputedScatteringData.MieG;
    return 1.5f * 1.0f / (4.0f * u_PrecomputedScatteringData.PI) * (1.0f - mieG2) * pow(1.0f + mieG2 - 2.0f * u_PrecomputedScatteringData.MieG * mu, -3.0f / 2.0f) * (1.0f + mu * mu) / (2.0f + mieG2);
}

vec4 texture4D (sampler3D table, float r, float mu, float muS, float nu)
{
	vec3 R = u_PrecomputedScatteringData.Rgtl;
    float H = sqrt(R.y * R.y - R.x * R.x);
    float rho = sqrt(r * r - R.x * R.x);

    float rmu = r * mu;
    float delta = rmu * rmu - r * r + R.x * R.x;
    vec4 cst = rmu < 0.0f && delta > 0.0f ? vec4(1.0f, 0.0f, 0.0f, 0.5f - 0.5f / float(RES_MU)) : vec4(-1.0f, H * H, H, 0.5f + 0.5f / float(RES_MU));
    float uR = 0.5f / float(RES_R) + rho / H * (1.0f - 1.0f / float(RES_R));
    float uMu = cst.w + (rmu * cst.x + sqrt(delta + cst.y)) / (rho + cst.z) * (0.5f - 1.0f / float(RES_MU));
    // paper formula
    //float uMuS = 0.5f / float(RES_MU_S) + max((1.0f - exp(-3.0f * muS - 0.6f)) / (1.0f - exp(-3.6f)), 0.0f) * (1.0f - 1.0f / float(RES_MU_S));
    // better formula
    float uMuS = 0.5f / float(RES_MU_S) + (atan(max(muS, -0.1975f) * tan(1.26f * 1.1f)) / 1.1f + (1.0f - 0.26f)) * 0.5f * (1.0f - 1.0f / float(RES_MU_S));

    float lerp = (nu + 1.0f) / 2.0f * (float(RES_NU) - 1.0f);
    float uNu = floor(lerp);
    lerp = lerp - uNu;

	return texture(table, vec3((uNu + uMuS) / float(RES_NU), uMu, uR)) * (1.0f - lerp) +
           texture(table, vec3((uNu + uMuS + 1.0f) / float(RES_NU), uMu, uR)) * lerp;
}

// scattered sunlight between two points
// camera=observer
// viewdir=unit vector towards observed point
// sundir=unit vector towards the sun
// return scattered light and extinction coefficient
vec3 skyRadiance (vec3 camera, vec3 viewdir, vec3 sundir, out vec3 extinction)
{
    vec3 result;
    float r = length(camera);
    float rMu = dot(camera, viewdir);
    float mu = rMu / r;
    float r0 = r;
    float mu0 = mu;

	vec3 R = u_PrecomputedScatteringData.Rgtl;
    float deltaSq = sqrt(rMu * rMu - r * r + R.y * R.y);
    float din = max(-rMu - deltaSq, 0.0f);

    if (din > 0.0f)
	{
        camera += din * viewdir;
        rMu += din;
        mu = rMu / R.y;
        r = R.y;
    }

    if (r <= R.y)
	{
        float nu = dot(viewdir, sundir);
        float muS = dot(camera, sundir) / r;

        vec4 inScatter = texture4D(u_PrecomputedScatteringData.InscatterMap, r, rMu / r, muS, nu);
        extinction = transmittance(r, mu);

        vec3 inScatterM = getMie(inScatter);
        float phase = phaseFunctionR(nu);
        float phaseM = phaseFunctionM(nu);
        result = inScatter.rgb * phase + inScatterM * phaseM;
    }
	else {
        result = vec3(0.0f);
        extinction = vec3(1.0f);
    }

    return result * u_PrecomputedScatteringData.SunIntensity;
}

////////////////

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

///////////////////////////////

void main (void)
{
	vec3 viewDir = normalize(v_viewDir);
	vec3 lightDir = normalize(u_SunDirection);

	vec3 finalColor = vec3(0.0f);

	vec3 sunColor = vec3(step(cos(u_PrecomputedScatteringData.PI / 180.0f), dot(viewDir, lightDir))) * u_PrecomputedScatteringData.SunIntensity;

	if (u_IsReflMode) // reflecion texture
	{
		vec2 u = v_UV;
		float l = dot(u, u);
		if (l <= 1.02f)
		{
			if (l > 1.0f)
			{
				u = u / l;
				l = 1.0f / l;
			}

			// inverse stereographic projection,
			// from skymap coordinates to world space directions
			vec3 r = vec3(2.0f * u, 1.0f - l) / (1.0f + l);

			vec3 extinction;
			vec3 inscatter = skyRadiance(vec3(0.0f, 0.0f, u_PrecomputedScatteringData.EarthRadius), r, lightDir, extinction);

			finalColor = inscatter + sunColor * extinction;
		}
		else
		{
			// below horizon:
			// use average fresnel * average sky radiance
			// to simulate multiple reflections on waves

			const float avgFresnel = 0.17f;
			finalColor = skyIrradiance(u_PrecomputedScatteringData.EarthRadius, lightDir.z) / u_PrecomputedScatteringData.PI * avgFresnel;
		}
	}
	else // normal sky
	{
		vec3 extinction;
		vec3 inscatter = skyRadiance(vec3(0.0f, 0.0f, u_PrecomputedScatteringData.EarthRadius), viewDir, lightDir, extinction);

		finalColor = inscatter + sunColor * extinction;
	}

	// this hdr is a must, it will not work without it !!!
    fragColor.rgb = hdr(finalColor);
	fragColor.a = 1.0f;
}	
