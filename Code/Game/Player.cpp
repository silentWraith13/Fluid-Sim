#include "Game/Player.hpp"
#include "Game/GameCommon.hpp"
#include "Game/App.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Player::Player(Game* owner)
	:Entity(owner)
{
	m_position = Vec3(0.f, 0.f, 100.f);
	m_orientation = EulerAngles(45.f, 45.f, 0.f);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Player::~Player()
{

}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Player::Update(float deltaSeconds)
{
	UpdatePlayerKeyboardControls(deltaSeconds);
	m_playerCamera.SetTransform(m_position, m_orientation);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Player::Render()
{

}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Player::UpdatePlayerKeyboardControls(float deltaSeconds)
{
	const float MOVE_SPEED = 200.f;
	const float TURN_RATE = 90.f;
	const float MOUSE_SPEED = 0.1f;
	
	m_velocity = Vec3(0.f, 0.f, 0.f);
	
	if (g_theInput->IsKeyDown('W'))
	{
		Vec3 forward = Vec3(1.f, 0.f, 0.f);
		Vec3 left = Vec3(0.f, 1.f, 0.f);
		Vec3 up = Vec3(0.f, 0.f, 1.f);
		m_orientation.GetAsVectors_XFwd_YLeft_ZUp(forward, left, up);
		m_velocity += forward * MOVE_SPEED;
	}

	if (g_theInput->IsKeyDown('A'))
	{
		Vec3 forward = Vec3(1.f, 0.f, 0.f);
		Vec3 left = Vec3(0.f, 1.f, 0.f);
		Vec3 up = Vec3(0.f, 0.f, 1.f);
		m_orientation.GetAsVectors_XFwd_YLeft_ZUp(forward, left, up);
		m_velocity += left * MOVE_SPEED;
	}

	if (g_theInput->IsKeyDown('S'))
	{
		Vec3 forward = Vec3(1.f, 0.f, 0.f);
		Vec3 left = Vec3(0.f, 1.f, 0.f);
		Vec3 up = Vec3(0.f, 0.f, 1.f);
		m_orientation.GetAsVectors_XFwd_YLeft_ZUp(forward, left, up);
		m_velocity -= forward * MOVE_SPEED;
	}

	if (g_theInput->IsKeyDown('D'))
	{
		Vec3 forward = Vec3(1.f, 0.f, 0.f);
		Vec3 left = Vec3(0.f, 1.f, 0.f);
		Vec3 up = Vec3(0.f, 0.f, 1.f);
		m_orientation.GetAsVectors_XFwd_YLeft_ZUp(forward, left, up);
		m_velocity -= left * MOVE_SPEED;
	}



	if (g_theInput->WasKeyJustPressed('H'))
	{
		m_position = Vec3(0.f, 0.f, 0.f);
		m_orientation= EulerAngles(0.f, 0.f ,0.f);
	}

	if (g_theInput->IsKeyDown('Q'))
	{
		m_velocity += MOVE_SPEED * Vec3(0.f, 0.f, 1.f);
	}

	if (g_theInput->IsKeyDown('E'))
	{
		m_velocity += MOVE_SPEED * Vec3(0.f, 0.f, -1.f);
	}


	if (g_theInput->IsKeyDown('Z'))
	{
		m_orientation.m_rollDegrees += TURN_RATE * deltaSeconds;
	}

	if (g_theInput->IsKeyDown('C'))
	{
		m_orientation.m_rollDegrees -= TURN_RATE * deltaSeconds;
	}

	if (g_theInput->IsKeyDown(KEYCODE_SHIFT))
	{
		m_velocity *= 10.0f;
	}

	Vec2 cursorDelta = g_theInput->GetCursorClientDelta();
	m_orientation.m_yawDegrees -= cursorDelta.x * MOUSE_SPEED;
	m_orientation.m_pitchDegrees += cursorDelta.y * MOUSE_SPEED;

	m_orientation.m_pitchDegrees = GetClamped(m_orientation.m_pitchDegrees, -85.f, 85.f);
	m_orientation.m_rollDegrees = GetClamped(m_orientation.m_rollDegrees, -45.f, 45.f);

	m_position += m_velocity * deltaSeconds;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Player::UpdatePlayerGamepadControls(float deltaSeconds)
{
	const float MOVE_SPEED = 2.f;
	const float TURN_RATE = 90.f;
	const float MOUSE_SPEED = 0.1f;

	m_velocity = Vec3(0.f, 0.f, 0.f);
	(void)deltaSeconds;
	
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
