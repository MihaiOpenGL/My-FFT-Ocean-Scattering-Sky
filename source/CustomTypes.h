/* Author: BAIRAC MIHAI */

#ifndef CUSTOM_TYPES_H
#define CUSTOM_TYPES_H

namespace CustomTypes
{
	namespace Sky
	{
		enum class ModelType : unsigned short
		{
			MT_CUBE_MAP = 0,
			MT_SCATTERING,
			MT_PRECOMPUTED_SCATTERING,
			MT_COUNT
		};
	}

	namespace Ocean
	{
		enum class GridType : unsigned short
		{
			GT_WORLD_SPACE = 0,
			GT_SCREEN_SPACE,
			GT_COUNT
		};
		
		enum class ComputeFFTType : unsigned short
		{ 
			CFT_GPU_FRAG = 0,
			CFT_GPU_COMP,
			CFT_CPU_FFTW, 
			CFT_COUNT
		};

		enum class SpectrumType : unsigned short 
		{
			ST_PHILLIPS = 0,
			ST_UNIFIED,
			ST_COUNT
		};

		enum class NormalGradientFoldingType : unsigned short
		{ 
			NGF_GPU_FRAG = 0,
			NGF_GPU_COMP,
			NGF_COUNT
		};
	}

	namespace PostProcessing
	{
		enum class EffectType : unsigned short 
		{ 
			PPET_Invert = 0,
			PPET_Grey,
			PPET_BlackWhite,
			PPET_Sepia,
			PPET_Wavy,
			PPET_Blur,
			PPET_EdgeDetection,
			PPET_NoEffect,
			PPET_COUNT
		};
	}
}

#endif /* CUSTOM_TYPES_H */