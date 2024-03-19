#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Core/Material.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/JobSystem.hpp"
//--------------------------------------------------------------------------------------------------------------------------------------------------------
class Map;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
constexpr float TILE_SIZE_METERS = 100.f;
constexpr int CHUNK_SIZE_TILES = 100;
constexpr float CHUNK_SIZE_METERS = TILE_SIZE_METERS * CHUNK_SIZE_TILES;
const float HEIGHT_THRESHOLD = 100.f;
//-------------------------------------------------------------------------------------------------------------------------------------------------------
struct TimeCB
{
	float time;
	float padding[3];
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
class Chunk
{
public:
	Chunk(IntVec2 const& chunkCoords);
	~Chunk();

	void			Startup();
	void			Shutdown();
	void			Update(float deltaSeconds);
	void			Render();
	void			GenerateChunk();
	void			CreateVertexAndIndexBufferForChunk();
	static IntVec2  GetChunkCoordinatesForWorldPosition(const Vec3& position, IntVec2 chunkSize);
	static Vec2	    GetChunkCenterXYForChunkCoords(const IntVec2& chunkCoords, IntVec2 chunkSize);
	float			GetHeightAtWorldCoordinate(IntVec2 const& coords);
	void			CreateTimeCB();
	void			BindCBForTime();
	
	VertexBuffer*				m_terrainVertexBuffer = nullptr;
	Image		                m_currentHeightMapImage;
	IntVec2		                m_dimensions = IntVec2(-1, -1);
	float		                m_maxTerrainHeight = 10000.f;
	std::vector<Vertex_PCUTBN>	m_terrainVerts;
	std::vector<unsigned int>   m_terrainIndexes;
	IndexBuffer*				m_terrainIndexBuffer = nullptr;
	IntVec2						m_chunkCoords = IntVec2(-1, -1);
	Shader*						m_waterShader = nullptr;
	ConstantBuffer*				m_timeCBo = nullptr;
	bool						m_shouldApplyTexture = true;
	Material*					m_material = nullptr;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
