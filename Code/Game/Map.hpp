#pragma once
#include "Game/GameCommon.hpp"
#include "Game/Chunk.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/RenderTexture.hpp"
#include "Engine/Core/Image.hpp"
#include "Game/Plane.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
struct ChunkDistance
{
	float distanceSq;
	Chunk* chunk;
	ChunkDistance(float d, Chunk* c) : distanceSq(d), chunk(c) {}
	bool operator<(const ChunkDistance& other) const { return distanceSq < other.distanceSq; } 
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
struct ClipingPlane
{
	Vec4 clippingPlane;
	float time;
	Vec3 cameraPos;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
class OctreeNode
{
public:
	OctreeNode(AABB3 const& bounds);
	~OctreeNode();
	
	bool IsNodeLeaf();
	void AddChunk(Chunk* chunk);
	void Query(AABB3 const& queryArea, std::vector<Chunk*>& foundChunks);
	void RemoveChunk(Chunk* chunk);
	bool CanNodeBeSplitFurther();
	void SplitNode();
	bool ShouldSplitNode();

	AABB3 m_nodeBounds;
	OctreeNode* m_children[8];
	std::vector<Chunk*> m_chunks;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
class Octree
{
public:
	Octree(AABB3 const& bounds);
	~Octree();
	void AddChunk(Chunk* chunk);
	void Query(AABB3 const& queryArea, std::vector<Chunk*>& foundChunks);
	void RemoveChunk(Chunk* chunk);

	OctreeNode* m_rootNode = nullptr;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
constexpr static int MAX_CHUNKS_PER_NODE = 5;
constexpr static int MIN_NODE_SIZE = 20;

//--------------------------------------------------------------------------------------------------------------------------------------------------------
class Map
{
public:
	Map();
	~Map();

	void Startup();
	void Shutdown();
	void Update(float deltaSeconds);
	void Render();

	bool ActivateNearestMissingChunk();
	void InitializeChunk(IntVec2 const& coords);
	void ActivateChunk(Chunk* chunk);
	void UpdateChunk(float deltaSeconds);
	void RenderChunks(Vec4 const& clippingPlane, int cull);
	void DeactivateFurthestChunk();
	void ClearChunkVector();
	Vec3 GetPlaneWorldPosition();
	bool IsChunkWithinBounds(IntVec2 const& chunkCoords);
	bool IsChunkActive(IntVec2 const& chunkCoords);
	bool IsSphereCollidingWithChunk(const Vec3& sphereCenter, float sphereRadius, Chunk* chunk);
	Chunk* FindChunkByCoordinates(IntVec2 chunkCoords);
	bool CheckSphereCollisionWithTerrain(const Vec3& sphereCenter, float sphereRadius);
	std::vector<Chunk*> GetRelevantChunksWhichMightCollideWithPlane(const Vec3& sphereCenter, float sphereRadius);
	void CheckIfPlaneHasCollidedWithTerrain();
	void DestroyPlaneIfDead(float deltaSeconds);
	void RespawnPlane(float deltaSeconds);
	void InitializePlane();
	void UpdatePlane(float deltaSeconds);
	void RenderPlane();
	void SetWorldCamera();
	void RenderGameOverUI();
	void InitializeRunway();
	void RenderRunWay();
	void RenderAllUI();
	void RenderFPSUI();
	void InitializeUIFont();
	void InitializeLRIndicatorShape();
	void RenderLRIndicatorShape();
	void RenderPlaneVelocitySpeedUI();
	void CheckForCompletedJobs();
	float CalculateDistanceToChunkBoundary(const Vec3& playerPosition, const Chunk* chunk);
	void InitializeRenderTextureQuad();
	void InitializeWaterQuad();
	void RenderWaterQuad();
	void CreateClippingPlaneCB();
	void BindClippingPlaneCB();
	AABB3 CreateQueryBoundsAroundPlayer(Vec3 const& playerPos, float queryRange);

	Plane*					m_plane = nullptr;
	BitmapFont*				m_UIFont = nullptr;
	
	int						m_chunkActivationRange =20;
	int						m_chunkDeactivationRange = 40;
	float					m_planeDeathTimer = 0.f;
	float					m_fpsMs = 0.f;
	IntVec2					m_chunkSize = IntVec2(-1, -1);
	bool					m_isDead = false;
	bool					m_planeIsReadyToSpawn = false;
	Camera					m_worldCamera;
	std::vector<Chunk*>		m_chunks;
	std::vector<Vertex_PCU> m_gameOverVerts;
	std::vector<Vertex_PCU> m_runWayVerts;
	std::vector<Vertex_PCU> m_fpsTextVerts;
	std::vector<Vertex_PCU> m_reflectionRTVQuadVerts;
	std::vector<Vertex_PCU> m_refractionRTVQuadVerts;
	std::vector<Vertex_PCU> m_LRIndicatorVerts;
	std::vector<Vertex_PCU> m_planeVelocityUIVerts;
	std::vector<Vertex_PCU> m_youDeadVerts;
	std::vector<Vertex_PCU> m_cubeSkyBoxVerts;
	std::vector<Vertex_PCUTBN>	m_waterVerts;
	VertexBuffer* m_waterVertexBuffer = nullptr;
	Shader* m_waterLightingShader = nullptr;
	Shader* m_waterRenderShader = nullptr;
	ConstantBuffer* m_clippingPlaneCBO = nullptr;
	IndexBuffer* m_waterIndexBuffer = nullptr;
	std::vector<unsigned int>   m_waterIndexes;
	AABB2					m_gameOverBounds = AABB2(0.f, 0.f, 1600.f, 800.f);
	std::mutex				m_chunkMutex;
	Octree* m_octree = nullptr;

};
//--------------------------------------------------------------------------------------------------------------------------------------------------------