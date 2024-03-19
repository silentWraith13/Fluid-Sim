#include "Game/Plane.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/OBJLoader.hpp"
#include "Engine/Core/Time.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Plane::Plane(const char* filePath)
{
	LoadPlaneObjModel(filePath);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Plane::~Plane()
{
	delete m_cpuMeshh;
	m_cpuMeshh = nullptr;

	delete m_gpuMesh;
	m_gpuMesh = nullptr;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Plane::Startup()
{
	m_planePosition = Vec3(3150.f, 4000.f, 300.f);
	m_planeOrientation = EulerAngles(90.f, 0.0f, 0.f);
	m_cameraOffset = Vec3(-30.f, 0.f, 5.f);
	m_planeCamera.m_orientation = m_planeOrientation;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Plane::Render()
{
	RenderCollisionSphereForPlane();

	g_theRenderer->SetModelConstants(GetModelMatrix());
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->BindShader(m_shader);
	g_theRenderer->SetLightConstants(Vec3(0.0f, 0.0f, -0.5f), 0.9f, 0.1f, Vec3(0.f, 0.f, 0.f), 0, 0, 0, 0);
	m_gpuMesh->Render();
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->SetModelConstants();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Plane::Update(float deltaSeconds)
{
	if (!m_isInFreeFlyMode)
	{
		float timeStep = std::min(0.005f, deltaSeconds);
		UpdatePlaneControlsGamepad(timeStep);
		m_planeCamera.SetTransform(GetPlaneCameraPosRelativeToPlanePos(), m_planeOrientation);
		m_collisionSphereCenter = m_planePosition;
	}
	else
	{
		UpdateFreeFlyModeKeyboard(deltaSeconds);
		m_planeCamera.SetTransform(m_freeFlyPosition, m_freeFlyOrientation);
	}
	
	TogglePlaneModes();
	ToggleCollisionSphereForPlane();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Plane::Shutdown()
{

}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Plane::LoadPlaneObjModel(const char* filePath)
{
	XmlDocument modelDefsXML;
	XmlResult result = modelDefsXML.LoadFile(filePath);
	if (result == tinyxml2::XML_ERROR_FILE_NOT_FOUND)
	{
		ERROR_AND_DIE("Could not open XML file");
	}

	GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS, Stringf("Failed to open required model \"%s\"", filePath));

	XmlElement* modelDefElement = modelDefsXML.RootElement();
	GUARANTEE_OR_DIE(modelDefElement, Stringf("Failed to open model def element"));

	std::string modelName = ParseXmlAttribute(*modelDefElement, "name", "INVALID MODEL NAME");
	std::string modelPath = ParseXmlAttribute(*modelDefElement, "path", "INVALID MODEL PATH");
	std::string shaderName = ParseXmlAttribute(*modelDefElement, "shader", "INVALID MODEL SHADER");

	m_shader = g_theRenderer->CreateShaderOrGetFromFile(shaderName.c_str(), VertexType::PNCU);
	if (!m_shader)
	{
		ERROR_AND_DIE("Could not create shader");
	}

	XmlElement const* transformChildElement = modelDefElement->FirstChildElement("Transform");
	Vec3 matrixI = Vec3(0.f, 0.f, 0.f);
	Vec3 matrixJ = Vec3(0.f, 0.f, 0.f);
	Vec3 matrixK = Vec3(0.f, 0.f, 0.f);
	Vec3 matrixT = Vec3(0.f, 0.f, 0.f);
	float scale = 0.f;

	if (transformChildElement)
	{
		matrixI = ParseXmlAttribute(*transformChildElement, "x", Vec3(-1.f, -1.f, -1.f));
		matrixJ = ParseXmlAttribute(*transformChildElement, "y", Vec3(-1.f, -1.f, -1.f));
		matrixK = ParseXmlAttribute(*transformChildElement, "z", Vec3(-1.f, -1.f, -1.f));
		matrixT = ParseXmlAttribute(*transformChildElement, "t", Vec3(-1.f, -1.f, -1.f));
		scale = ParseXmlAttribute(*transformChildElement, "scale", -1.f);
	}

	Mat44 fixupTransform = Mat44(matrixI, matrixJ, matrixK, matrixT);
	fixupTransform.AppendScaleUniform3D(scale);

	m_cpuMeshh = new CPUMesh();
	m_gpuMesh = new GPUMesh();
	m_cpuMeshh->m_vertices.clear();
	m_cpuMeshh->m_indices.clear();
	m_cpuMeshh->m_vertices.reserve(10000);
	m_cpuMeshh->m_indices.reserve(10000);
	OBJLoader::ImportFromOBJFile(modelPath, fixupTransform, m_cpuMeshh->m_vertices, m_cpuMeshh->m_indices);
	m_gpuMesh->SetVertexAndIndexBuffers(m_cpuMeshh);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Plane::UpdateFreeFlyModeKeyboard(float deltaSeconds)
{
	const float MOVE_SPEED = 1500.f;
	const float TURN_RATE = 90.f;
	const float MOUSE_SPEED = 0.1f;

	m_freeFlyVelocity = Vec3(0.f, 0.f, 0.f);

	if (g_theInput->IsKeyDown('W'))
	{
		Vec3 forward = Vec3(1.f, 0.f, 0.f);
		Vec3 left = Vec3(0.f, 1.f, 0.f);
		Vec3 up = Vec3(0.f, 0.f, 1.f);
		m_freeFlyOrientation.GetAsVectors_XFwd_YLeft_ZUp(forward, left, up);
		m_freeFlyVelocity += forward * MOVE_SPEED;
	}

	if (g_theInput->IsKeyDown('A'))
	{
		Vec3 forward = Vec3(1.f, 0.f, 0.f);
		Vec3 left = Vec3(0.f, 1.f, 0.f);
		Vec3 up = Vec3(0.f, 0.f, 1.f);
		m_freeFlyOrientation.GetAsVectors_XFwd_YLeft_ZUp(forward, left, up);
		m_freeFlyVelocity += left * MOVE_SPEED;
	}

	if (g_theInput->IsKeyDown('S'))
	{
		Vec3 forward = Vec3(1.f, 0.f, 0.f);
		Vec3 left = Vec3(0.f, 1.f, 0.f);
		Vec3 up = Vec3(0.f, 0.f, 1.f);
		m_freeFlyOrientation.GetAsVectors_XFwd_YLeft_ZUp(forward, left, up);
		m_freeFlyVelocity -= forward * MOVE_SPEED;
	}

	if (g_theInput->IsKeyDown('D'))
	{
		Vec3 forward = Vec3(1.f, 0.f, 0.f);
		Vec3 left = Vec3(0.f, 1.f, 0.f);
		Vec3 up = Vec3(0.f, 0.f, 1.f);
		m_freeFlyOrientation.GetAsVectors_XFwd_YLeft_ZUp(forward, left, up);
		m_freeFlyVelocity -= left * MOVE_SPEED;
	}

	if (g_theInput->WasKeyJustPressed('H'))
	{
		m_freeFlyPosition = Vec3(0.f, 0.f, 0.f);
		m_freeFlyOrientation = EulerAngles(0.f, 0.f, 0.f);
	}

	if (g_theInput->IsKeyDown('Q'))
	{
		m_freeFlyVelocity += MOVE_SPEED * Vec3(0.f, 0.f, 1.f);
	}

	if (g_theInput->IsKeyDown('E'))
	{
		m_freeFlyVelocity += MOVE_SPEED * Vec3(0.f, 0.f, -1.f);
	}


	if (g_theInput->IsKeyDown('Z'))
	{
		m_freeFlyOrientation.m_rollDegrees += TURN_RATE * deltaSeconds;
	}

	if (g_theInput->IsKeyDown('C'))
	{
		m_freeFlyOrientation.m_rollDegrees -= TURN_RATE * deltaSeconds;
	}

	if (g_theInput->IsKeyDown(KEYCODE_SHIFT))
	{
		m_freeFlyVelocity *= 10.0f;
	}

	Vec2 cursorDelta = g_theInput->GetCursorClientDelta();
	m_freeFlyOrientation.m_yawDegrees -= cursorDelta.x * MOUSE_SPEED;
	m_freeFlyOrientation.m_pitchDegrees += cursorDelta.y * MOUSE_SPEED;

	m_freeFlyOrientation.m_pitchDegrees = GetClamped(m_freeFlyOrientation.m_pitchDegrees, -85.f, 85.f);
	m_freeFlyOrientation.m_rollDegrees = GetClamped(m_freeFlyOrientation.m_rollDegrees, -45.f, 45.f);

	m_freeFlyPosition += m_freeFlyVelocity * deltaSeconds;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Plane::UpdatePlaneControlsGamepad(float deltaSeconds)
{
	const XboxController& controller = g_theInput->GetController(0);

	float targetYawDegrees = m_planeOrientation.m_yawDegrees;

	if (controller.IsButtonDown(XBOX_BUTTON_L_SHOULDER))
	{
		targetYawDegrees += YAW_RATE * deltaSeconds;
	}
	if (controller.IsButtonDown(XBOX_BUTTON_R_SHOULDER))
	{
		targetYawDegrees -= YAW_RATE * deltaSeconds;
	}
	
	AnalogJoystick const& leftStick = controller.GetLeftStick();
	Vec2 leftStickPos = leftStick.GetPosition();
	const float LERP_FACTOR = 0.2f;
	m_planeOrientation.m_yawDegrees = Interpolate(m_planeOrientation.m_yawDegrees, targetYawDegrees, LERP_FACTOR);
	m_planeOrientation.m_pitchDegrees += leftStickPos.y * PITCH_RATE * deltaSeconds;
	m_planeOrientation.m_rollDegrees += leftStickPos.x * ROLL_RATE * deltaSeconds;


	float leftTrigger = controller.GetLeftTrigger();
	float rightTrigger = controller.GetRightTrigger();

	float throttleInput = rightTrigger - leftTrigger;
	float currentSpeed = m_planeVelocity.GetLength();

	currentSpeed += throttleInput * (throttleInput > 0 ? ACCELERATION_RATE : DECELERATION_RATE) * deltaSeconds;
	currentSpeed = GetClamped(currentSpeed, 0.0f, MAX_SPEED);

	Vec3 forwardDir = m_planeOrientation.GetForwardVector().GetNormalized();
	m_planeVelocity = forwardDir * currentSpeed;
	CalculateAngleOfAttack();
	Vec3 liftPitchForce = CalculateLift(m_angleOfAttackPitch, Vec3(0.f, 1.f, 0.f), LIFT_POWER);
	Vec3 liftYawForce = CalculateLift(m_angleOfAttackYaw, Vec3(0.f, 0.f, 1.f), LIFT_POWER);
	Vec3 dragForce = CalculateDragForce();
	Vec3 totalExternalForcesActingOnPlane = liftPitchForce + liftYawForce + dragForce /*+ GRAVITY_FORCE*/;
	m_planeVelocity += totalExternalForcesActingOnPlane * deltaSeconds;
	m_planePosition += m_planeVelocity * deltaSeconds;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Mat44 Plane::GetModelMatrix()
{
	Mat44 modelMatrix = m_planeOrientation.GetAsMatrix_XFwd_YLeft_ZUp();
	modelMatrix.SetTranslation3D(m_planePosition);
	return modelMatrix;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Plane::TogglePlaneModes()
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_F1))
	{
		if (!m_isInFreeFlyMode)
		{
			m_freeFlyPosition = m_planeCamera.m_position;
			m_freeFlyOrientation = m_planeCamera.m_orientation;
		}
		else
		{
			m_planeCamera.m_position = m_planePosition;
			m_planeCamera.m_orientation = m_planeOrientation;
		}

		m_isInFreeFlyMode = !m_isInFreeFlyMode;
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec3 Plane::GetPlaneCameraPosRelativeToPlanePos()
{
	Vec3 cameraPosition = m_planePosition + m_planeOrientation.GetAsMatrix_XFwd_YLeft_ZUp().TransformVectorQuantity3D(m_cameraOffset);
	return cameraPosition;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Plane::RenderCollisionSphereForPlane()
{
	if (m_enableCollisionSphere)
	{
		m_collisionSphereVerts.clear();
		AddVertsForSphere3D(m_collisionSphereVerts, m_collisionSphereCenter, m_collisionSphereRadius);

		g_theRenderer->SetRasterizerModes(RasterizerMode::WIREFRAME_CULL_BACK);
		g_theRenderer->BindShader(nullptr);
		g_theRenderer->BindTexture(nullptr);
		g_theRenderer->DrawVertexArray((int)m_collisionSphereVerts.size(), m_collisionSphereVerts.data());
		g_theRenderer->SetRasterizerModes(RasterizerMode::SOLID_CULL_NONE);
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Plane::ToggleCollisionSphereForPlane()
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_F2))
	{
		m_enableCollisionSphere = !m_enableCollisionSphere;
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Plane::CalculateAngleOfAttack()
{
	if (m_planeVelocity.GetLengthSquared() < 0.1f)
	{
		m_angleOfAttackYaw = 0.f;
		m_angleOfAttackPitch = 0.f;
		return;
	}

	//Angle of attack on yaw and pitch in our coordinate system.
	m_angleOfAttackPitch = Atan2Degrees(-m_planeVelocity.z , m_planeVelocity.x); // -ve sign because we get it from the pitch direction to velocity vector. 
	m_angleOfAttackYaw = Atan2Degrees(m_planeVelocity.y, m_planeVelocity.x);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Plane::UpdateThrottle(float deltaSeconds)
{
	const XboxController& controller = g_theInput->GetController(0);

	float leftTrigger = controller.GetLeftTrigger();
	float rightTrigger = controller.GetRightTrigger();

	float throttleInput = rightTrigger - leftTrigger;
	float currentSpeed = m_planeVelocity.GetLength();

	currentSpeed += throttleInput * (throttleInput > 0 ? ACCELERATION_RATE : DECELERATION_RATE) * deltaSeconds;
	currentSpeed = GetClamped(currentSpeed, 0.0f, MAX_SPEED);

	Vec3 forwardDir = m_planeOrientation.GetForwardVector().GetNormalized();

	// Apply throttle
	m_planeVelocity = forwardDir * currentSpeed;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec3 Plane::GetDragCoefficient()
{
	Vec3 scaledDragBasedOnDirection = ScaleVec3(m_planeVelocity, DRAG_FORWARD, DRAG_LEFT, DRAG_TOP, DRAG_BACK, DRAG_RIGHT, DRAG_BOTTOM);
	return scaledDragBasedOnDirection;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec3 Plane::CalculateDragForce()
{
	Vec3 dragCoefficient = GetDragCoefficient();
	Vec3 normalizedVelocity = m_planeVelocity.GetNormalized();
	float velocitySquared = m_planeVelocity.GetLengthSquared();

	Vec3 dragForce = dragCoefficient.GetLength() * velocitySquared * -1 * normalizedVelocity;
	return dragForce;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec3 Plane::CalculateLift(float angleOfAttack, Vec3 const& axis, float liftPower)
{
	if (m_planeVelocity.GetLengthSquared() < 1.f)
	{
		return Vec3(0.f, 0.f, 0.f);
	}

	Vec3 liftVelocity = GetProjectedOnto3D(m_planeVelocity, axis);

	// 2. Lift velocity squared
	float velocitySquared = liftVelocity.GetLengthSquared();

	// 3. liftCoeff - Calculate lift coefficient based on the angle of attack
	float liftCoeff = CalculateLiftCoefficient(angleOfAttack);

	// 4. liftForce = v^2 * liftCoeff * liftPower
	float liftForce = velocitySquared * liftCoeff * liftPower;

	// 5. liftDirection - perpendicular to the airflow (cross product of lift velocity and axis)
	Vec3 liftDirection = CrossProduct3D(liftVelocity.GetNormalized(), axis);
	
	// 6. lift = liftDirection * liftForce
	Vec3 lift = liftDirection * liftForce;

	// 7. dragForce = liftCoeff * liftCoeff * some value
	float dragForce = liftCoeff * liftCoeff * DRAG_COEFFICIENT;

	// 8. dragDirection = -liftVelocity.GetNormalized()
	Vec3 inducedDragDirection = -1 * liftVelocity.GetNormalized();

	// 9. induced drag = dragDirection * v^2 * dragForce
	Vec3 inducedDrag = inducedDragDirection * velocitySquared * dragForce;

	// 10. return lift + inducedDrag
	return lift + inducedDrag;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
float Plane::CalculateLiftCoefficient(float angleOfAttack)
{
	float angleOfAttackRadians = ConvertDegreesToRadians(angleOfAttack);
	float liftCoeff = std::max(0.f, angleOfAttackRadians);
	return liftCoeff;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------