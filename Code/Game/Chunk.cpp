#include "Game/Chunk.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Renderer/ConstantBuffer.hpp"
#include "Engine/Core/Time.hpp"
#include "Game/Map.hpp"
#include "Game/Game.hpp"
#include <algorithm>
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Chunk::Chunk(IntVec2 const& chunkCoords)
{
	m_chunkCoords = chunkCoords;
	m_dimensions = g_theGame->m_currentHeightMapImage.GetDimensions();
	
	Startup();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Chunk::~Chunk()
{
	delete m_terrainVertexBuffer;
	m_terrainVertexBuffer = nullptr;

	delete m_terrainIndexBuffer;
	m_terrainIndexBuffer = nullptr;

	delete m_timeCBo;
	m_timeCBo = nullptr;

	delete m_clippingPlaneCBO;
	m_clippingPlaneCBO = nullptr;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Chunk::Startup()
{
	GenerateChunk();
	CreateVertexAndIndexBufferForChunk();
	CreateTimeCB();
	m_waterShader = g_theRenderer->CreateShaderOrGetFromFile("Data/Shaders/ChunkShader", VertexType::PCUTBN);
	m_clippingPlaneCBO = g_theRenderer->CreateConstantBuffer(sizeof(ClipingPlane));
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Chunk::Shutdown()
{

}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Chunk::Update(float deltaSeconds)
{
	(void)deltaSeconds;

	if (m_isActivated)
	{
		if (g_theInput->WasKeyJustPressed('K'))
		{
			m_showChunkAABB3 = !m_showChunkAABB3;
		}
	}
	

}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Chunk::Render(Vec4 const& clippingPlane, int cull)
{
// 	if (m_isActivated)
// 	{
		BindCBForTime(cull);

		ClipingPlane cp = {};
		cp.clippingPlane = clippingPlane;

		g_theRenderer->CopyCPUtoGPU(&cp, sizeof(ClipingPlane), m_clippingPlaneCBO);
		g_theRenderer->BindConstantBuffer(4, m_clippingPlaneCBO);

		g_theRenderer->SetRasterizerModes(RasterizerMode::WIREFRAME_CULL_NONE);
		//g_theRenderer->SetDepthModes(DepthMode::ENABLED);
		g_theRenderer->SetLightConstants(Vec3(0.0f, 0.0f, -1.f), 0.9f, 0.1f, g_theGame->m_map->m_plane->m_planePosition, 1, 0, 0, 0);
		g_theRenderer->BindTexture(g_theGame->m_dirtGrassDiffuseTexture, 0);
		g_theRenderer->BindTexture(g_theGame->m_dirtGrassNormalTexture, 1);
		g_theRenderer->BindTexture(g_theGame->m_snowDiffuseTexture, 2);
		g_theRenderer->BindTexture(g_theGame->m_snowNormalTexture, 3);
		g_theRenderer->BindTexture(g_theGame->m_waterDiffuseTexture, 4);
		g_theRenderer->BindTexture(g_theGame->m_waterNormalTexture, 5);
		g_theRenderer->BindShader(m_waterShader);
		g_theRenderer->DrawVertexBuffer((int)m_terrainVerts.size(), m_terrainVertexBuffer, (int)m_terrainIndexes.size(), m_terrainIndexBuffer);
		g_theRenderer->BindTexture(nullptr, 0);
		g_theRenderer->BindTexture(nullptr, 1);
		g_theRenderer->BindTexture(nullptr, 2);
		g_theRenderer->BindTexture(nullptr, 3);
		g_theRenderer->BindTexture(nullptr, 4);
		g_theRenderer->BindTexture(nullptr, 5);
		g_theRenderer->BindShader(nullptr);
		//g_theRenderer->SetDepthModes(DepthMode::DISABLED);
		g_theRenderer->SetRasterizerModes(RasterizerMode::SOLID_CULL_NONE);
		if (m_showChunkAABB3)
		{
			RenderChunkAABB3();
		}
	//}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Chunk::GenerateChunk()
{
	float textureScale = 8.f;

	int minX = m_chunkCoords.x * CHUNK_SIZE_TILES;
	int minY = m_chunkCoords.y * CHUNK_SIZE_TILES;
	int maxX = minX + CHUNK_SIZE_TILES;
	int maxY = minY + CHUNK_SIZE_TILES;
	float minZ = 0.f;
	float maxZ = 0.f;
	maxX = std::min(maxX, m_dimensions.x);
	maxY = std::min(maxY, m_dimensions.y);

	for (int localTileY = 0; localTileY < CHUNK_SIZE_TILES; localTileY++)
	{
		for (int localTileX = 0; localTileX < CHUNK_SIZE_TILES; localTileX++)
		{
			int worldTileX = m_chunkCoords.x * CHUNK_SIZE_TILES + localTileX;
			int worldTileY = m_chunkCoords.y * CHUNK_SIZE_TILES + localTileY;

			// Calculate world positions of the tile corners
			float tileMinX = TILE_SIZE_METERS * worldTileX;
			float tileMinY = TILE_SIZE_METERS * worldTileY;
			float tileMaxX = tileMinX + TILE_SIZE_METERS;
			float tileMaxY = tileMinY + TILE_SIZE_METERS;

			// Calculate terrain height at each corner using the global height map
			float swZ = GetHeightAtWorldCoordinate(IntVec2(worldTileX, worldTileY));
			float seZ = GetHeightAtWorldCoordinate(IntVec2(worldTileX + 1, worldTileY));
			float nwZ = GetHeightAtWorldCoordinate(IntVec2(worldTileX, worldTileY + 1));
			float neZ = GetHeightAtWorldCoordinate(IntVec2(worldTileX + 1, worldTileY + 1));

			// Calculate vertex positions at each corner
			Vec3 SW(tileMinX, tileMinY, swZ);
			Vec3 SE(tileMaxX, tileMinY, seZ);
			Vec3 NW(tileMinX, tileMaxY, nwZ);
			Vec3 NE(tileMaxX, tileMaxY, neZ);

			Vec2 uvMins = Vec2(static_cast<float>(localTileX - minX) / textureScale, static_cast<float>(localTileY - minY) / textureScale);
			Vec2 uvMaxs = uvMins + Vec2(1.f / textureScale, 1.f / textureScale);
			AABB2 UVs = AABB2(uvMins, uvMaxs);

			Rgba8 color = Rgba8::WHITE;
			if (swZ <= HEIGHT_THRESHOLD && seZ <= HEIGHT_THRESHOLD && nwZ <= HEIGHT_THRESHOLD && neZ <= HEIGHT_THRESHOLD)
			{
				color = Rgba8(0, 0, 255, 255);
			}

			AddVertsForIndexedQuadPCUTBN(m_terrainVerts, m_terrainIndexes, SW, SE, NE, NW, color, UVs);

			minZ = std::min({ minZ, swZ, seZ, nwZ, neZ });
			maxZ = std::max({ maxZ, swZ, seZ, nwZ, neZ });
		}
	}

	Vec3 chunkMin = Vec3(TILE_SIZE_METERS * minX, TILE_SIZE_METERS * minY, minZ);
	Vec3 chunkMax = Vec3(TILE_SIZE_METERS * maxX, TILE_SIZE_METERS * maxY, maxZ);
	m_chunkAABB3 = AABB3(chunkMin, chunkMax);

	AddVertsForAABB3D(m_chunkAABB3Verts,m_chunkAABB3);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Chunk::CreateVertexAndIndexBufferForChunk()
{
	m_terrainIndexBuffer = g_theRenderer->CreateIndexBuffer(m_terrainIndexes.size() * sizeof(m_terrainIndexes[0]), sizeof(unsigned int));
	g_theRenderer->CopyCPUToGPU(m_terrainIndexes.data(), m_terrainIndexes.size() * sizeof(m_terrainIndexes[0]), m_terrainIndexBuffer);

	m_terrainVertexBuffer = g_theRenderer->CreateVertexBuffer(m_terrainVerts.size() * sizeof(m_terrainVerts[0]), sizeof(Vertex_PCUTBN));
	g_theRenderer->CopyCPUToGPU(m_terrainVerts.data(), m_terrainVerts.size() * sizeof(m_terrainVerts[0]), m_terrainVertexBuffer);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
IntVec2 Chunk::GetChunkCoordinatesForWorldPosition(const Vec3& position, IntVec2 chunkSize)
{
	const float scaleFactor = 1.0f;

	int chunkX = static_cast<int>(floor((position.x * scaleFactor) / chunkSize.x));
	int chunkY = static_cast<int>(floor((position.y * scaleFactor) / chunkSize.y));

	return IntVec2(chunkX, chunkY);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec2 Chunk::GetChunkCenterXYForChunkCoords(const IntVec2& chunkCoords, IntVec2 chunkSize)
{
	float centerX = (chunkCoords.x * chunkSize.x) + (chunkSize.x / 2.0f);
	float centerY = (chunkCoords.y * chunkSize.y) + (chunkSize.y / 2.0f);

	return Vec2(centerX, centerY);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Vec2 Chunk::GetWorldPositionForChunkCoordinate(IntVec2 const& localChunkCoords, IntVec2 chunkSize)
{
	float worldX = static_cast<float>(localChunkCoords.x * chunkSize.x);
	float worldY = static_cast<float>(localChunkCoords.y * chunkSize.y);
	return Vec2(worldX, worldY);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
float Chunk::GetHeightAtWorldCoordinate(IntVec2 const& coords)
{
	int worldheightMapX = (coords.x % m_dimensions.x + m_dimensions.x) % m_dimensions.x;
	int worldHeightMapY = (coords.y % m_dimensions.y + m_dimensions.y) % m_dimensions.y;
	IntVec2 heightMapCoords = IntVec2(worldheightMapX, worldHeightMapY);

	Rgba8 pixelColor = g_theGame->m_currentHeightMapImage.GetTexelColor(heightMapCoords);
	float height = RangeMap((float)pixelColor.r, 0.f, 255.f, 0.f, m_maxTerrainHeight);

	return height;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Chunk::CreateTimeCB()
{
	m_timeCBo = g_theRenderer->CreateConstantBuffer(sizeof(TimeCB));
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Chunk::BindCBForTime(int cull)
{
	TimeCB cb = {};
	cb.cameraWorldPos = g_theGame->m_map->m_plane->m_planePosition;
	cb.time = (float)GetCurrentTimeSeconds();
	cb.fogStartDistance = m_fogStart;
	cb.fogEndDistance = m_fogEnd;
	cb.fogMaxAlpha = m_fogMaxAlpha;
	cb.cull = cull;
	g_theRenderer->CopyCPUtoGPU(&cb, sizeof(TimeCB), m_timeCBo);
	g_theRenderer->BindConstantBuffer(0, m_timeCBo);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Chunk::RenderChunkAABB3()
{
	g_theRenderer->SetRasterizerModes(RasterizerMode::WIREFRAME_CULL_BACK);
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->DrawVertexArray((int)m_chunkAABB3Verts.size(), m_chunkAABB3Verts.data());
	g_theRenderer->SetRasterizerModes(RasterizerMode::SOLID_CULL_NONE);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
AABB3 Chunk::GetChunkAABB3()
{
	return m_chunkAABB3;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
