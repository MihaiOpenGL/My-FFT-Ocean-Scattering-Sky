/* Author: BAIRAC MIHAI */

#ifndef BASE_SKY_MODEL_H
#define BASE_SKY_MODEL_H

#include <string>
#include <vector>
#include <map>

#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp" /////

#include "ShaderManager.h"
#include "MeshBufferManager.h"

class GlobalConfig;
class Camera;

/*
 Base Sky Model class
*/

class BaseSkyModel
{
protected:
	//// Variables ////
	ShaderManager m_SM;
	MeshBufferManager m_MBM;

	// self init
	// name, location
	std::map<std::string, int> m_Uniforms;

	struct SunData
	{
		glm::vec3 Direction;
		bool AllowChangeDirWithMouse;
		bool IsDynamic;
		float MoveFactor;
	} m_SunData;

	unsigned int m_IndexCount;
	bool m_IsWireframeMode;

private:
	//// Methods ////
	void Destroy ( void );

public:
	BaseSkyModel ( void );
	BaseSkyModel ( const GlobalConfig& i_Config );
	virtual ~BaseSkyModel ( void );

	virtual void Initialize ( const GlobalConfig& i_Config );

	virtual void Update ( const Camera& i_Camera, bool i_IsUnderWater, bool i_IsWireframeMode, float i_CrrTime );
	virtual void UpdateReflected ( const glm::mat4& i_ScaleMatrix, const Camera& i_Camera, bool i_IsUnderWater, bool i_IsWireframeMode, float i_CrrTime );
	virtual void UpdateRefracted ( const glm::mat4& i_ScaleMatrix, const Camera& i_Camera, bool i_IsUnderWater, bool i_IsWireframeMode, float i_CrrTime );

	virtual void Render ( void );
	virtual void RenderReflected ( void );
	virtual void RenderRefracted ( void );

	virtual void UpdateSunPosition ( float i_CrrTime );

	virtual void SetSunDirection ( float i_Phi, float i_Theta );
	virtual const glm::vec3& GetSunDirection ( void ) const;

	virtual bool GetAllowChangeDirWithMouse ( void ) const;

	virtual void SetEnabledClouds ( bool i_Value );
	virtual bool GetEnabledClouds ( void ) const;

	virtual void SetCloudsOctaves ( unsigned short i_Octaves );
	virtual unsigned short GetCloudsOctaves ( void ) const;
	virtual void SetCloudsLacunarity ( float i_Lacunarity );
	virtual float GetCloudsLacunarity ( void ) const;
	virtual void SetCloudsGain  (float i_Gain );
	virtual float GetCloudsGain ( void ) const;
	virtual void SetCloudsScaleFactor ( float i_ScaleFactor );
	virtual float GetCloudsScaleFactor ( void ) const;
	virtual void SetCloudsNorm ( float i_Norm );
	virtual float GetCloudsNorm ( void ) const;
};

#endif /* BASE_SKY_MODEL_H */