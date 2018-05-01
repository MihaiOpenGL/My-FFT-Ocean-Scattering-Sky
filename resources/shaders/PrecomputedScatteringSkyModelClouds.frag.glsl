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

uniform vec3 u_CameraPosition;
uniform vec3 u_SunDirection;

struct CloudsData 
{
	sampler2D NoiseMap;
	int Octaves;
	float Lacunarity;
	float Gain;
	float Norm;
	float Clamp1;
	float Clamp2;
	float Height;
	vec3 Color;
};
uniform CloudsData u_CloudsData;

uniform float u_HDRExposure;

/// Interpolated inputs across mesh 
in vec3 v_worldPos;
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

/////////////// SKY /////////////////

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
    return texture2D(u_PrecomputedScatteringData.TransmittanceMap, uv).rgb;
}

// transmittance(=transparency) of atmosphere for infinite ray (r,mu)
// (mu=cos(view zenith angle)), or zero if ray intersects ground
vec3 transmittanceWithShadow (float r, float mu)
{
	vec3 R = u_PrecomputedScatteringData.Rgtl;
    return mu < -sqrt(1.0f - (R.y / r) * (R.y / r)) ? vec3(0.0f) : transmittance(r, mu);
}

vec3 irradiance (float r, float muS)
{
    vec2 uv = getIrradianceUV(r, muS);
    return texture2D(u_PrecomputedScatteringData.IrradianceMap, uv).rgb;
}

// incident sun light at given position (radiance)
// r=length(x)
// muS=dot(x,s) / r
vec3 sunRadiance (float r, float muS)
{
    return transmittanceWithShadow(r, muS) * u_PrecomputedScatteringData.SunIntensity;
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
    return texture3D(table, vec3((uNu + uMuS) / float(RES_NU), uMu, uR)) * (1.0f - lerp) +
           texture3D(table, vec3((uNu + uMuS + 1.0f) / float(RES_NU), uMu, uR)) * lerp;
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
	else
	{
        result = vec3(0.0f);
        extinction = vec3(1.0f);
    }

    return result * u_PrecomputedScatteringData.SunIntensity;
}

// scattered sunlight between two points
// camera=observer
// point=point on the ground
// sundir=unit vector towards the sun
// return scattered light and extinction coefficient
vec3 inScattering (vec3 camera, vec3 point, vec3 sundir, out vec3 extinction)
{
    vec3 result;
    vec3 viewdir = point - camera;
    float d = length(viewdir);
    viewdir = viewdir / d;
    float r = length(camera);
    float rMu = dot(camera, viewdir);
    float mu = rMu / r;
    float r0 = r;
    float mu0 = mu;

	vec3 R = u_PrecomputedScatteringData.Rgtl;
    float deltaSq = sqrt(rMu * rMu - r * r + R.y * R.y);
    float din = max(-rMu - deltaSq, 0.0);
    if (din > 0.0f)
	{
        camera += din * viewdir;
        rMu += din;
        mu = rMu / R.y;
        r = R.y;
        d -= din;
    }

    if (r <= R.y)
	{
        float nu = dot(viewdir, sundir);
        float muS = dot(camera, sundir) / r;

        vec4 inScatter;

        if (r < R.x + 600.0f)
		{
            // avoids imprecision problems in aerial perspective near ground
            float f = (R.x + 600.0f) / r;
            r = r * f;
            rMu = rMu * f;
            point = point * f;
        }

        float r1 = length(point);
        float rMu1 = dot(point, viewdir);
        float mu1 = rMu1 / r1;
        float muS1 = dot(point, sundir) / r1;

        if (mu > 0.0f)
		{
            extinction = min(transmittance(r, mu) / transmittance(r1, mu1), 1.0f);
        }
		else
		{
            extinction = min(transmittance(r1, -mu1) / transmittance(r, -mu), 1.0f);
        }

        vec4 inScatter0 = texture4D(u_PrecomputedScatteringData.InscatterMap, r, mu, muS, nu);
        vec4 inScatter1 = texture4D(u_PrecomputedScatteringData.InscatterMap, r1, mu1, muS1, nu);
        inScatter = max(inScatter0 - inScatter1 * extinction.rgbr, 0.0f);

        // avoids imprecision problems in Mie scattering when sun is below horizon
        inScatter.w *= smoothstep(0.00f, 0.02f, muS);

        vec3 inScatterM = getMie(inScatter);
        float phase = phaseFunctionR(nu);
        float phaseM = phaseFunctionM(nu);
        result = inScatter.rgb * phase + inScatterM * phaseM;
    }
	else
	{
        result = vec3(0.0f);
        extinction = vec3(1.0f);
    }

    return result * u_PrecomputedScatteringData.SunIntensity;
}

void sunRadianceAndSkyIrradiance (vec3 worldP, vec3 worldS, out vec3 sunL, out vec3 skyE)
{
    vec3 worldV = normalize(worldP); // vertical vector
    float r = length(worldP);
    float muS = dot(worldV, worldS);
    sunL = sunRadiance(r, muS);
    skyE = skyIrradiance(r, muS);
}

/////////////// CLOUDS ///////////////

vec4 cloudColor (vec3 worldP, vec3 worldCamera, vec3 worldSunDir)
{
    float a = 23.0f / 180.0f * u_PrecomputedScatteringData.PI;
    mat2 m = mat2(cos(a), sin(a), -sin(a), cos(a));

    vec2 st = worldP.xy / 1000000.0f;
    float g = 1.0f;
    float r = 0.0f;
    for (float i = 0.0f; i < u_CloudsData.Octaves; i += 1.0f)
	{
        r -= g * (2.0f * texture2D(u_CloudsData.NoiseMap, st).r - 1.0f);
        st = (m * st) * u_CloudsData.Lacunarity;
        g *= u_CloudsData.Gain;
    }

    float v = clamp((r * u_CloudsData.Norm - u_CloudsData.Clamp1) / (u_CloudsData.Clamp2 - u_CloudsData.Clamp1), 0.0f, 1.0f);
    float t = clamp((r * u_CloudsData.Norm * 3.0f - u_CloudsData.Clamp1) / (u_CloudsData.Clamp2 - u_CloudsData.Clamp1), 0.0f, 1.0f);

	vec3 earthPos = vec3(0.0f, 0.0f, u_PrecomputedScatteringData.EarthRadius);
    vec3 PP = worldP + earthPos;

    vec3 Lsun;
    vec3 Esky;
    vec3 extinction;
    sunRadianceAndSkyIrradiance(PP, worldSunDir, Lsun, Esky);

	vec3 cloudL = v * (Lsun * max(worldSunDir.z, 0.0f) + Esky / 10.0f) / u_PrecomputedScatteringData.PI;

    vec3 inscatter = inScattering(worldCamera + earthPos, PP, worldSunDir, extinction);
    cloudL = cloudL * extinction + inscatter;

    return vec4(cloudL, t) * vec4(u_CloudsData.Color, 1.0f);
}

vec4 cloudColorV (vec3 worldCamera, vec3 V, vec3 worldSunDir)
{
    vec3 P = worldCamera + V * (u_CloudsData.Height - worldCamera.z) / V.z;
    return cloudColor(P, worldCamera, worldSunDir);
}

//////////////

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

////////////////////////////////

void main (void)
{
	vec4 finalColor = cloudColor(v_worldPos, u_CameraPosition, u_SunDirection);

#ifdef HDR
    finalColor.rgb = hdr(finalColor.rgb);
#endif // HDR

	fragColor = finalColor;
}	
