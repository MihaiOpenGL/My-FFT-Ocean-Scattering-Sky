/* Author: BAIRAC MIHAI */

#ifndef FFT_NORMAL_GRADIENT_FOLDING_GPU_FRAG_H
#define FFT_NORMAL_GRADIENT_FOLDING_GPU_FRAG_H

#include "FFTNormalGradientFoldingBase.h"
#include "FrameBufferManager.h"
#include <string>
#include <map>

class GlobalConfig;

/*
  GPU implementation of the normal gradients and folding using fragment shaders
*/

class FFTNormalGradientFoldingGPUFrag : public FFTNormalGradientFoldingBase
{
public:
	FFTNormalGradientFoldingGPUFrag(void);
	FFTNormalGradientFoldingGPUFrag(const GlobalConfig& i_Config);
	virtual ~FFTNormalGradientFoldingGPUFrag(void);

	void Initialize(const GlobalConfig& i_Config) override;

	void ComputeNormalGradientFolding(void) override;

	void BindTexture(void) const override;
	unsigned short GetTexUnitId(void) const override;

	void LinkSourceTex(unsigned int i_SourceTex) override;

private:
	//// Methods ////
	void Destroy(void);

	//// Variables ////
	FrameBufferManager m_FBM;
};

#endif /* FFT_NORMAL_GRADIENT_FOLDING_GPU_FRAG_H */