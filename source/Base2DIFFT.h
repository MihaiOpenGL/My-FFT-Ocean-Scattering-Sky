/* Author: BAIRAC MIHAI */

#ifndef BASE_2D_IFFT_H
#define BASE_2D_IFFT_H

#include "TextureManager.h"

class GlobalConfig;

/*
 Base class for 2D IFFT computation
*/

class Base2DIFFT
{
public:
	Base2DIFFT(void);
	Base2DIFFT(const GlobalConfig& i_Config);
	virtual ~Base2DIFFT(void);

	virtual void Initialize(const GlobalConfig& i_Config);

	virtual void Perform2DIFFT(void);

	virtual void BindDestinationTexture(void) const;

	virtual unsigned short GetFFTLayerCount(void) const;
	virtual bool GetUseFFTSlopes(void) const;

	virtual unsigned int GetSourceTexId(void) const;
	virtual unsigned short GetSourceTexUnitId(void) const;
	virtual unsigned int GetDestinationTexId(void) const;
	virtual unsigned short GetDestinationTexUnitId(void) const;

protected:
	//// Variables ////
	static const unsigned short m_kMipmapCount = 3;

	unsigned short m_NumButterflies;

	unsigned short m_FFTSize;
	unsigned short m_FFTLayerCount;

	TextureManager m_TM;

	bool m_UseFFTSlopes;

private:
	//// Methods ////
	void Destroy ( void );
};

#endif /* BASE_2D_IFFT_H */