/* Author: BAIRAC MIHAI

 Reference: GeoMathLib
 web page: http://cb.demon.fi/MathGeoLib/nightly/sourcecode.html
 License: Apache 2 license

*/

/*
DirectX vs OpenGL
http://fgiesen.wordpress.com/2012/02/12/row-major-vs-column-major-row-vectors-vs-column-vectors/

OpenGL fixed-function is specced as using column vectors and column-major storage.
D3D fixed-function is specced as using row vectors and row-major storage.
If you look at an individual matrix (say translation by (tx, ty, tz)),
the D3D and GL versions look exactly the same in memory. However, they do not
behave the same way: in OpenGL, the matrix that represents “first A then B” is B*A;
in D3D, it is A*B.

GLM uses row-major matrices column-major vectors!

*/

#ifndef HELPER_FUNCTIONS_H
#define HELPER_FUNCTIONS_H

#define GLM_SWIZZLE //offers the possibility to use: .xx(), xy(), xyz(), ...
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "glm/mat4x4.hpp"


namespace HelperFunctions
{
	bool LogicXOR ( bool i_A, bool i_B );

	bool EqualAbs ( float i_A, float i_B, float i_Epsilon = 1e-6f );


	/////// VECTOR TRANSFORMS ////////////
	glm::vec3 TransformPosition ( const glm::vec3& i_Position, const glm::mat4& i_Matrix );

	glm::vec3 TransformDirection ( const glm::vec3& i_Direction, const glm::mat4& i_Matrix );

	glm::vec4 Transform ( const glm::vec4& i_Vector, const glm::mat4& i_Matrix, bool i_ContainsProjection = false );

	////// PLANE ///////////////
	//// i_Plane.xyz - normal
	//// i_Plane.w - D Term
	glm::vec4 CreatePlaneFromPointNormal ( const glm::vec3& i_Point, const glm::vec3& i_Normal );

	// world space 3d coordiantes
	bool PlaneIntersectRay ( const glm::vec4& i_Plane, const glm::vec3& i_RayOrigin, const glm::vec3& i_RayDirection, glm::vec3& o_IntersectionPoint );
	// homogenous space coordinates
	bool PlaneIntersectRay ( const glm::vec4& i_Plane, const glm::vec4& i_RayOrigin, const glm::vec4& i_RayDirection, glm::vec4& o_IntersectionPoint );
	//// world space 3d coordiantes - modified DirectX implementation - avoid artifacts
	bool PlaneIntersectSegment ( const glm::vec4& i_Plane, const glm::vec3& i_Point1, const glm::vec3& i_Point2, glm::vec3& o_IntersectionPoint );
	// homogenous space coordinates
	bool PlaneIntersectSegment ( const glm::vec4& i_Plane, const glm::vec4& i_Point1, const glm::vec4& i_Point2, glm::vec4& o_IntersectionPoint );

	// computes the signed distance between a plane and a point in space
	// source: http://clb.demon.fi/MathGeoLib/nightly/docs/Plane.cpp_code.html#222
	float PlaneDotPosition ( const glm::vec4& i_Plane, const glm::vec3& i_Position );

	float PlaneDotDirection ( const glm::vec4& i_Plane, const glm::vec3& i_Direction );

	glm::vec3 ProjectPositionOnPlane ( const glm::vec4& i_Plane, const glm::vec3& i_Position );

	glm::vec3 MirrorPointToPlane ( const glm::vec4& i_Plane, const glm::vec3& i_Point );
}

#endif /* HELPER_FUNCTIONS_H */
