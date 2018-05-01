/* Author: BAIRAC MIHAI */

#ifndef POST_PROCESSING_MANANGER_H
#define POST_PROCESSING_MANANGER_H

#include <string>
#include <map>

#include "FrameBufferManager.h"
#include "ShaderManager.h"

#include "CustomTypes.h"

class GlobalConfig;

class PostProcessingManager
{
private:
	//// Variables ////
	FrameBufferManager m_FBM;
	ShaderManager m_SM;

	// self init
	// name, location
	std::map<std::string, int> m_Uniforms;

	CustomTypes::PostProcessing::EffectType m_EffectType;

	//// Methods ////
	void Destroy ( void );

public:
	PostProcessingManager ( const GlobalConfig& i_Config );
	~PostProcessingManager ( void );

	void Initialize ( const GlobalConfig& i_Config );

	void Update ( float i_CrrTime, float i_DeltaTime );
	void Render ( void );

	void BindFB ( void );
	void UnBindFB ( void );
};

#endif /* POST_PROCESSING_MANANGER_H */