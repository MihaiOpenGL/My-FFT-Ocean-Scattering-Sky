/* Author: BAIRAC MIHAI */

#include "Projector.h"
#include <iostream>
#include "Common.h"
#include "HelperFunctions.h"
#include "ErrorHandler.h"
// glm::vec2, glm::vec3, glm::mat4 come from the header
#include "glm/common.hpp" //mix(), abs()
#include "glm/geometric.hpp" //normalize()
#include "GlobalConfig.h"


Projector::Projector ( void )
	: m_IsPlaneWithinFrustum(false), m_IsFrustumVisible(false),
	  m_MaxWaveAmplitude(0.0f), m_StrengthElevation(0.0f), m_AimPointCorrection(0.0f), m_IsUnderMainPlane(false)
{}

Projector::Projector (const GlobalConfig& i_Config, bool i_IsUnderwater)
	: m_IsPlaneWithinFrustum(false), m_IsFrustumVisible(false),
	  m_MaxWaveAmplitude(0.0f), m_StrengthElevation(0.0f), m_AimPointCorrection(0.0f), m_IsUnderMainPlane(false)
{
	Initialize(i_Config, i_IsUnderwater);
}

Projector::~Projector ( void )
{
	Destroy();
}

void Projector::Initialize ( const GlobalConfig& i_Config, bool i_IsUnderwater )
{
	if (i_IsUnderwater)
	{
		m_Position = i_Config.Scene.Ocean.Bottom.Projector.Position;
		m_Normal = glm::normalize(i_Config.Scene.Ocean.Bottom.Projector.Normal);

		m_MaxWaveAmplitude = i_Config.Scene.Ocean.Bottom.Projector.MaxWaveAmplitude;
		m_StrengthElevation = i_Config.Scene.Ocean.Bottom.Projector.StrengthElevation;
		m_AimPointCorrection = i_Config.Scene.Ocean.Bottom.Projector.AimPointCorrection;
	}
	else
	{
		m_Position = i_Config.Scene.Ocean.Surface.Projector.Position;
		m_Normal = glm::normalize(i_Config.Scene.Ocean.Surface.Projector.Normal);

		m_MaxWaveAmplitude = i_Config.Scene.Ocean.Surface.Projector.MaxWaveAmplitude;
		m_StrengthElevation = i_Config.Scene.Ocean.Surface.Projector.StrengthElevation;
		m_AimPointCorrection = i_Config.Scene.Ocean.Surface.Projector.AimPointCorrection;
	}

	//create base plane
	m_Plane = HelperFunctions::CreatePlaneFromPointNormal(m_Position, m_Normal);

	// NOTE! IF one increaseas the displaceent wave amplitude so will increase the rendered ocean surface (geometry)

	//create upper & lower planes
	SetDisplacementWaveAmplitude();
}

void Projector::Destroy ( void )
{
}

