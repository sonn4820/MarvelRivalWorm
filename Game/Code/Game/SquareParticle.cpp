#include "SquareParticle.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Game/Game.hpp"

std::vector<SquareParticleDefinition*> SquareParticleDefinition::s_particleDefList;

void SquareParticleDefinition::InitializeBlockDefs()
{
	CreateNewBlockDef("Hela_P1", 0, IntVec2(0, 0), false);
	CreateNewBlockDef("Hela_P2", 1, IntVec2(0, 1), false);
	CreateNewBlockDef("Hela_P3", 2, IntVec2(0, 2), false);
	CreateNewBlockDef("Hela_P4", 3, IntVec2(0, 3), false);

	CreateNewBlockDef("Adam_P1", 4, IntVec2(1, 0), false);
	CreateNewBlockDef("Adam_P2", 5, IntVec2(1, 1), false);
	CreateNewBlockDef("Adam_P3", 6, IntVec2(1, 2), false);
	CreateNewBlockDef("Adam_P4", 7, IntVec2(1, 3), false);

	CreateNewBlockDef("IW_P1", 8, IntVec2(2, 0), false);
	CreateNewBlockDef("IW_P2", 9, IntVec2(2, 1), false);
	CreateNewBlockDef("IW_P3", 10, IntVec2(2, 2), false);
	CreateNewBlockDef("IW_P4", 11, IntVec2(2, 3), false);

	CreateNewBlockDef("Iron_P1", 12, IntVec2(3, 0), false);
	CreateNewBlockDef("Iron_P2", 13, IntVec2(3, 1), false);
	CreateNewBlockDef("Iron_P3", 14, IntVec2(3, 2), false);
	CreateNewBlockDef("Iron_P4", 15, IntVec2(3, 3), false);

	CreateNewBlockDef("Thor_P1", 16, IntVec2(4, 0), false);
	CreateNewBlockDef("Thor_P2", 17, IntVec2(4, 1), false);
	CreateNewBlockDef("Thor_P3", 18, IntVec2(4, 2), false);
	CreateNewBlockDef("Thor_P4", 19, IntVec2(4, 3), false);

	CreateNewBlockDef("Hulk_P1", 20, IntVec2(5, 0), false);
	CreateNewBlockDef("Hulk_P2", 21, IntVec2(5, 1), false);
	CreateNewBlockDef("Hulk_P3", 22, IntVec2(5, 2), false);
	CreateNewBlockDef("Hulk_P4", 23, IntVec2(5, 3), false);

	CreateNewBlockDef("Blood_P1", 24, IntVec2(5, 0), true);
	CreateNewBlockDef("Blood_P2", 25, IntVec2(5, 1), true);
	CreateNewBlockDef("Blood_P3", 26, IntVec2(5, 2), true);
	CreateNewBlockDef("Heal", 27, IntVec2(5, 4), false);
}

void SquareParticleDefinition::ClearDefinition()
{
	for (auto& i : s_particleDefList)
	{
		if (i != nullptr)
		{
			delete i;
			i = nullptr;
		}
	}
}

void SquareParticleDefinition::CreateNewBlockDef(std::string name, unsigned char index, IntVec2 spriteCoord, bool gravity)
{
	SquareParticleDefinition* particleDef = new SquareParticleDefinition();
	particleDef->m_name = name;
	particleDef->m_index = index;
	particleDef->m_spriteCoord = spriteCoord;
	particleDef->m_hasGravity = gravity;
	s_particleDefList.push_back(particleDef);
}

SquareParticleDefinition* SquareParticleDefinition::GetByName(std::string name)
{
	for (auto& i : s_particleDefList)
	{
		if (i->m_name == name)
		{
			return i;
		}
	}
	return nullptr;
}

SquareParticleDefinition* SquareParticleDefinition::GetByIndex(unsigned int index)
{
	for (auto& i : s_particleDefList)
	{
		if (i->m_index == index)
		{
			return i;
		}
	}
	return nullptr;
}

SquareParticle::SquareParticle(SquareParticleDefinition* def, ParticleSystemDFS2* system, Vec2 position, Vec2 velocity, float orientation /*= 0.f*/, float lifeTime /*= 1.f*/)
	:m_particleDef(def), m_system(system), m_position(position), m_velocity(velocity), m_orientation(orientation), m_lifeTime(lifeTime)
{
	AABB2 uv = system->m_vfxSpriteSheet->GetSpriteUVs(def->m_index);
	AddVertsForAABB2D(m_verts, AABB2(-0.5f, -0.5f, 0.5f, 0.5f), m_tint, uv.m_mins, uv.m_maxs);

	m_vbuffer = g_theRenderer->CreateVertexBuffer(sizeof(Vertex_PCU) * (int)m_verts.size());
	g_theRenderer->CopyCPUToGPU(m_verts.data(), (int)(m_verts.size() * sizeof(Vertex_PCU)), m_vbuffer);

	m_system->AddParticle(this);
}

SquareParticle::~SquareParticle()
{
	delete m_vbuffer;
	m_vbuffer = nullptr;
}

void SquareParticle::Update(float deltaSeconds)
{
	if (m_lifeTime <= 0.f) return;

	m_lifeTime -= deltaSeconds;

	if (m_particleDef->m_hasGravity)
	{
		m_velocity += Vec2(0, -31.8f) * deltaSeconds;
	}

	m_position += m_velocity * deltaSeconds;

	//m_tint.a = (unsigned char)RangeMapClamped(m_tint.a, m_lifeTime, 0.f, 255.f, 0.f);
}

void SquareParticle::Render() const
{
	if (m_lifeTime <= 0.f) return;

	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(&m_system->m_vfxSpriteSheet->GetTexture());
	g_theRenderer->SetModelConstants(GetModelMatrix(), m_tint);
	g_theRenderer->DrawVertexBuffer(m_vbuffer, m_verts.size());
}

Mat44 SquareParticle::GetModelMatrix() const
{
	Mat44 result = Mat44::CreateTranslation2D(m_position);
	result.AppendZRotation(m_orientation);
	return	result;
}
