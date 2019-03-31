/* Author: BAIRAC MIHAI

Atmospheric scattring code based and adapted from
code: https://www.gamedev.net/forums/topic/461747-atmospheric-scattering-sean-oneill---gpu-gems2/
paper: https://developer.nvidia.com/gpugems/GPUGems2/gpugems2_chapter16.html

License: GameDev.net Open License

*/

uniform mat4 u_WorldToClipMatrix;
uniform mat4 u_ObjectToWorldMatrix;

uniform vec3 u_CameraPosition;
uniform vec3 u_SunDirection;

struct ScatteringData 
{
	float EarthRadius;
	int SampleCount;
	vec3 InvWavelength;
	float InnerRadius;
	float KrESun;
	float KmESun;
	float Kr4PI;
	float Km4PI;
	float Scale;
	float ScaleDepth;
	float ScaleOverScaleDepth;
};
uniform ScatteringData u_ScatteringData;

in vec3 a_position;

out float v_fragY;
out vec3 v_RayleighColor;
out vec3 v_MieColor;
out vec3 v_Direction;

float applyScale (float cos)
{
	float x = 1.0f - cos;
	return u_ScatteringData.ScaleDepth * exp(-0.00287f + x * (0.459f + x * (3.83f + x * (-6.80f + x * 5.25f))));
}

void computeAtmosphereScattering (void)
{
	vec3 pos = a_position;

	// Get the ray from the camera to the vertex, and its length (which is the far point of the ray passing through the atmosphere)
	pos /= u_ScatteringData.EarthRadius;
	pos.y += u_ScatteringData.InnerRadius;

	vec3 ray = pos - u_CameraPosition;
	float far = length(ray);
	ray /= far;

	// Calculate the ray's starting position, then calculate its scattering offset
	vec3 start = u_CameraPosition;
	float height = length(start);
	float depth = exp(u_ScatteringData.ScaleOverScaleDepth * (u_ScatteringData.InnerRadius - u_CameraPosition.y));
	float startAngle = dot(ray, start) / height;
	float startOffset = depth * applyScale(startAngle);

	// Initialize the scattering loop variables
	float fSampleCount = u_ScatteringData.SampleCount;
	float sampleLength = far / fSampleCount;
	float scaledLength = sampleLength * u_ScatteringData.Scale;
	vec3 sampleRay = ray * sampleLength;
	vec3 samplePoint = start + sampleRay * 0.5f;

	// Now loop through the sample rays
	vec3 accColor = vec3(0.0f); //accumulated color

	for(int i = 0; i < u_ScatteringData.SampleCount; i ++)
	{
		float height = length(samplePoint);
		float depth = exp(u_ScatteringData.ScaleOverScaleDepth * (u_ScatteringData.InnerRadius - height));
		float lightAngle = dot(u_SunDirection, samplePoint) / height;
		float cameraAngle = dot(ray, samplePoint) / height;
		float scatter = (startOffset + depth * (applyScale(lightAngle) - applyScale(cameraAngle)));

		// Accumulate color
		vec3 attenuate = exp(- scatter * (u_ScatteringData.InvWavelength * u_ScatteringData.Kr4PI + u_ScatteringData.Km4PI));
		accColor += attenuate * (depth * scaledLength);

		// Next sample point
		samplePoint += sampleRay;
	}

	v_Direction = u_CameraPosition - pos;

	// Finally, scale the Mie and Rayleigh colors and set up the varying variables for the fragment shader
	v_RayleighColor = accColor * (u_ScatteringData.InvWavelength * u_ScatteringData.KrESun);
	v_MieColor = accColor * u_ScatteringData.KmESun;
}

void main (void)
{
	computeAtmosphereScattering();

	vec4 world_pos = u_ObjectToWorldMatrix * vec4(a_position, 1.0f);

	v_fragY = a_position.y;

	// we use the w value instead of z as explained in here: https://learnopengl.com/#!Advanced-OpenGL/Cubemaps
	vec4 clip_pos = u_WorldToClipMatrix * world_pos;
	gl_Position = clip_pos.xyww;
}
