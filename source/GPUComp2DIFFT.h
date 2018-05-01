/* Author: BAIRAC MIHAI

 Compute shader implementation based on 
 c++ code: https://github.com/McNopper/OpenGL/tree/master/Example41
 License: GNU LESSER GENERAL PUBLIC LICENSE

*/

#ifndef GPU_COMP_2D_IFFT_H
#define GPU_COMP_2D_IFFT_H

#include <string>
#include <vector>
#include <map>

#include "Base2DIFFT.h"

#include "ShaderManager.h"

class GlobalConfig;

/*
 GPU implementation of the 2D IFFT using compute shaders
 based on: GLUS lib tutorial example
 https://github.com/McNopper/OpenGL/tree/master/Example41

 More info about compute shaders
 https://www.khronos.org/opengl/wiki/Compute_Shader
 https://www.khronos.org/opengl/wiki/Image_Load_Store

 Compute Shaders use case:
 https://www.khronos.org/assets/uploads/developers/library/2014-siggraph-bof/KITE-BOF_Aug14.pdf
 useful extension: https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_compute_variable_group_size.txt
*/

class GPUComp2DIFFT: public Base2DIFFT
{
private:
	static const unsigned short m_kPingPongLayerCount = 2;

	unsigned int m_IndicesTexId, m_WeightsTexId;
	unsigned int m_PingPongTexIds[m_kPingPongLayerCount];

	//// Variables ////
	unsigned short m_NumButterflies;
 
	ShaderManager m_HorizontalSM, m_VerticalSM;

	// self init
	// name, location
	std::map<std::string, int> m_HorizontalUniforms;
	std::map<std::string, int> m_VerticalUniforms;

	//// Methods ////
	void Destroy ( void );

	float* ComputeIndicesLookupTexture ( void );
	void ShuffleIndices ( float* o_pBuffer, unsigned short i_N, unsigned short i_Offset );

	float* ComputeWeightsLookupTexture ( void );
	void ComputeWeight ( unsigned short i_K, float& i_Wr, float& i_Wi );

public:
	GPUComp2DIFFT ( void );
	GPUComp2DIFFT ( const GlobalConfig& i_Config );
	~GPUComp2DIFFT ( void );

	void Initialize ( const GlobalConfig& i_Config ) override;
	void Perform2DIFFT ( void ) override;

	void BindDestinationTexture ( void ) const override;

	unsigned int GetSourceTexId ( void ) const override;
	unsigned short GetSourceTexUnitId ( void ) const override;
	unsigned int GetDestinationTexId ( void ) const override;
	unsigned short GetDestinationTexUnitId ( void ) const override;
};

#endif /* GPU_COMP_2D_IFFT_H */