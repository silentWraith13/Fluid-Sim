#pragma once
#include "Game/Entity.hpp"
#include "Engine/Renderer/Camera.hpp"

class Game;

class Player: public Entity
{
public:
	Player(Game* owner);
	~Player();
	virtual void    Update(float deltaSeconds) override;
	virtual void    Render() override;
	void			UpdatePlayerKeyboardControls(float deltaSeconds);
	void			UpdatePlayerGamepadControls(float deltaSeconds);
public:
	Camera			m_playerCamera;
};

