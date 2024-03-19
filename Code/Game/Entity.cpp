#include "Game/Entity.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Entity::Entity(Game* owner)
	:m_game(owner)
{
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Entity::~Entity()
{

}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Entity::Update(float deltaSeconds)
{
	m_position += (m_velocity * deltaSeconds);
	
	m_orientation.m_yawDegrees += m_angularVelocity.m_yawDegrees * deltaSeconds;
	m_orientation.m_pitchDegrees += m_angularVelocity.m_pitchDegrees * deltaSeconds;
	m_orientation.m_rollDegrees += m_angularVelocity.m_rollDegrees * deltaSeconds;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Entity::Render() 
{

}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Mat44 Entity::GetModelMatrix() const
{
 	Mat44 rotation = m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();
 	rotation.SetTranslation3D(m_position);
 	Mat44 modelMatrix = rotation;
 	return modelMatrix;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
