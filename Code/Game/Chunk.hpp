#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/JobSystem.hpp"
#include "Engine/Math/Vec4.hpp"
//--------------------------------------------------------------------------------------------------------------------------------------------------------
class Map;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
constexpr float TILE_SIZE_METERS = 10.f;
constexpr int CHUNK_SIZE_TILES = 10;
constexpr float CHUNK_SIZE_METERS = TILE_SIZE_METERS * CHUNK_SIZE_TILES;
const float HEIGHT_THRESHOLD = 10.f;
const float TERRAIN_MAX_HEIGHT = 1000.f;
//-------------------------------------------------------------------------------------------------------------------------------------------------------
struct TimeCB
{
	Vec4 cameraWorldPos;
	float time;
	float fogStartDistance;
	float fogEndDistance;
	float fogMaxAlpha;
	int cull;
	int padding[3];
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
	void			Render(Vec4 const& clippingPlane, int cull);
	void			GenerateChunk();
	void			CreateVertexAndIndexBufferForChunk();
	static IntVec2  GetChunkCoordinatesForWorldPosition(const Vec3& position, IntVec2 chunkSize);
	static Vec2	    GetChunkCenterXYForChunkCoords(const IntVec2& chunkCoords, IntVec2 chunkSize);
	static Vec2		GetWorldPositionForChunkCoordinate(IntVec2 const& localChunkCoords, IntVec2 chunkSize);
	float			GetHeightAtWorldCoordinate(IntVec2 const& coords);
	void			CreateTimeCB();
	void			BindCBForTime(int cull);
	void			RenderChunkAABB3();
	AABB3			GetChunkAABB3();

	VertexBuffer*				m_terrainVertexBuffer = nullptr;
	Shader*						m_waterShader = nullptr;
	ConstantBuffer*				m_timeCBo = nullptr;
	ConstantBuffer* m_clippingPlaneCBO = nullptr;
	IndexBuffer*				m_terrainIndexBuffer = nullptr;
	IntVec2		                m_dimensions = IntVec2(-1, -1);
	IntVec2						m_chunkCoords = IntVec2(-1, -1);
	std::vector<Vertex_PCUTBN>	m_terrainVerts;
	std::vector<unsigned int>   m_terrainIndexes;
	float		                m_maxTerrainHeight = TERRAIN_MAX_HEIGHT;
	float						m_fogStart = 2500.f;
	float						m_fogEnd = 700.f;
	float						m_fogMaxAlpha = 1.f;
	bool						m_shouldDrawChunk = false;
	AABB3						m_chunkAABB3;
	std::vector<Vertex_PCU> m_chunkAABB3Verts;
	bool					m_showChunkAABB3 = false;
	bool	m_isActivated = false;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
constexpr int CHUNK_GENERATION_JOB_TYPE = 1;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
class ChunkGenerationJob : public Job 
{
public:
	ChunkGenerationJob(Chunk* chunk);
	virtual void Execute() override;
	virtual void OnFinished() override;

	Chunk* m_chunk = nullptr;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------