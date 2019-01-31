/* Author: BAIRAC MIHAI

 Code based on nVidia DIrect3D 11 SDK OceanCS demo
 Lcense: check the License.pdf file in the sample package

*/

#ifndef FFT_NORMAL_GRADIENT_FOLDING_BASE_H
#define FFT_NORMAL_GRADIENT_FOLDING_BASE_H

#include <string>
#include <map>
#include "ShaderManager.h"

class GlobalConfig;

/*
 Base class for FFT gradients of normals and folding computation

 Jerry Tessendorf ocean article: 
 http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.161.9102&rep=rep1&type=pdf

 Normal map and folding implementations are based on Jerry Tessendorf work 
 and also the nVidia DirectX11 Ocean demo.
 https://developer.nvidia.com/dx11-samples - check the ocean sample
*/

class FFTNormalGradientFoldingBase
{
public:
	FFTNormalGradientFoldingBase(void);
	FFTNormalGradientFoldingBase(const GlobalConfig& i_Config);
	virtual ~FFTNormalGradientFoldingBase(void);

	virtual void Initialize(const GlobalConfig& i_Config);

	virtual void ComputeNormalGradientFolding(void);

	virtual void BindTexture(void) const;
	virtual unsigned short GetTexUnitId(void) const;

	virtual void LinkSourceTex(unsigned int i_SourceTex);

	virtual void SetPatchSize(unsigned short i_PatchSize);
	virtual void SetChoppyScale(float i_ChoppyScale);

protected:
	//// Variables ////
	ShaderManager m_SM;

	std::map<std::string, int> m_Uniforms;

	unsigned short m_FFTSize;

private:
	//// Methods ////
	void Destroy ( void );
};

#endif /* FFT_NORMAL_GRADIENT_FOLDING_BASE_H */