void Projector::Update ( const Camera& i_RenderingCamera )
{
	float x_min = 0.0f, y_min = 0.0f, x_max = 0.0f, y_max = 0.0f;
	glm::vec3 frustum[8], projPoints[24]; // frustum[8] - frustum corners to check the camera against, projPoints - buffer for the projected points

	unsigned short intersectionPointsCount = 0; //number of intersection points
	const unsigned short k_cube[] = { 0, 1,  0, 2,  2, 3,  1, 3,	//first point in pair is src, second is dst
									  0, 4,  2, 6,  3, 7,  1, 5,
									  4, 6,  4, 5,  5, 7,  6, 7
									};	// which frustum points are connected together?


		// transform frustum points to worldspace (should be done to the rendering_camera because it's the interesting one)
		// frustum points can be used later to check for interection with upper and lower bound planes
		// frustum ul e definit in clipspace, deci folosim matricea invers pt ProjectionView ca sa ajungem din clipspace inapoi in worldspace(spatiul in care lucram)
		// http://www.songho.ca/opengl/gl_projectionmatrix.html - from NDC space to world-space
		// http://www.songho.ca/opengl/gl_transform.html
		// http://www.opengl-tutorial.org/beginners-tutorials/tutorial-3-matrices/
		// in NDC space (Normalized Device Space) camera frustum is represented by a cube
		// every cube point(each of the 8) having coordinates like (+-1, +-1, +-1)
		// to obtaine ech of the points in world-space an inverse transform is needed
		// newPos = (ProjViewMatrix * oldPos) / w whih includes also the required perspective devide
		// in OpenGL pipeline the perspective devide happens by default and can not be changed
		// it happens between projection matrix transform and the viewport transform

	frustum[0] = HelperFunctions::TransformPosition(glm::vec3(-1.0f, -1.0f, -1.0f), i_RenderingCamera.GetInverseProjectionViewMatrix());
	frustum[1] = HelperFunctions::TransformPosition(glm::vec3(+1.0f, -1.0f, -1.0f), i_RenderingCamera.GetInverseProjectionViewMatrix());
	frustum[2] = HelperFunctions::TransformPosition(glm::vec3(-1.0f, +1.0f, -1.0f), i_RenderingCamera.GetInverseProjectionViewMatrix());
	frustum[3] = HelperFunctions::TransformPosition(glm::vec3(+1.0f, +1.0f, -1.0f), i_RenderingCamera.GetInverseProjectionViewMatrix());

	frustum[4] = HelperFunctions::TransformPosition(glm::vec3(-1.0f, -1.0f, +1.0f), i_RenderingCamera.GetInverseProjectionViewMatrix());
	frustum[5] = HelperFunctions::TransformPosition(glm::vec3(+1.0f, -1.0f, +1.0f), i_RenderingCamera.GetInverseProjectionViewMatrix());
	frustum[6] = HelperFunctions::TransformPosition(glm::vec3(-1.0f, +1.0f, +1.0f), i_RenderingCamera.GetInverseProjectionViewMatrix());
	frustum[7] = HelperFunctions::TransformPosition(glm::vec3(+1.0f, +1.0f, +1.0f), i_RenderingCamera.GetInverseProjectionViewMatrix());

	// all intersection points are in world space
	// check intersections with upper_bound and lower_bound
	for (unsigned short i = 0; i < 12; ++i)
	{
		unsigned short src = k_cube[i * 2], dst = k_cube[i * 2 + 1];

		// comparison with 0.0f denotes on which side of the plane(positive or negative) is the point of the frustum
		if (HelperFunctions::PlaneDotPosition(m_UpperPlane, frustum[src]) / HelperFunctions::PlaneDotPosition(m_UpperPlane, frustum[dst]) < 0.0f)
		{
			HelperFunctions::PlaneIntersectSegment(m_UpperPlane, frustum[src], frustum[dst], projPoints[intersectionPointsCount++]);
		}

		if (HelperFunctions::PlaneDotPosition(m_LowerPlane, frustum[src]) / HelperFunctions::PlaneDotPosition(m_LowerPlane, frustum[dst]) < 0.0f)
		{
			HelperFunctions::PlaneIntersectSegment(m_LowerPlane, frustum[src], frustum[dst], projPoints[intersectionPointsCount++]);
		}
	}

	// check if any of the frustums vertices lie between the upper_bound and lower_bound planes
	for (unsigned short i = 0; i < 8; ++i)
	{
		if (HelperFunctions::PlaneDotPosition(m_UpperPlane, frustum[i]) / HelperFunctions::PlaneDotPosition(m_LowerPlane, frustum[i]) < 0.0f)
		{
			projPoints[intersectionPointsCount++] = frustum[i];
		}
	}

	// create the camera the grid will be projected from
	m_ProjectingCamera.Copy(i_RenderingCamera);

	float heightInPlane = i_RenderingCamera.GetAltitude();

	m_IsUnderMainPlane = false;
	if (heightInPlane < m_Position.y)
	{
		m_IsUnderMainPlane = true;
	}

	// with increasing strengthAndElevation increases the quality of the ocean surface if the camera is near
	if (heightInPlane < m_StrengthElevation)
	{
		glm::vec3 newPos;
		if (m_IsUnderMainPlane)
		{
			//m_LowerBound.xyz() - plane normal
			newPos = m_ProjectingCamera.GetPosition() + m_LowerPlane.xyz() * (m_StrengthElevation - 2.0f * heightInPlane);
		}
		else
		{
			newPos = m_ProjectingCamera.GetPosition() + m_LowerPlane.xyz() * (m_StrengthElevation - heightInPlane);
		}
		m_ProjectingCamera.SetPosition(newPos);
	}

	// correction aim points in world space
	glm::vec3 aimPoint, aimPoint2;

	// aim the projector at the point where the camera view-vector intersects the plane
	// if the camera is aimed away from the plane, mirror it's view-vector against the plane 
	if (HelperFunctions::LogicXOR((HelperFunctions::PlaneDotDirection(m_Plane, i_RenderingCamera.GetForward()) < 0.0f), (HelperFunctions::PlaneDotPosition(m_Plane, i_RenderingCamera.GetPosition()) < 0.0f)))
	{
		HelperFunctions::PlaneIntersectSegment(m_Plane, i_RenderingCamera.GetPosition(), i_RenderingCamera.GetPosition() + i_RenderingCamera.GetForward(), aimPoint);
	}
	else
	{
		glm::vec3 flipped = glm::normalize(HelperFunctions::MirrorPointToPlane(m_Plane, i_RenderingCamera.GetForward()));

		HelperFunctions::PlaneIntersectSegment(m_Plane, i_RenderingCamera.GetPosition(), i_RenderingCamera.GetPosition() + flipped, aimPoint);
	}

	// force the point the camera is looking at in a plane, and have the projector look at it
	// works well against horizon, even when camera is looking upwards
	// doesn't work straight down/up
	float af = glm::abs(HelperFunctions::PlaneDotDirection(m_Plane, i_RenderingCamera.GetForward()));

	aimPoint2 = i_RenderingCamera.GetPosition() + m_AimPointCorrection * i_RenderingCamera.GetForward();
	aimPoint2 = HelperFunctions::ProjectPositionOnPlane(m_Plane, aimPoint2);

	// fade between aimPoint & aimPoint2 depending on view angle
	//mix
	aimPoint = glm::mix(aimPoint2, aimPoint, af);

	m_ProjectingCamera.SetForward(aimPoint - m_ProjectingCamera.GetPosition());

	m_ProjectingCamera.UpdateViewMatrix();

	// project the intersection points onto the main plane
	for (unsigned short i = 0; i < intersectionPointsCount; ++i)
	{
		// project the intersection points onto the surface plane
		projPoints[i] = HelperFunctions::ProjectPositionOnPlane(m_Plane, projPoints[i]);

		// bring all intersection points to the clip/post-perspective/projector space
		projPoints[i] = HelperFunctions::TransformPosition(projPoints[i], m_ProjectingCamera.GetProjectionViewMatrix());
	}

	m_IsPlaneWithinFrustum = false;

	// get max/min x & y-values to determine how big the "projection window" must be
	if (intersectionPointsCount > 0)
	{
		x_min = projPoints[0].x;
		x_max = projPoints[0].x;
		y_min = projPoints[0].y;
		y_max = projPoints[0].y;
		for (unsigned short i = 1; i < intersectionPointsCount; ++i)
		{
			if (projPoints[i].x > x_max) x_max = projPoints[i].x;
			if (projPoints[i].x < x_min) x_min = projPoints[i].x;
			if (projPoints[i].y > y_max) y_max = projPoints[i].y;
			if (projPoints[i].y < y_min) y_min = projPoints[i].y;
		}

		// build the packing matrix that spreads the grid across the "projection window"
		// when building the pack matrix, we feed it with column vectors, because GLM uses
		// column major ordering convention !!!

		// so we create the pack matrix directly transposed!
		glm::mat4 pack(x_max - x_min, 0.0f,          0.0f, 0.0f,
					   0.0f,		  y_max - y_min, 0.0f, 0.0f,
					   0.0f,		  0.0f,          1.0f, 0.0f,
			           x_min,         y_min,         0.0f, 1.0f
			          );

		// clip space to world space matrix
		m_ProjectingMatrix = m_ProjectingCamera.GetInverseProjectionViewMatrix() * pack;

		m_IsPlaneWithinFrustum = true;
	}
}

