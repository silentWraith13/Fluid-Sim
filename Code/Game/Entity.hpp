#pragma once
 #include "Engine/Math/Vec3.hpp"
 #include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Core/Rgba8.hpp"

class Game;

class Entity
{
public:
	Entity(Game* owner);
	virtual ~Entity();

	virtual void Update(float deltaSeconds) = 0;
	virtual void Render() = 0;
	Mat44		 GetModelMatrix() const;

public:
	Game*		m_game = nullptr;
	
	Vec3		m_position;
	Vec3		m_velocity;
	
	EulerAngles	m_orientation;
	EulerAngles m_angularVelocity;
	Rgba8		m_color = Rgba8(255, 255, 255, 255);
};