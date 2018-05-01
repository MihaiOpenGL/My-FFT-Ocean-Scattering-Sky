/* Author: BAIRAC MIHAI */

#ifndef FFT_NORMAL_GRADIENT_FOLDING_GPU_COMP_H
#define FFT_NORMAL_GRADIENT_FOLDING_GPU_COMP_H

#include <string>
#include <map>

#include "GlobalConfig.h"

#include "TextureManager.h"

#include "FFTNormalGradientFoldingBase.h"

/*
  GPU implementation of the normal gradients and folding using compute shaders
*/

class FFTNormalGradientFoldingGPUComp : public FFTNormalGradientFoldingBase
{
private:
	//// Variables ////
	TextureManager m_TM;

	unsigned int m_TexId, m_SourceTexId;
	
	//// Methods ////
	void Destroy ( void );

public:
	FFTNormalGradientFoldingGPUComp ( void );
	FFTNormalGradientFoldingGPUComp ( const GlobalConfig& i_Config );
	virtual ~FFTNormalGradientFoldingGPUComp ( void );

	void Initialize ( const GlobalConfig& i_Config ) override;

	void ComputeNormalGradientFolding ( void ) override;

	void BindTexture ( void ) const override;
	unsigned short GetTexUnitId ( void ) const override;

	void LinkSourceTex ( unsigned int i_SourceTex ) override;
};

#endif /* FFT_NORMAL_GRADIENT_FOLDING_GPU_COMP_H */