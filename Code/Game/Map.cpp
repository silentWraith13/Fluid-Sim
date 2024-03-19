#include "Game/Map.hpp"
#include "Game/Game.hpp"
#include "Game/Player.hpp"
#include "Game/GameCommon.hpp"
#include "Game/App.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/ConstantBuffer.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/JobSystem.hpp"
#include <queue>
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Map::Map()
{ 
	m_chunkSize.x = (int)CHUNK_SIZE_METERS;
	m_chunkSize.y = (int)CHUNK_SIZE_METERS;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Map::~Map()
{
	ClearChunkVector();
	
	delete m_plane;
	m_plane = nullptr;

// 	delete m_octree;
// 	m_octree = nullptr;

	delete m_waterVertexBuffer;
	m_waterVertexBuffer = nullptr;

	delete m_waterIndexBuffer;
	m_waterIndexBuffer = nullptr;

	delete m_clippingPlaneCBO;
	m_clippingPlaneCBO = nullptr;

}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Map::Startup()
{
	InitializePlane();
	InitializeRunway();
	InitializeUIFont();
	InitializeLRIndicatorShape();
	InitializeRenderTextureQuad();
	InitializeWaterQuad();
	CreateClippingPlaneCB();
	m_waterLightingShader = g_theRenderer->CreateShaderOrGetFromFile("Data/Shaders/WaterLighting", VertexType::PCUTBN);
	m_waterRenderShader = g_theRenderer->CreateShaderOrGetFromFile("Data/Shaders/WaterRender", VertexType::PCUTBN);

// 	IntVec2 terrainDimensions = g_theGame->m_currentHeightMapImage.GetDimensions();
// 	float terrainWidth = terrainDimensions.x * TILE_SIZE_METERS * CHUNK_SIZE_TILES;
// 	float terrainLength = terrainDimensions.y * TILE_SIZE_METERS * CHUNK_SIZE_TILES;

// 	AABB3 terrainBounds(Vec3(0, 0, 0), Vec3(terrainWidth, terrainLength, TERRAIN_MAX_HEIGHT));
// 	m_octree = new Octree(terrainBounds);
// 
// 	int totalChunksX = CHUNK_SIZE_TILES;
// 	int totalChunksY = CHUNK_SIZE_TILES;
// 	for (int x = 0; x < totalChunksX; x++) 
// 	{
// 		for (int y = 0; y < totalChunksY; y++) 
// 		{
// 			IntVec2 chunkCoord(x, y);
// 			Chunk* newChunk = new Chunk(chunkCoord);
// 			m_chunks.push_back(newChunk);
// 			m_octree->AddChunk(newChunk);
// 		}
// 	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Map::Shutdown()
{
	
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Map::Update(float deltaSeconds)
{
	BindClippingPlaneCB();
	m_fpsMs = deltaSeconds;
	bool isChunkActivated = ActivateNearestMissingChunk();

	if (!isChunkActivated)
	{
		DeactivateFurthestChunk();
	}
	
	//CheckForCompletedJobs();
	UpdateChunk(deltaSeconds);
	UpdatePlane(deltaSeconds);
// 	CheckIfPlaneHasCollidedWithTerrain();
// 	DestroyPlaneIfDead(deltaSeconds);
// 	RespawnPlane(deltaSeconds);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Map::Render()
{
	Rgba8 clearColor{ 135,206, 235, 200 };
	g_theRenderer->ClearScreen(clearColor);

	if (m_plane)
	{
		g_theRenderer->BeginCamera(m_plane->m_planeCamera);
		
		//reflection
   		g_theRenderer->SetRenderTarget(g_theGame->m_waterReflectionRenderTexure);
   		g_theRenderer->ClearRenderTarget(g_theGame->m_waterReflectionRenderTexure, clearColor);
 		float distance = 2 * (m_plane->m_planeCamera.m_position.z - 270.f);
  		m_plane->m_planeCamera.m_position.z -= distance;
  		m_plane->m_planeCamera.m_orientation.m_pitchDegrees *= -1;
		g_theRenderer->BeginCamera(m_plane->m_planeCamera);
		RenderChunks(Vec4(0.f, 0.f, 1.f, -270.f), 1);
 		g_theRenderer->SetBackBufferRenderTarget();
		m_plane->m_planeCamera.m_position.z += distance;
		m_plane->m_planeCamera.m_orientation.m_pitchDegrees *= -1;
		
		//refraction
 		g_theRenderer->SetRenderTarget(g_theGame->m_waterRefractionRenderTexure);
 		g_theRenderer->ClearRenderTarget(g_theGame->m_waterRefractionRenderTexure, clearColor);
 		g_theRenderer->BeginCamera(m_plane->m_planeCamera);
 		RenderChunks(Vec4(0.f, 0.f, 1.f, -270.f), 0);
 		g_theRenderer->SetBackBufferRenderTarget();

		//original render back buffer
		g_theRenderer->BeginCamera(m_plane->m_planeCamera);
		RenderChunks(Vec4(0.f, 0.f, -1.f, -10.f), 2);
		RenderWaterQuad();
		RenderRunWay();
		RenderPlane();
		
		g_theRenderer->EndCamera(m_plane->m_planeCamera);
	}

	g_theRenderer->BeginCamera(g_theGame->m_screenCamera);
	
//     	g_theRenderer->BindRenderTexture(g_theGame->m_waterReflectionRenderTexure);
//    	g_theRenderer->BindRenderTexture(g_theGame->m_waterReflectionRenderTexure,1);
//     	g_theRenderer->BindShader(m_waterLightingShader);
//     	g_theRenderer->DrawVertexArray((int)m_reflectionRTVQuadVerts.size(), m_reflectionRTVQuadVerts.data());
//    	g_theRenderer->BindRenderTexture(nullptr, 0);
//     	g_theRenderer->BindRenderTexture(nullptr, 1);
//     	g_theRenderer->BindShader(nullptr);
//   
//    	g_theRenderer->BindRenderTexture(g_theGame->m_waterRefractionRenderTexure);
//   	g_theRenderer->BindRenderTexture(g_theGame->m_waterRefractionRenderTexure,1);
//    	g_theRenderer->BindShader(m_waterLightingShader);
//    	g_theRenderer->DrawVertexArray((int)m_refractionRTVQuadVerts.size(), m_refractionRTVQuadVerts.data());
//    	g_theRenderer->BindRenderTexture(nullptr);
//   	g_theRenderer->BindRenderTexture(nullptr, 1);
//    	g_theRenderer->BindShader(nullptr);

	RenderAllUI();
	
	g_theRenderer->EndCamera(g_theGame->m_screenCamera);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool Map::ActivateNearestMissingChunk()
{
	Vec3 playerPosition = GetPlaneWorldPosition();
	IntVec2 playerChunkCoords = Chunk::GetChunkCoordinatesForWorldPosition(playerPosition, m_chunkSize);
	int bufferZoneWidth = 5;
	int activationRadius = (int)m_chunkActivationRange;
	int effectiveActivationRadius = activationRadius - bufferZoneWidth;
	bool chunkActivated = false;

	for (int y = -activationRadius; y <= activationRadius; ++y)
	{
		for (int x = -activationRadius; x <= activationRadius; ++x)
		{
			if (x * x + y * y <= effectiveActivationRadius * effectiveActivationRadius)
			{
				IntVec2 currentCoords = playerChunkCoords + IntVec2(x, y);

				if (IsChunkWithinBounds(currentCoords) && !IsChunkActive(currentCoords))
				{
					InitializeChunk(currentCoords);
					chunkActivated = true;
				}
			}
		}
	}

	return chunkActivated;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Map::InitializeChunk(IntVec2 const& coords)
{
	Chunk* newChunk = new Chunk(coords);
	if (newChunk)
	{
		m_chunks.push_back(newChunk);
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Map::ActivateChunk(Chunk* chunk)
{
	m_chunkMutex.lock();
	
	if (chunk)
	{
		m_chunks.push_back(chunk);
	}
	
	m_chunkMutex.unlock();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Map::UpdateChunk(float deltaSeconds)
{
	for (int i = 0; i < m_chunks.size(); i++)
	{
		Chunk* chunk = m_chunks[i];
		if (chunk)
		{
			chunk->Update(deltaSeconds);
		}
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Map::RenderChunks(Vec4 const& clippingPlane,  int cull)
{
	for (int i = 0; i < m_chunks.size(); i++)
	{
		Chunk* chunk = m_chunks[i];
		if (chunk)
		{
			chunk->Render(clippingPlane, cull);
		}
	}

}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Map::DeactivateFurthestChunk()
{
	if (m_chunks.empty())
	{
		return;
	}

	int bufferZoneWidth = 5;
	int effectiveDeactivationRadius = m_chunkDeactivationRange + bufferZoneWidth;
	Vec3 playerPosition = GetPlaneWorldPosition();

	std::priority_queue<ChunkDistance> chunkQueue;

	for (Chunk* chunk : m_chunks)
	{
		float distanceSq = CalculateDistanceToChunkBoundary(playerPosition, chunk);
		if (distanceSq > (effectiveDeactivationRadius * effectiveDeactivationRadius))
		{
			chunkQueue.push(ChunkDistance(distanceSq, chunk));
		}
	}

	if (!chunkQueue.empty())
	{
		Chunk* furthestChunk = chunkQueue.top().chunk;
		auto it = std::find(m_chunks.begin(), m_chunks.end(), furthestChunk);
		if (it != m_chunks.end())
		{
			delete* it;
			m_chunks.erase(it);
		}
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Map::ClearChunkVector()
{
	for (int i = 0; i < m_chunks.size(); i++)
	{
		Chunk* chunk = m_chunks[i];
		if (chunk)
		{
			m_octree->RemoveChunk(chunk);
			delete m_chunks[i];
			m_chunks[i] = nullptr;
		}
	}
	m_chunks.clear();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec3 Map::GetPlaneWorldPosition()
{
	Vec3 playerWorldPos;

	if (m_plane)
	{
		playerWorldPos = m_plane->m_planePosition;
	}

	return playerWorldPos;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool Map::IsChunkWithinBounds(IntVec2 const& chunkCoords)
{
	IntVec2 worldDimensionsInChunks = g_theGame->m_currentHeightMapImage.GetDimensions();
	worldDimensionsInChunks.x /= CHUNK_SIZE_TILES;
	worldDimensionsInChunks.y /= CHUNK_SIZE_TILES;

	if ((chunkCoords.x >= 0) && (chunkCoords.y >= 0) && (chunkCoords.x < worldDimensionsInChunks.x) && (chunkCoords.y < worldDimensionsInChunks.y))
	{
		return true;
	}

	return false;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool Map::IsChunkActive(IntVec2 const& chunkCoords)
{
	for (Chunk* chunk : m_chunks)
	{
		if (chunk && chunk->m_chunkCoords == chunkCoords)
		{
			return true;
		}
	}
	return false;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool Map::IsSphereCollidingWithChunk(const Vec3& sphereCenter, float sphereRadius,  Chunk* chunk)
{
	for (Vertex_PCUTBN& vertex : chunk->m_terrainVerts)
	{
		Vec3 vertexPosition = Vec3(vertex.m_position.x, vertex.m_position.y, vertex.m_position.z);
		float distanceSq = (vertexPosition - sphereCenter).GetLengthSquared();
		if (distanceSq <= ( sphereRadius * sphereRadius) )
		{
			return true;
		}
	}
	return false;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Chunk* Map::FindChunkByCoordinates(IntVec2 chunkCoords)
{
	Chunk* chunkToBeFound = nullptr;
	for (Chunk* chunk : m_chunks)
	{
		if (chunk)
		{
			if (chunk->m_chunkCoords == chunkCoords)
			{
				chunkToBeFound = chunk;
			}
		}
	}

	return chunkToBeFound;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool Map::CheckSphereCollisionWithTerrain(const Vec3& sphereCenter, float sphereRadius)
{
	std::vector<Chunk*> chunksToCheck = GetRelevantChunksWhichMightCollideWithPlane(sphereCenter, sphereRadius);

	for (Chunk* chunk : chunksToCheck)
	{
		if (chunk)
		{
			if (IsSphereCollidingWithChunk(sphereCenter, sphereRadius, chunk))
			{
				return true; 
			}
		}
	}
	return false;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
std::vector<Chunk*> Map::GetRelevantChunksWhichMightCollideWithPlane(const Vec3& sphereCenter, float sphereRadius)
{
	std::vector<Chunk*> relevantChunks;

	IntVec2 chunkCenterCoords = Chunk::GetChunkCoordinatesForWorldPosition(sphereCenter, m_chunkSize);

	int chunkRadius = (int)(ceil(sphereRadius / CHUNK_SIZE_METERS));
	for (int yOffset = -chunkRadius; yOffset <= chunkRadius; yOffset++) 
	{
		for (int xOffset = -chunkRadius; xOffset <= chunkRadius; xOffset++)
		{
			IntVec2 chunkCoords = chunkCenterCoords + IntVec2(xOffset, yOffset);

			Chunk* chunk = FindChunkByCoordinates(chunkCoords);
			if (chunk) 
			{
				relevantChunks.push_back(chunk);
			}
		}
	}

	return relevantChunks;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Map::CheckIfPlaneHasCollidedWithTerrain()
{
	if (m_plane)
	{
		bool didCollide = CheckSphereCollisionWithTerrain(m_plane->m_collisionSphereCenter, m_plane->m_collisionSphereRadius);
		if (didCollide)
		{
			m_isDead = true;
		}
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Map::DestroyPlaneIfDead(float deltaSeconds)
{
	if (m_isDead)
	{
		m_planeDeathTimer += deltaSeconds;
		if (m_planeDeathTimer >= 2.1f)
		{
			delete m_plane;
			m_plane = nullptr;

			m_isDead = false;
			m_planeIsReadyToSpawn = true;
			m_planeDeathTimer = 0.0f;
		}
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Map::RespawnPlane(float deltaSeconds)
{
	(void)deltaSeconds;
	if (m_planeIsReadyToSpawn)
	{
		InitializePlane();
		m_planeIsReadyToSpawn = false;
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Map::InitializePlane()
{
	std::string path = "Data/Models/FighterJett.xml";
	m_plane = new Plane(path.c_str());
	if (m_plane)
	{
		m_plane->Startup();
	}

	SetWorldCamera();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Map::SetWorldCamera()
{
	m_plane->m_planeCamera.m_mode = Camera::eMode_Perspective;
	m_plane->m_planeCamera.SetPerspectiveView(g_theWindow->m_config.m_clientAspect, 60.0f, 10.f, 10000.0f);
	Vec3 Ibasis(0.f, 0.f, 1.f);
	Vec3 JBasis(-1.f, 0.f, 0.f);
	Vec3 Kbasis(0.f, 1.f, 0.f);
	m_plane->m_planeCamera.SetRenderBasis(Ibasis, JBasis, Kbasis);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Map::RenderGameOverUI()
{
	if (m_isDead)
	{
		m_gameOverVerts.clear();

		AddVertsForAABB2D(m_gameOverVerts, m_gameOverBounds, Rgba8::RED);
		g_theRenderer->BindShader(nullptr);
		g_theRenderer->BindTexture(nullptr);
		g_theRenderer->DrawVertexArray((int)m_gameOverVerts.size(), m_gameOverVerts.data());


		std::string text = Stringf("You dead. Trash pilot.");
		m_UIFont->AddVertsForText2D(m_youDeadVerts, Vec2(610.f, 390.f), 25.f, text, Rgba8(255, 255, 255, 255), 0.7f);

		g_theRenderer->BindShader(nullptr);
		g_theRenderer->BindTexture(&m_UIFont->GetTexture());
		g_theRenderer->DrawVertexArray((int)m_youDeadVerts.size(), m_youDeadVerts.data());
		g_theRenderer->BindTexture(nullptr);


		
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Map::InitializeRunway()
{
	AABB3 runWayBounds = AABB3(Vec3(3000.f, 0.f, 10.f), Vec3(3300.f, 1000.f, 100.f));
	AddVertsForAABB3D(m_runWayVerts, runWayBounds, Rgba8::GREY);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Map::RenderRunWay()
{
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray((int)m_runWayVerts.size(), m_runWayVerts.data());
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Map::RenderAllUI()
{
	RenderGameOverUI();
	RenderFPSUI();
	RenderLRIndicatorShape();
	if (m_plane && !m_isDead)
	{
		RenderPlaneVelocitySpeedUI();
	}
	
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Map::RenderFPSUI()
{
	m_fpsTextVerts.clear();
	std::string fpsText = Stringf("FPS: %.2f", 1.f / m_fpsMs);

	m_UIFont->AddVertsForText2D(m_fpsTextVerts, Vec2(1470.f, 781.f), 15.f, fpsText, Rgba8(255, 255, 0, 255), 0.7f);

	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(&m_UIFont->GetTexture());
	g_theRenderer->DrawVertexArray((int)m_fpsTextVerts.size(), m_fpsTextVerts.data());
	g_theRenderer->BindTexture(nullptr);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Map::InitializeUIFont()
{
	std::string fontName = "SquirrelFixedFont";
	m_UIFont = g_theRenderer->CreateOrGetBitmapFontFromFile(("Data/Images/" + fontName).c_str());
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Map::InitializeLRIndicatorShape()
{
	Vec2 lineR1(1100.f, 380.f);
	Vec2 lineR2(1100.f, 420.f);
	Vec2 lineR3(1030.f, 420.f);
	Vec2 lineR4(1000.f, 400.f);
	Vec2 lineR5(1030.f, 380.f);

	AddVertsForLineSegment2D(m_LRIndicatorVerts, lineR1, lineR2, 2.f, Rgba8::GREEN);
	AddVertsForLineSegment2D(m_LRIndicatorVerts, lineR2, lineR3, 2.f, Rgba8::GREEN);
	AddVertsForLineSegment2D(m_LRIndicatorVerts, lineR3, lineR4, 2.f, Rgba8::GREEN);
	AddVertsForLineSegment2D(m_LRIndicatorVerts, lineR4, lineR5, 2.f, Rgba8::GREEN);
	AddVertsForLineSegment2D(m_LRIndicatorVerts, lineR5, lineR1, 2.f, Rgba8::GREEN);

	Vec2 lineL1(800.f - (lineR1.x - 800.f), lineR1.y);
	Vec2 lineL2(800.f - (lineR2.x - 800.f), lineR2.y);
	Vec2 lineL3(800.f - (lineR3.x - 800.f), lineR3.y);
	Vec2 lineL4(800.f - (lineR4.x - 800.f), lineR4.y);
	Vec2 lineL5(800.f - (lineR5.x - 800.f), lineR5.y);

	AddVertsForLineSegment2D(m_LRIndicatorVerts, lineL1, lineL2, 2.f, Rgba8::GREEN);
	AddVertsForLineSegment2D(m_LRIndicatorVerts, lineL2, lineL3, 2.f, Rgba8::GREEN);
	AddVertsForLineSegment2D(m_LRIndicatorVerts, lineL3, lineL4, 2.f, Rgba8::GREEN);
	AddVertsForLineSegment2D(m_LRIndicatorVerts, lineL4, lineL5, 2.f, Rgba8::GREEN);
	AddVertsForLineSegment2D(m_LRIndicatorVerts, lineL5, lineL1, 2.f, Rgba8::GREEN);

}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Map::RenderLRIndicatorShape()
{
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray((int)m_LRIndicatorVerts.size(), m_LRIndicatorVerts.data());
	g_theRenderer->BindTexture(nullptr);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Map::RenderPlaneVelocitySpeedUI()
{
	m_planeVelocityUIVerts.clear();
	std::string velocityText = Stringf("%.2f", m_plane->m_planeVelocity.GetLength());
	std::string altitudeText = Stringf("%.2f", m_plane->m_planePosition.z);
	m_UIFont->AddVertsForText2D(m_planeVelocityUIVerts, Vec2(505.f, 390.f), 15.f, velocityText, Rgba8(255, 255, 0, 255), 0.7f);
	m_UIFont->AddVertsForText2D(m_planeVelocityUIVerts, Vec2(1020.f,390.f), 15.f, altitudeText, Rgba8(255, 255, 0, 255), 0.7f);

	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(&m_UIFont->GetTexture());
	g_theRenderer->DrawVertexArray((int)m_planeVelocityUIVerts.size(), m_planeVelocityUIVerts.data());
	g_theRenderer->BindTexture(nullptr);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Map::CheckForCompletedJobs()
{
	Job* completedJob = g_theJobSystem->RetrieveCompletedJobs();

	while (completedJob)
	{
		if (completedJob->m_jobType == CHUNK_GENERATION_JOB_TYPE)
		{
			ChunkGenerationJob* chunkJob = (ChunkGenerationJob*)completedJob;
			if (chunkJob)
			{
				Chunk* chunk = chunkJob->m_chunk;

				if (chunk)
				{
					if (chunk)
					{
						ActivateChunk(chunk);
					}
				}
			}
		}

		delete completedJob;
		completedJob = nullptr;

		completedJob = g_theJobSystem->RetrieveCompletedJobs();
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Map::UpdatePlane(float deltaSeconds)
{
	if (m_plane)
	{
		m_plane->Update(deltaSeconds);
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Map::RenderPlane()
{
	if (m_plane)
	{
		m_plane->Render();
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
float Map::CalculateDistanceToChunkBoundary(const Vec3& playerPosition, const Chunk* chunk)
{
	IntVec2 chunkHalfSize = m_chunkSize * 0.5f;
	Vec2 chunkCenter = Chunk::GetWorldPositionForChunkCoordinate(chunk->m_chunkCoords, m_chunkSize);

	Vec2 chunkMin = chunkCenter - (Vec2)chunkHalfSize;
	Vec2 chunkMax = chunkCenter + (Vec2)chunkHalfSize;

	Vec3 nearestPoint = Vec3(std::max(chunkMin.x, std::min(playerPosition.x, chunkMax.x)), std::max(chunkMin.y, std::min(playerPosition.y, chunkMax.y)), playerPosition.z );

	float distanceToChunkBoundary = (nearestPoint - playerPosition).GetLengthSquared();
	return distanceToChunkBoundary;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Map::InitializeRenderTextureQuad()
{
	//Reflection
	AABB2 bounds = AABB2(0, 0, 600.f, 400.f);
	AABB2 UVs = AABB2(0, 1, 1, 0);
	AddVertsForAABB2D(m_reflectionRTVQuadVerts, bounds, Rgba8::WHITE, UVs);


	//Refraction
	AABB2 bounds2 = AABB2(1000, 0, 1600.f, 400.f);
	AABB2 UVs2 = AABB2(0, 1, 1, 0);
	AddVertsForAABB2D(m_refractionRTVQuadVerts, bounds2, Rgba8::WHITE, UVs2);

}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Map::InitializeWaterQuad()
{
	Vec3 BL(3100.f, 4050.f, 270.f);
	Vec3 BR(3300.f, 4050.f, 270.f);
	Vec3 TL(3100.f, 4250.f, 270.f);
	Vec3 TR(3300.f, 4250.f, 270.f);
	AABB2 UVs = AABB2(0.f, 1.f, 1.f, 0.f);
	AddVertsForIndexedQuadPCUTBN(m_waterVerts, m_waterIndexes, BL, BR, TR, TL, Rgba8::DARKBLUE, UVs);

	m_waterIndexBuffer = g_theRenderer->CreateIndexBuffer(m_waterIndexes.size() * sizeof(m_waterIndexes[0]), sizeof(unsigned int));
	g_theRenderer->CopyCPUToGPU(m_waterIndexes.data(), m_waterIndexes.size() * sizeof(m_waterIndexes[0]), m_waterIndexBuffer);

	m_waterVertexBuffer = g_theRenderer->CreateVertexBuffer(m_waterVerts.size() * sizeof(m_waterVerts[0]), sizeof(Vertex_PCUTBN));
	g_theRenderer->CopyCPUToGPU(m_waterVerts.data(), m_waterVerts.size() * sizeof(m_waterVerts[0]), m_waterVertexBuffer);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Map::RenderWaterQuad()
{
	g_theRenderer->SetLightConstants(Vec3(0.5f, 0.5f, -1.f), 0.3f, 0.7f, m_plane->m_planeCamera.m_position, 0, 0, 1, 32);
  	g_theRenderer->BindRenderTexture(g_theGame->m_waterReflectionRenderTexure, 0);
  	g_theRenderer->BindRenderTexture(g_theGame->m_waterRefractionRenderTexure, 1);
 	g_theRenderer->BindTexture(g_theGame->m_waterDudvTexture, 2);
	g_theRenderer->BindTexture(g_theGame->m_waterNormalSpecTexture, 3);
 	g_theRenderer->BindShader(m_waterLightingShader);
 	g_theRenderer->DrawVertexBuffer((int)m_waterVerts.size(), m_waterVertexBuffer, (int)m_waterIndexes.size(), m_waterIndexBuffer);
 	g_theRenderer->BindRenderTexture(nullptr, 0);
 	g_theRenderer->BindRenderTexture(nullptr, 1);
 	g_theRenderer->BindTexture(nullptr, 2);
	g_theRenderer->BindTexture(nullptr, 3);
 	g_theRenderer->BindShader(nullptr);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Map::CreateClippingPlaneCB()
{
	m_clippingPlaneCBO = g_theRenderer->CreateConstantBuffer(sizeof(ClipingPlane));
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Map::BindClippingPlaneCB()
{
	ClipingPlane cp = {};
	cp.clippingPlane = Vec4(0.f,0.f, -1.f, 210.f);
	cp.time = (float)GetCurrentTimeSeconds();
	cp.cameraPos = m_plane->m_planeCamera.m_position;
	g_theRenderer->CopyCPUtoGPU(&cp, sizeof(ClipingPlane), m_clippingPlaneCBO);
	g_theRenderer->BindConstantBuffer(0, m_clippingPlaneCBO);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
AABB3 Map::CreateQueryBoundsAroundPlayer(Vec3 const& playerPos, float queryRange)
{
	Vec3 minPoint = playerPos - Vec3(queryRange, queryRange, queryRange);
	Vec3 maxPoint = playerPos + Vec3(queryRange, queryRange, queryRange);
	return AABB3(minPoint, maxPoint);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------
OctreeNode::OctreeNode(AABB3 const& bounds)
	:m_nodeBounds(bounds)
{
	for (int i = 0; i < 8; ++i) 
	{
		m_children[i] = nullptr; 
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
OctreeNode::~OctreeNode()
{
	for (int i = 0; i < 8; ++i) 
	{
		delete m_children[i];  
		m_children[i] = nullptr; 
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool OctreeNode::IsNodeLeaf()
{
	if (m_children[0] == nullptr)
	{
		return true;
	}
	else
	{
		return false;
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void OctreeNode::AddChunk(Chunk* chunk)
{
	AABB3 chunkBounds = chunk->GetChunkAABB3();

	if (IsNodeLeaf())
	{
		if (!ShouldSplitNode())
		{
			m_chunks.push_back(chunk);
		}
		else
		{
			SplitNode();
			AddChunk(chunk); //call recursively
		}
	}

	else
	{
		for (int i = 0; i < 8; i++) //Determine which child node to pass chunk to
		{
			if (m_children[i]->m_nodeBounds.DoesContainAABB3(chunkBounds))
			{
				m_children[i]->AddChunk(chunk);
			}
		}

		m_chunks.push_back(chunk); //If no child can completely contain the chunk, keep it in this node
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void OctreeNode::Query(AABB3 const& queryArea, std::vector<Chunk*>& foundChunks)
{
	if (!m_nodeBounds.Intersects(queryArea)) 
	{
		return;
	}

	// Add all chunks from this node that intersect the query area
	for (Chunk* chunk : m_chunks)
	{
		if (chunk)
		{
			if (chunk->GetChunkAABB3().Intersects(queryArea))
			{
				foundChunks.push_back(chunk);
			}
		}
		
	}

	// If this node has children, query them as well
	if (!IsNodeLeaf()) 
	{
		for (int i = 0; i < 8; i++) 
		{
			m_children[i]->Query(queryArea, foundChunks);
		}
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void OctreeNode::RemoveChunk(Chunk* chunk)
{
	if (IsNodeLeaf()) 
	{
		m_chunks.erase(std::remove(m_chunks.begin(), m_chunks.end(), chunk), m_chunks.end());
	}
	else
	{
		// Otherwise, find the child node containing the chunk and remove it there
		for (int i = 0; i < 8; i++) 
		{
			if (m_children[i]->m_nodeBounds.DoesContainAABB3(chunk->GetChunkAABB3()))
			{
				m_children[i]->RemoveChunk(chunk);
				return;
			}
		}
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------;
bool OctreeNode::CanNodeBeSplitFurther()
{
	Vec3 size = m_nodeBounds.GetSize();
	float minSize = MIN_NODE_SIZE;

	if (size.x > minSize && size.y > minSize && size.z > minSize)
	{
		return true;
	}

	return false;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------;
void OctreeNode::SplitNode()
{
	Vec3 center = m_nodeBounds.GetCenter();
	Vec3 min = m_nodeBounds.m_mins;
	Vec3 max = m_nodeBounds.m_maxs;
	Vec3 size = (max - min) * 0.5f;

	for (int i = 0; i < 8; ++i) 
	{
		Vec3 newMin = min;
		Vec3 newMax = center;
		if (i & 1) newMin.x += size.x; else newMax.x -= size.x;
		if (i & 2) newMin.y += size.y; else newMax.y -= size.y;
		if (i & 4) newMin.z += size.z; else newMax.z -= size.z;
		m_children[i] = new OctreeNode(AABB3(newMin, newMax));
	}

	// Reassign existing chunks to new children
	for (Chunk* chunk : m_chunks) 
	{
		for (int i = 0; i < 8; ++i) 
		{
			if (m_children[i]->m_nodeBounds.DoesContainAABB3(chunk->GetChunkAABB3())) 
			{
				m_children[i]->AddChunk(chunk);
				break;
			}
		}
	}
	m_chunks.clear();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool OctreeNode::ShouldSplitNode()
{
	return m_chunks.size() > MAX_CHUNKS_PER_NODE && CanNodeBeSplitFurther();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Octree::Octree(AABB3 const& bounds)
{
	m_rootNode = new OctreeNode(bounds);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Octree::~Octree()
{
	delete m_rootNode;
	m_rootNode = nullptr;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Octree::AddChunk(Chunk* chunk)
{
	if (chunk)
	{
		if (m_rootNode)
		{
			m_rootNode->AddChunk(chunk);
		}
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Octree::Query(AABB3 const& queryArea, std::vector<Chunk*>& foundChunks)
{
	if (m_rootNode)
	{
		m_rootNode->Query(queryArea, foundChunks);
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Octree::RemoveChunk(Chunk* chunk)
{
	if (chunk)
	{
		if (m_rootNode)
		{
			m_rootNode->RemoveChunk(chunk);
		}
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------