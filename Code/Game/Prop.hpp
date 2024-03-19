#pragma once
#include "Game/Entity.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/Texture.hpp"
#include <vector>

class Game;

class Prop : public Entity
{
public:
	Prop(Game* owner);
	~Prop();
	virtual void     Update(float deltaSeconds) override;
	virtual void     Render()  override;
	void			 CreateQuadOne();			//Creates a 1x1x1 cube centered at (2,2,0).
	void			 CreateQuadTwo();			//Creates a 1x1x1 cube centered at (-2,-2,0).
	void			 CreateSphere3D();
	void			 CreateGrid();

public:
	
	std::vector<Vertex_PCU>	 m_vertexes;
	
	Texture*				 m_texture = nullptr;
};