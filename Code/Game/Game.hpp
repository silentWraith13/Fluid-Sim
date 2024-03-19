#pragma once
#include "Engine/Core/Material.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/Image.hpp"
//--------------------------------------------------------------------------------------------------------------------------------------------------------
class Camera;
class Entity;
class Prop;
class Map;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
class Game
{
public:
	Game();
	~Game();
	void                Startup();
	void                ShutDown();
	void                Update();
	void                Render() const;
	void				SetScreenCamera();

	Camera				m_screenCamera;
	Map*				m_map = nullptr;
	Image		        m_currentHeightMapImage;
	Texture*			m_dirtGrassDiffuseTexture = nullptr;
	Texture*			m_dirtGrassNormalTexture = nullptr;
	Texture*			m_snowDiffuseTexture = nullptr;
	Texture*			m_snowNormalTexture = nullptr;
	Texture*			m_waterDiffuseTexture = nullptr;
	Texture*			m_waterNormalTexture = nullptr;
	Texture* m_waterNormalSpecTexture = nullptr;
	Texture* m_waterDudvTexture = nullptr;
	RenderTexture* m_waterReflectionRenderTexure = nullptr;
	RenderTexture* m_waterRefractionRenderTexure = nullptr;
	Material*			m_material = nullptr;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------