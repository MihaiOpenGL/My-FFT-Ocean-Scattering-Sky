/* Author: BAIRAC MIHAI

Compute shader FFT code based on https://github.com/McNopper/OpenGL/tree/master/Example41 glsl code
License: GNU LESSER GENERAL PUBLIC LICENSE

*/

layout (binding = 0, rgba16f) uniform image2DArray u_imageFFTIn; 
layout (binding = 1, rgba16f) uniform image2DArray u_imageFFTOut;

layout (binding = 2, r16f) uniform image1D u_imageIndices; 
layout (binding = 3, rg16f) uniform image2D u_imageWeights; 

// FFT_SIZE is added from C++ - via a simulated preprocessor

// Local size is FFT_SIZE/2. Processing two fields per invocation.
layout (local_size_x = FFT_SIZE/2, local_size_y = 1, local_size_z = 1) in;

// Faster, when stored in shared memory compared to global memory.
// we have 3 layers: DY, DXDZ and SXSZ
shared vec4 sharedStore[3][FFT_SIZE];

uniform int u_Steps;


vec2 complex_mult_complex (vec2 c1, vec2 c2)
{
	// (x, yi) * (u, vi) = (xu - yv), (xv + yu)i

	return vec2(c1.x * c2.x - c1.y * c2.y, c1.x * c2.y + c1.y * c2.x);
}

void loadData (ivec2 storeIndex, ivec2 leftLoadPos, ivec2 rightLoadPos, int layer)
{
	sharedStore[layer][storeIndex.x] = imageLoad(u_imageFFTIn, ivec3(leftLoadPos, layer));
	sharedStore[layer][storeIndex.y] = imageLoad(u_imageFFTIn, ivec3(rightLoadPos, layer));
}

void storeData (ivec2 leftStorePos, ivec2 rightStorePos, int layer, vec3 leftData, vec3 rightData)
{
	imageStore(u_imageFFTOut, ivec3(leftStorePos, layer), vec4(leftData, 0.0f));
	imageStore(u_imageFFTOut, ivec3(rightStorePos, layer), vec4(rightData, 0.0f));
}

void computeIFFT (ivec2 index, vec2 weight, int layer)
{
	vec2 leftValue = sharedStore[layer][index.x].xy;
	vec2 rightValue = sharedStore[layer][index.y].xy;

	vec2 mult = complex_mult_complex(weight, rightValue);	
		
	vec2 add = leftValue + mult;
	vec2 sub = leftValue - mult;

	sharedStore[layer][index.x] = vec4(add, 0.0f, 0.0f);
	sharedStore[layer][index.y] = vec4(sub, 0.0, 0.0f);
}

void compute2IFFT (ivec2 index, vec2 weight, int layer)
{
	vec4 leftValue = sharedStore[layer][index.x];
	vec4 rightValue = sharedStore[layer][index.y];

	vec4 mult;
	mult.xy = complex_mult_complex(weight, rightValue.xy);
	mult.zw = complex_mult_complex(weight, rightValue.zw);	
		
	vec4 add = leftValue + mult;
	vec4 sub = leftValue - mult;

	sharedStore[layer][index.x] = add;
	sharedStore[layer][index.y] = sub;
}

void main (void)
{
	ivec2 index = ivec2(gl_GlobalInvocationID);
	ivec2 storeIndex = ivec2(2 * index.x, 2 * index.x + 1);

	// Load the swizzled indices
	ivec2 loadIndex;
	loadIndex.x = int(imageLoad(u_imageIndices, storeIndex.x).x);
	loadIndex.y = int(imageLoad(u_imageIndices, storeIndex.y).x);

	ivec2 leftLoadPos = ivec2(index.y, loadIndex.x);
	ivec2 rightLoadPos = ivec2(index.y, loadIndex.y);

	ivec2 leftStorePos = ivec2(index.y, storeIndex.x);
	ivec2 rightStorePos = ivec2(index.y, storeIndex.y);

	// Copy and swizzle values for butterfly algortihm into the shared memory.
	loadData(storeIndex, leftLoadPos, rightLoadPos, 0);
	loadData(storeIndex, leftLoadPos, rightLoadPos, 1);
	loadData(storeIndex, leftLoadPos, rightLoadPos, 2);

	// Make sure that all values are stored and visible after the barrier. 
	memoryBarrierShared();
	barrier();

	int currentButterfly = 0;
	int currentSection = index.x;
	int numberButterfliesInSection = 1;

	// Performing needed FFT steps per either row or column.
	for (int currentStep = 0; currentStep < u_Steps; ++currentStep)
	{	
		ivec2 localIndex;
		localIndex.x = currentButterfly + currentSection * numberButterfliesInSection * 2;
		localIndex.y = localIndex.x + numberButterfliesInSection;
	
		// "Butterfly" math.
		
		// load weight
		ivec2 uv = ivec2(localIndex.x, currentStep);
		vec2 weight = imageLoad(u_imageWeights, uv).xy;

		computeIFFT(localIndex, weight, 0);
		compute2IFFT(localIndex, weight, 1);
		compute2IFFT(localIndex, weight, 2);

		// Make sure, that values are written.		
		memoryBarrierShared();

		// Change parameters for butterfly and section index calculation.		
		currentSection /= 2;
		numberButterfliesInSection *= 2;
		currentButterfly = index.x % numberButterfliesInSection;

		// Make sure, that all shaders are at the same stage, as now indices are changed.
		barrier();
	}

	// sign correction
	if ((index.y + storeIndex.x) % 2 == 1)
	{
		sharedStore[0][storeIndex.x] *= -1.0f;
		sharedStore[1][storeIndex.x] *= -1.0f;
		sharedStore[2][storeIndex.x] *= -1.0f;
	}

	if ((index.y + storeIndex.y) % 2 == 1)
	{
		sharedStore[0][storeIndex.y] *= -1.0f;
		sharedStore[1][storeIndex.y] *= -1.0f;
		sharedStore[2][storeIndex.y] *= -1.0f;
	}
		
	// Make sure, that values are written.
	memoryBarrierShared();

	///// Aggregate final data and store it /////
	// Displacement
	vec3 leftDisplacement = vec3(- sharedStore[1][storeIndex.x].x, sharedStore[0][storeIndex.x].x, - sharedStore[1][storeIndex.x].z);
	vec3 rightDisplacement = vec3(- sharedStore[1][storeIndex.y].x, sharedStore[0][storeIndex.y].x, - sharedStore[1][storeIndex.y].z);

	storeData(leftStorePos, rightStorePos, 0, leftDisplacement, rightDisplacement);

	// Slopes
	vec3 leftSlopes = vec3(- sharedStore[2][storeIndex.x].x, - sharedStore[2][storeIndex.x].z, 0.0f);
	vec3 rightSlopes = vec3(- sharedStore[2][storeIndex.y].x, - sharedStore[2][storeIndex.y].z, 0.0f);

	storeData(leftStorePos, rightStorePos, 1, leftSlopes, rightSlopes);
}	
