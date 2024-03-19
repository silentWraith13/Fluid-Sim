#include "Game/Chunk.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Renderer/ConstantBuffer.hpp"
#include "Engine/Core/Time.hpp"
#include "Game/Map.hpp"
#include "Game/Game.hpp"
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

// 	delete m_material;
// 	m_material = nullptr;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Chunk::Startup()
{
 	GenerateChunk();
 	CreateVertexAndIndexBufferForChunk();
	CreateTimeCB();
	m_waterShader = g_theRenderer->CreateShaderOrGetFromFile("Data/Shaders/ChunkShader", VertexType::PCUTBN);
// 	m_material = new Material("Data/Materials/Sand.xml");
// 	m_material->InitializeDefinitions("Data/Materials/Sand.xml");
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Chunk::Shutdown()
{

}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Chunk::Update(float deltaSeconds)
{
	(void)deltaSeconds;
	BindCBForTime();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Chunk::Render()
{
	g_theRenderer->SetDepthModes(DepthMode::ENABLED);
	//g_theRenderer->SetLightConstants(Vec3(0.f, 0.f, -1.f), 1.f, 0.f, g_theGame->m_map->m_plane->m_planePosition, 0, 0, 1.f, 32.f);
	//g_theRenderer->BindTextures(m_material->m_diffuseTexture, m_material->m_normalTextures, m_material->m_specGlossEmitTexture);
	//g_theRenderer->BindShader(m_waterShader);
	g_theRenderer->DrawVertexBuffer((int)m_terrainVerts.size(), m_terrainVertexBuffer, (int)m_terrainIndexes.size(), m_terrainIndexBuffer);
	//g_theRenderer->BindTextures(nullptr, nullptr, nullptr);
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->SetDepthModes(DepthMode::DISABLED);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Chunk::GenerateChunk()
{
	float textureScale = 8.f;

	int minX = m_chunkCoords.x * CHUNK_SIZE_TILES;
	int minY = m_chunkCoords.y * CHUNK_SIZE_TILES;
	int maxX = minX + CHUNK_SIZE_TILES;
	int maxY = minY + CHUNK_SIZE_TILES;

	maxX = std::min(maxX, m_dimensions.x);
	maxY = std::min(maxY, m_dimensions.y);

	for (int localTileY = 0; localTileY < CHUNK_SIZE_TILES; localTileY ++) 
	{
		for (int localTileX = 0; localTileX < CHUNK_SIZE_TILES ; localTileX ++) 
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
		}
	}
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
	int chunkX = static_cast<int>(floor(position.x / chunkSize.x));
	int chunkY = static_cast<int>(floor(position.y / chunkSize.y));

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
float Chunk::GetHeightAtWorldCoordinate(IntVec2 const& coords)
{
	Image const& heightMap = g_theGame->m_currentHeightMapImage;
	IntVec2 heightMapDimensions = heightMap.GetDimensions();

	int worldheightMapX = (coords.x % heightMapDimensions.x + heightMapDimensions.x) % heightMapDimensions.x;
	int worldHeightMapY = (coords.y % heightMapDimensions.y + heightMapDimensions.y) % heightMapDimensions.y;
	IntVec2 heightMapCoords = IntVec2(worldheightMapX, worldHeightMapY);

	Rgba8 pixelColor = heightMap.GetTexelColor(heightMapCoords);
	float height = RangeMap((float)pixelColor.r, 0.f, 255.f, 0.f, m_maxTerrainHeight);

	return height;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Chunk::CreateTimeCB()
{
	m_timeCBo = g_theRenderer->CreateConstantBuffer(sizeof(TimeCB));
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Chunk::BindCBForTime()
{
	TimeCB cb = {};
	cb.time = (float)GetCurrentTimeSeconds();
	g_theRenderer->CopyCPUtoGPU(&cb, sizeof(TimeCB), m_timeCBo);
	g_theRenderer->BindConstantBuffer(0, m_timeCBo);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------