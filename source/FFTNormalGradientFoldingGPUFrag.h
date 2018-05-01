/* Author: BAIRAC MIHAI */

#ifndef FFT_NORMAL_GRADIENT_FOLDING_GPU_FRAG_H
#define FFT_NORMAL_GRADIENT_FOLDING_GPU_FRAG_H

#include <string>
#include <map>

#include "GlobalConfig.h"

#include "FFTNormalGradientFoldingBase.h"

#include "FrameBufferManager.h"

/*
  GPU implementation of the normal gradients and folding using fragment shaders
*/

class FFTNormalGradientFoldingGPUFrag : public FFTNormalGradientFoldingBase
{
private:
	
	//// Variables ////
	FrameBufferManager m_FBM;

	//// Methods ////
	void Destroy(void);

public:
	FFTNormalGradientFoldingGPUFrag ( void );
	FFTNormalGradientFoldingGPUFrag ( const GlobalConfig& i_Config );
	virtual ~FFTNormalGradientFoldingGPUFrag ( void );

	void Initialize ( const GlobalConfig& i_Config ) override;

	void ComputeNormalGradientFolding ( void ) override;

	void BindTexture ( void ) const override;
	unsigned short GetTexUnitId ( void ) const override;

	void LinkSourceTex ( unsigned int i_SourceTex ) override;
};

#endif /* FFT_NORMAL_GRADIENT_FOLDING_GPU_FRAG_H */