glm::mat4 Projector::ComputeGridCorners ( void )
{
	glm::vec4 gridCorners[4];

	ComputeGridCornerPosition(glm::vec2(0.0f, 0.0f), gridCorners[0]); //near
	ComputeGridCornerPosition(glm::vec2(1.0f, 0.0f), gridCorners[1]); //near
	ComputeGridCornerPosition(glm::vec2(0.0f, 1.0f), gridCorners[2]); //far
	ComputeGridCornerPosition(glm::vec2(1.0f, 1.0f), gridCorners[3]); //far

	return glm::mat4(gridCorners[0], gridCorners[1], gridCorners[2], gridCorners[3]);
}


bool Projector::ComputeGridCornerPosition ( const glm::vec2& i_UV, glm::vec4& o_GridCornerPos )
{
	// points in homogenous normalized device coordinates (post-perspective space)
	glm::vec4 point1(i_UV, -1.0f, 1.0f);
	glm::vec4 point2(i_UV, 1.0f, 1.0f);

	// transform the points to world space homogenous coordinates (projection is done here)
	point1 = m_ProjectingMatrix * point1;
	point2 = m_ProjectingMatrix * point2;

	// compute the intersection point
	return HelperFunctions::PlaneIntersectSegment(m_Plane, point1, point2, o_GridCornerPos);
}

void Projector::SetDisplacementWaveAmplitude ( void )
{
	m_UpperPlane = HelperFunctions::CreatePlaneFromPointNormal(m_Position + m_Normal * m_MaxWaveAmplitude, m_Normal);
	m_LowerPlane = HelperFunctions::CreatePlaneFromPointNormal(m_Position - m_Normal * m_MaxWaveAmplitude, m_Normal);
}

const glm::mat4& Projector::GetProjectingMatrix ( void ) const
{
	return m_ProjectingMatrix;
}

const Camera& Projector::GetProjectingCamera ( void ) const
{
	return m_ProjectingCamera;
}

const glm::vec4& Projector::GetPlane ( void ) const
{
	return m_Plane;
}

float Projector::GetPlaneDistance ( void ) const
{
	return m_Plane.w;
}

bool Projector::IsPlaneWithinFrustum ( void ) const
{
	return m_IsPlaneWithinFrustum;
}

bool Projector::IsUnderMainPlane ( void ) const
{
	return m_IsUnderMainPlane;
}