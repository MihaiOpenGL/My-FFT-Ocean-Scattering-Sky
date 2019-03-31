/* Author: BAIRAC MIHAI

Implemented a simplied version of edge detection
using a slightly different version of the laplacian operator

Edge detection algorithm: https://gist.github.com/Hebali/6ebfc66106459aacee6a9fac029d0115
Laplacian: https://homepages.inf.ed.ac.uk/rbf/HIPR2/log.htm

*/

uniform sampler2D u_ColorMap;

/// Interpolated inputs across mesh
in vec2 v_uv;
///

out vec4 fragColor;

void main(void)
{	
	const float offset = 1.0f / 300.0f; 

    vec2 offsets[9] = vec2[](
        vec2(-offset, offset),  // top-left
        vec2(0.0f,    offset),  // top-center
        vec2(offset,  offset),  // top-right
        vec2(-offset, 0.0f),    // center-left
        vec2(0.0f,    0.0f),    // center-center
        vec2(offset,  0.0f),    // center-right
        vec2(-offset, -offset), // bottom-left
        vec2(0.0f,    -offset), // bottom-center
        vec2(offset,  -offset)  // bottom-right    
    );

	// based on laplacian operator
    float kernel[9] = float[9](
        -1, -1, -1,
        -1,  9, -1,
        -1, -1, -1
    );
    
    vec3 sampleTex[9];
    for(int i = 0; i < 9; i++)
    {
        sampleTex[i] = vec3(texture(u_ColorMap, v_uv + offsets[i]));
    }
    vec3 finalColor = vec3(0.0f);
    for(int i = 0; i < 9; i++)
	{
		finalColor += sampleTex[i] * kernel[i];
	}
    
    fragColor = vec4(finalColor, 1.0f);
}
