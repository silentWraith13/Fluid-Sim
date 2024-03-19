#include "Game/Prop.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/App.hpp"
#include "Game/Game.hpp"
#include "Engine/Math/AABB3.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
Prop::Prop(Game* owner)
	:Entity(owner)
	
{
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
Prop::~Prop()
{
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Prop::Update(float deltaSeconds)
{
	m_position += (m_velocity * deltaSeconds);

	m_orientation.m_yawDegrees += m_angularVelocity.m_yawDegrees * deltaSeconds;
	m_orientation.m_pitchDegrees += m_angularVelocity.m_pitchDegrees * deltaSeconds;
	m_orientation.m_rollDegrees += m_angularVelocity.m_rollDegrees * deltaSeconds;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Prop::Render() 
{	
	g_theRenderer->SetModelConstants(GetModelMatrix(), m_color);
	g_theRenderer->BindTexture(m_texture);
    g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
	g_theRenderer->DrawVertexArray((int)m_vertexes.size(), m_vertexes.data());
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Prop::CreateQuadTwo()
{
	float maxX1 = 0.5f;
	float maxY1 = 0.5f;
	float maxZ1 = 0.5f;
	float minX1 = -0.5f;
	float minY1 = -0.5f;
	float minZ1 = -0.5f;

	 // +x
	AddVertsForQuad3D(m_vertexes, Vec3(maxX1, minY1, maxZ1), Vec3(maxX1, minY1, minZ1), Vec3(maxX1, maxY1, minZ1), Vec3(maxX1, maxY1, maxZ1), Rgba8(255, 0, 0), AABB2(Vec2(0.f, 0.f), Vec2(1.f, 1.f)));
	// -x
	AddVertsForQuad3D(m_vertexes, Vec3(minX1, minY1, minZ1), Vec3(minX1, minY1, maxZ1), Vec3(minX1, maxY1, maxZ1), Vec3(minX1, maxY1, minZ1), Rgba8(0, 255, 255), AABB2(Vec2(0.f, 0.f), Vec2(1.f, 1.f)));
	// +y
	AddVertsForQuad3D(m_vertexes, Vec3(maxX1, maxY1, minZ1), Vec3(minX1, maxY1, minZ1), Vec3(minX1, maxY1, maxZ1), Vec3(maxX1, maxY1, maxZ1), Rgba8(0, 255, 0), AABB2(Vec2(0.f, 0.f), Vec2(1.f, 1.f)));
	// -y
	AddVertsForQuad3D(m_vertexes, Vec3(minX1, minY1, minZ1), Vec3(maxX1, minY1, minZ1), Vec3(maxX1, minY1, maxZ1), Vec3(minX1, minY1, maxZ1), Rgba8(255, 0, 255), AABB2(Vec2(0.f, 0.f), Vec2(1.f, 1.f)));
	// +z
	AddVertsForQuad3D(m_vertexes, Vec3(minX1, minY1, maxZ1), Vec3(maxX1, minY1, maxZ1), Vec3(maxX1, maxY1, maxZ1), Vec3(minX1, maxY1, maxZ1), Rgba8(0, 0, 255), AABB2(Vec2(0.f, 0.f), Vec2(1.f, 1.f)));
	// -z
	AddVertsForQuad3D(m_vertexes, Vec3(maxX1, minY1, minZ1), Vec3(minX1, minY1, minZ1), Vec3(minX1, maxY1, minZ1), Vec3(maxX1, maxY1, minZ1), Rgba8(255, 255, 0), AABB2(Vec2(0.f, 0.f), Vec2(1.f, 1.f)));

	m_position = Vec3(-2.0f, -2.f, 0.f);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Prop::CreateSphere3D()
{
	m_angularVelocity.m_yawDegrees = 45.0f;
	m_position = Vec3(10.f, -5.0f, 1.f);
	
	// Create vertices for sphere
	AddVertsForSphere3D(m_vertexes, Vec3(0.f, 0.f, 0.f) , 1.f, Rgba8(255,255,255), AABB2(Vec2(0.f, 0.f),Vec2(1.f, 1.f) ), 8);
	m_texture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/TestUV.png");
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Prop::CreateGrid()
{
	// Add vertices for x-axis lines
	for (int x = -50; x <= 50; x++)
	{
		float thickness = 0.03f;
		Rgba8 color = Rgba8(255, 255, 255, 255);
		
		if (x == 0)
		{
			//continue;
			color = Rgba8(0, 255, 0, 255);
			thickness = 0.05f;
		}
		
		if (x > 0 || x < 0)
		{
			if (x % 5 == 0)
			{
				color = Rgba8(255, 0, 0, 255);
			}
			else
			{
				color = Rgba8(128, 128, 128, 255);
			}
		}
		
		Vec3 start = Vec3((float)x, -50.f, 0.f);
		Vec3 end = Vec3((float)x, 50.f, 0.f);

		AABB3 bounds = AABB3(start - Vec3(thickness, thickness, thickness), end + Vec3(thickness, thickness, thickness));
		AddVertsForAABB3D(m_vertexes, bounds, color);
	}

	// Add vertices for y-axis lines
	for (int y = -50; y <= 50; y++)
	{
		Rgba8 color = Rgba8(255, 255, 255, 255);
		float thickness = 0.03f;
		if (y == 0.f)
		{
			//continue;
			thickness = 0.05f;
			color = Rgba8(255, 0, 0, 255);
		}
		if (y > 0 || y < 0)
		{
			if (y % 5 == 0)
			{
				color = Rgba8(0, 255, 0, 255);
			}
			else
			{
				color = Rgba8(128, 128, 128, 255);
			}
		}

		Vec3 start = Vec3(-50.f, (float)y, 0.f);
		Vec3 end = Vec3(50.f, (float)y, 0.f);

		AABB3 bounds = AABB3(start - Vec3(thickness, thickness, thickness), end + Vec3(thickness, thickness, thickness));
		AddVertsForAABB3D(m_vertexes, bounds, color);
	}
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------------------------------------------------------------
void Prop::CreateQuadOne()
 {
	 float maxX = 0.5f;
	 float maxY = 0.5f;
	 float maxZ = 0.5f;
	 float minX = -0.5f;
	 float minY = -0.5f;
	 float minZ = -0.5f;
	 
	 //Cube centered at (2, 2, 0)
	 // +x
	 AddVertsForQuad3D(m_vertexes, Vec3(maxX, minY, maxZ), Vec3(maxX, minY, minZ), Vec3(maxX, maxY, minZ), Vec3(maxX, maxY, maxZ), Rgba8(255, 0, 0), AABB2(Vec2(0.f, 0.f),Vec2(1.f, 1.f)));
	 // -x
	 AddVertsForQuad3D(m_vertexes, Vec3(minX, minY, minZ), Vec3(minX, minY, maxZ), Vec3(minX, maxY, maxZ), Vec3(minX, maxY, minZ), Rgba8(0, 255, 255), AABB2(Vec2(0.f, 0.f), Vec2(1.f, 1.f)));
	 // +y
	 AddVertsForQuad3D(m_vertexes, Vec3(maxX, maxY, minZ), Vec3(minX, maxY, minZ), Vec3(minX, maxY, maxZ), Vec3(maxX, maxY, maxZ), Rgba8(0, 255, 0),AABB2(Vec2(0.f, 0.f), Vec2(1.f, 1.f)));
	 // -y
	 AddVertsForQuad3D(m_vertexes, Vec3(minX, minY, minZ), Vec3(maxX, minY, minZ), Vec3(maxX, minY, maxZ), Vec3(minX, minY, maxZ), Rgba8(255, 0, 255),AABB2(Vec2(0.f, 0.f), Vec2(1.f, 1.f)));
	 // +z
	 AddVertsForQuad3D(m_vertexes, Vec3(minX, minY, maxZ), Vec3(maxX, minY, maxZ), Vec3(maxX, maxY, maxZ), Vec3(minX, maxY, maxZ), Rgba8(0, 0, 255),AABB2(Vec2(0.f, 0.f), Vec2(1.f, 1.f)));
	 // -z
	 AddVertsForQuad3D(m_vertexes, Vec3(maxX, minY, minZ), Vec3(minX, minY, minZ), Vec3(minX, maxY, minZ), Vec3(maxX, maxY, minZ), Rgba8(255, 255, 0),AABB2(Vec2(0.f, 0.f), Vec2(1.f, 1.f)));

	 //Rotate cube around x and y 30 each
	 m_angularVelocity.m_pitchDegrees = 30.0f;
	 m_angularVelocity.m_yawDegrees = 30.0f;
	 m_position = Vec3(2.0f, 2.f, 0.f);
 }

//--------------------------------------------------------------------------------------------------------------------------------------------------------
