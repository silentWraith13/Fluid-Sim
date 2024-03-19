#include "Game/Model.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/OBJLoader.hpp"
#include "Engine/Core/Time.hpp"

//-------------------------------------------------------------------------------------------------------------------------------------------------------
std::unordered_map<std::string, Model*> Model::s_modelCache;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Model::Model(const char* filePath)
{
	XmlDocument modelDefsXML;
	XmlResult result = modelDefsXML.LoadFile(filePath);
	if (result == tinyxml2::XML_ERROR_FILE_NOT_FOUND)
	{
		ERROR_AND_DIE("Could not open XML file");
	}
	
	GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS, Stringf("Failed to open required model \"%s\"", filePath));

	XmlElement* modelDefElement = modelDefsXML.RootElement();
	GUARANTEE_OR_DIE(modelDefElement, Stringf("Failed to open model def element"));

	std::string modelName = ParseXmlAttribute(*modelDefElement, "name", "INVALID MODEL NAME");
	std::string modelPath = ParseXmlAttribute(*modelDefElement, "path", "INVALID MODEL PATH");
	std::string shaderName = ParseXmlAttribute(*modelDefElement, "shader", "INVALID MODEL SHADER");

	m_shader = g_theRenderer->CreateShaderOrGetFromFile(shaderName.c_str());
	if (!m_shader)
	{
		ERROR_AND_DIE("Could not create shader");
	}

	XmlElement const* transformChildElement = modelDefElement->FirstChildElement("Transform");
	Vec3 matrixI = Vec3(0.f, 0.f, 0.f);
	Vec3 matrixJ = Vec3(0.f, 0.f, 0.f);
	Vec3 matrixK = Vec3(0.f, 0.f, 0.f);
	Vec3 matrixT = Vec3(0.f, 0.f, 0.f);
	float scale = 0.f;

	if (transformChildElement)
	{
		matrixI = ParseXmlAttribute(*transformChildElement, "x", Vec3(-1.f, -1.f, -1.f));
		matrixJ = ParseXmlAttribute(*transformChildElement, "y", Vec3(-1.f, -1.f, -1.f));
		matrixK = ParseXmlAttribute(*transformChildElement, "z", Vec3(-1.f, -1.f, -1.f));
		matrixT = ParseXmlAttribute(*transformChildElement, "t", Vec3(-1.f, -1.f, -1.f));
		scale = ParseXmlAttribute(*transformChildElement, "scale", -1.f);
	}

	Mat44 fixupTransform = Mat44(matrixI, matrixJ, matrixK, matrixT);
	fixupTransform.AppendScaleUniform3D(scale);

	m_cpuMeshh = new CPUMesh();
	m_gpuMesh = new GPUMesh();
	m_cpuMeshh->m_vertices.clear();
	m_cpuMeshh->m_indices.clear();
	OBJLoader::ImportFromOBJFile(modelPath, fixupTransform, m_cpuMeshh->m_vertices, m_cpuMeshh->m_indices );
	m_gpuMesh->SetVertexAndIndexBuffers(m_cpuMeshh);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Model::~Model()
{
	delete m_cpuMeshh;
	m_cpuMeshh = nullptr;

	delete m_gpuMesh;
	m_gpuMesh = nullptr;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Model::InitializeDefinitions(const char* filePath)
{
	(void)filePath;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Model::Render(const Mat44& transform, const Rgba8& color)
{
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->BindShader(m_shader);
	g_theRenderer->SetDepthModes(DepthMode::ENABLED);
	g_theRenderer->SetModelConstants(transform, color);
	g_theRenderer->SetLightConstants(Vec3(0.5f, 0.5f, -1.f), 0.9f, 0.1f, Vec3(0.f, 0.f, 0.f), 0, 0, 0, 0);
	m_gpuMesh->Render();
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindShader(nullptr);
	
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Model* Model::GetOrCreateModel(const std::string& modelName)
{
	std::string pathKey = modelName;

	auto iter = s_modelCache.find(pathKey);
	if (iter != s_modelCache.end())
	{
		return iter->second;
	}

	Model* newModel = new Model(modelName.c_str());
	s_modelCache[pathKey] = newModel;
	return newModel;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Model::ClearCache()
{
	for (auto& pair : s_modelCache)
	{
		Model* model = pair.second;
		delete model;
		model = nullptr;
	}
	s_modelCache.clear();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
