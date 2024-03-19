#pragma once
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Core/CPUMesh.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Math/Mat44.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
const static float LIFT_POWER = 100000.0f;  
const static float DRAG_COEFFICIENT = 0.0000001f;
const static float MAX_SPEED = 300.f;
const static float ACCELERATION_RATE = 50.0f;
const static float DECELERATION_RATE = 50.0f;
const static float PITCH_RATE = 10.0f;
const static float ROLL_RATE = 10.0f;
const static float YAW_RATE = 20.0f;
const static float TAKEOFF_SPEED = 15000.0f;
const static float DRAG_FORWARD = 0.00000001f;
const static float DRAG_BACK = 0.0000001f;
const static float DRAG_LEFT = 0.0000001f;
const static float DRAG_RIGHT = 0.00000001f;
const static float DRAG_TOP = 0.00000001f;
const static float DRAG_BOTTOM = 0.00000001f;
const static Vec3  GRAVITY_FORCE = Vec3(0.f, 0.f, -9.8f);
//--------------------------------------------------------------------------------------------------------------------------------------------------------
class Plane
{
public:
	Plane(const char* filePath);
	~Plane();

	void  Startup();
	void  Render();
	void  Update(float deltaSeconds);
	void  Shutdown();
	
	void  LoadPlaneObjModel(const char* filePath);
	void  UpdateFreeFlyModeKeyboard(float deltaSeconds);
	void  UpdatePlaneControlsGamepad(float deltaSeconds);
	Mat44 GetModelMatrix();
	void  TogglePlaneModes();
	Vec3  GetPlaneCameraPosRelativeToPlanePos();
	void  RenderCollisionSphereForPlane();
	void  ToggleCollisionSphereForPlane();

	void CalculateAngleOfAttack();
	void UpdateThrottle(float deltaSeconds);
	Vec3 GetDragCoefficient();
	Vec3 CalculateDragForce();
	Vec3 CalculateLift(float angleOfAttack, Vec3 const& axis, float liftPower);
	float CalculateLiftCoefficient(float angleOfAttack);

	GPUMesh*				m_gpuMesh  = nullptr;
	CPUMesh*				m_cpuMeshh = nullptr;
	Shader*					m_shader   = nullptr;
	Camera					m_planeCamera;
	Vec3					m_planePosition;
	Vec3					m_freeFlyPosition;
	Vec3					m_freeFlyVelocity;
	Vec3					m_planeVelocity;
	Vec3					m_cameraOffset;
	EulerAngles				m_planeOrientation;
	EulerAngles				m_planeAngularVelocity;
	EulerAngles				m_freeFlyOrientation;
	EulerAngles				m_freeFlyAngularVelocity;
	bool					m_isInFreeFlyMode = false;
	Vec3					m_collisionSphereCenter;
	float					m_collisionSphereRadius = 5.f;
	std::vector<Vertex_PCU> m_collisionSphereVerts;
	bool					m_enableCollisionSphere = false;
	float					m_angleOfAttackPitch = 0.f;
	float					m_angleOfAttackYaw = 0.f;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------