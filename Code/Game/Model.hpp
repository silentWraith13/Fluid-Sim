#pragma once
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Core/CPUMesh.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Math/Mat44.hpp"
#include <unordered_map>
//--------------------------------------------------------------------------------------------------------------------------------------------------------
class Model
{
public:
	Model(const char* filePath);
	~Model();

	void InitializeDefinitions(const char* filePath);
	void Render(const Mat44& transform, const Rgba8& color);
	static  Model* GetOrCreateModel(const std::string& modelName);
	static void ClearCache();

	GPUMesh* m_gpuMesh = nullptr;
	CPUMesh* m_cpuMeshh = nullptr;
	Shader* m_shader = nullptr;
	float m_parseTime = 0.f;
	float m_creationtime = 0.f;
	static std::unordered_map<std::string, Model*> s_modelCache;
	
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------