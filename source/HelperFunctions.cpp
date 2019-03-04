/* Author: BAIRAC MIHAI */

#include "HelperFunctions.h"
// glm::vec3, glm::vec4, glm::mat4 come from the header
#include "glm/mat3x3.hpp"
#include "glm/common.hpp" //abs()
#include "glm/geometric.hpp" //dot(), normalize()
#include "glm/gtc/constants.hpp" //epsilon()


namespace HelperFunctions
{
	bool LogicXOR ( bool i_A, bool i_B )
	{
		//!a != !b //(!a && b) || (a && !b)
		return static_cast<bool>((i_A + i_B) % 2);
	}

	bool EqualAbs ( float i_A, float i_B, float i_Epsilon )
	{
		return glm::abs(i_A - i_B) < i_Epsilon;
	}

	//////////////////////// VECTORS ////////////////////////
	glm::vec3 TransformPosition ( const glm::vec3& i_Vector, const glm::mat4& i_Matrix )
	{
		glm::vec4 tV = i_Matrix * glm::vec4(i_Vector, 1.0f); //if w == 1.0 - position, w == 0.0f - orientation

		return tV.w ? tV.xyz() / tV.w : glm::vec3(0.0f);
	}

	glm::vec3 TransformDirection ( const glm::vec3& i_Vector, const glm::mat4& i_Matrix )
	{
		return glm::mat3(i_Matrix) * i_Vector;
	}

	glm::vec4 Transform ( const glm::vec4& i_Vector, const glm::mat4& i_Matrix, bool i_ContainsProjection )
	{
		glm::vec4 tV = i_Matrix * i_Vector;

		if (i_ContainsProjection && tV.w)
		{
			return tV / tV.w;
		}
		else
		{
			return tV;
		}
	}

	/////////////////// PLANE //////////////////////////
	
	glm::vec4 CreatePlaneFromPointNormal ( const glm::vec3& i_Point, const glm::vec3& i_Normal )
	{
		// plane ecuation: a * x + b * y + c * z + d * w = 0
		return glm::vec4(i_Normal, - glm::dot(i_Point, i_Normal));
	}

	bool PlaneIntersectRay ( const glm::vec4& i_Plane, const glm::vec3& i_RayOrigin, const glm::vec3& i_RayDirection, glm::vec3& o_IntersectionPoint )
	{
		glm::vec3 planeNormal = i_Plane.xyz();
		float planeDistance = i_Plane.w;

		float nom = glm::dot(planeNormal, i_RayOrigin);
		float denom = glm::dot(planeNormal, i_RayDirection);

		if (EqualAbs(denom, 0.0f))
		{
			// ray is parallel to plane
			o_IntersectionPoint = glm::vec3(0.0f);
			return false;
		}

		float distance = - (nom + planeDistance) / denom;

		if (EqualAbs(distance, 0.0f))
		{
			// no intersection
			o_IntersectionPoint = glm::vec3(0.0f);
			return false;
		}

		o_IntersectionPoint = i_RayOrigin + i_RayDirection * distance;
		return true;
	}

	bool PlaneIntersectRay ( const glm::vec4& i_Plane, const glm::vec4& i_RayOrigin, const glm::vec4& i_RayDirection, glm::vec4& o_IntersectionPoint )
	{
		glm::vec4 planeNormalHS = glm::vec4(i_Plane.xyz(), 1.0f);
		float planeDistance = i_Plane.w;

		float nom = glm::dot(planeNormalHS, i_RayOrigin);
		float denom = glm::dot(planeNormalHS, i_RayDirection);

		if (EqualAbs(denom, 0.0f))
		{
			// ray is parallel to plane
			o_IntersectionPoint = glm::vec4(0.0f);
			return false;
		}

		float distance = - (nom + planeDistance) / denom;

		if (EqualAbs(distance, 0.0f))
		{
			// no intersection
			o_IntersectionPoint = glm::vec4(0.0f);
			return false;
		}

		o_IntersectionPoint = i_RayOrigin + i_RayDirection * distance;
		return true;
	}

	bool PlaneIntersectSegment ( const glm::vec4& i_Plane, const glm::vec3& i_Point1, const glm::vec3& i_Point2, glm::vec3& o_IntersectionPoint )
	{
		return PlaneIntersectRay(i_Plane, i_Point1, glm::normalize(i_Point2 - i_Point1), o_IntersectionPoint);
	}

	bool PlaneIntersectSegment ( const glm::vec4& i_Plane, const glm::vec4& i_Point1, const glm::vec4& i_Point2, glm::vec4& o_IntersectionPoint )
	{
		return PlaneIntersectRay(i_Plane, i_Point1, glm::normalize(i_Point2 - i_Point1), o_IntersectionPoint);
	}

	float PlaneDotPosition ( const glm::vec4& i_Plane, const glm::vec3& i_Position )
	{
		return glm::dot(i_Plane, glm::vec4(i_Position, 1.0f));
	}

	float PlaneDotDirection ( const glm::vec4& i_Plane, const glm::vec3& i_Direction )
	{
		return glm::dot(i_Plane, glm::vec4(i_Direction, 0.0f));
	}

	glm::vec3 ProjectPositionOnPlane ( const glm::vec4& i_Plane, const glm::vec3& i_Position )
	{
		glm::vec3 planeNormal = i_Plane.xyz();
		float planeDistance = i_Plane.w;

		glm::vec3 projected = i_Position - (glm::dot(i_Position, planeNormal) + planeDistance) * planeNormal;

		return projected;
	}

	glm::vec3 MirrorPointToPlane ( const glm::vec4& i_Plane, const glm::vec3& i_Point )
	{
		glm::vec3 planeNormal = i_Plane.xyz();
		float planeDistance = i_Plane.w;

		glm::vec3 mirrored = i_Point - 2.0f * (glm::dot(i_Point, planeNormal) + planeDistance) * planeNormal;

		return mirrored;
	}
}