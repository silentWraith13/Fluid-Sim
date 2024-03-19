#include "Game/Game.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/Time.hpp"
#include "Game/App.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Game/Player.hpp"
#include "Game/Prop.hpp"
#include "Engine/Core/DebugRenderSystem.hpp"
#include "Game/Map.hpp"
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Game::Game()
{
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Game::~Game()
{
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Game::Startup()
{
	std::string imagePath = "Data/HeightMaps/HM_Australia.png";
	if (!imagePath.empty())
	{
		m_currentHeightMapImage = Image(imagePath.c_str());
	}
	
	m_dirtGrassDiffuseTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/DirtGrass_Diffuse.png");
	m_dirtGrassNormalTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/DirtGrass_Normal.png");

	m_snowDiffuseTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Snow_Diffuse.png");
	m_snowNormalTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Snow_Normal.png");
	m_waterDiffuseTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Water_Diffuse.png");
	m_waterNormalTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Water_Normal.png");
	m_waterNormalSpecTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/WaterNormalSpec.png");
	m_waterDudvTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/WaterDuDv.png");
	m_waterReflectionRenderTexure = g_theRenderer->CreateRenderTexture(g_theWindow->GetClientDimensions().x, g_theWindow->GetClientDimensions().y);
	m_waterRefractionRenderTexure = g_theRenderer->CreateRenderTexture(g_theWindow->GetClientDimensions().x, g_theWindow->GetClientDimensions().y);

	SetScreenCamera();

 	m_map = new Map();
 	if (m_map)
 	{
 		m_map->Startup();
 	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Game::ShutDown()
{
	delete m_map;
	m_map = nullptr;

	delete m_waterReflectionRenderTexure;
	m_waterReflectionRenderTexure = nullptr;

	delete m_waterRefractionRenderTexure;
	m_waterRefractionRenderTexure = nullptr;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Game::Update()
{
 	if (m_map)
 	{
 		m_map->Update(g_theApp->m_clock.GetDeltaSeconds());
 	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Game::Render() const
{
	if (m_map)
	{
		m_map->Render();
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Game::SetScreenCamera()
{
	m_screenCamera.m_mode = Camera::eMode_Orthographic;
	m_screenCamera.SetOrthographicView(Vec2(0.f, 0.f), Vec2(1600.f, 800.f));
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
