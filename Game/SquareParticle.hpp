#pragma once
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "ParticleSystemDFS2.hpp"

struct SquareParticleDefinition
{
	std::string m_name = "";
	unsigned char m_index = 0;
	IntVec2 m_spriteCoord;

	bool m_hasGravity = true;

	SquareParticleDefinition() = default;
	static void InitializeBlockDefs();
	static void ClearDefinition();
	static void CreateNewBlockDef(std::string name, unsigned char index, IntVec2 spriteCoord, bool gravity);
	static SquareParticleDefinition* GetByName(std::string name);
	static SquareParticleDefinition* GetByIndex(unsigned int type);
	static std::vector<SquareParticleDefinition*> s_particleDefList;
};

class SquareParticle
{
public:
	SquareParticle(SquareParticleDefinition* def, ParticleSystemDFS2* system, Vec2 position, Vec2 velocity, float orientation = 0.f, float lifeTime = 1.f);
	~SquareParticle();

	void Update(float deltaSeconds);
	void Render() const;
	Mat44 GetModelMatrix() const;

	ParticleSystemDFS2* m_system = nullptr;

	Vec2 m_position;
	Vec2 m_velocity;
	float m_orientation = 0.f;
	float m_lifeTime = 0.2f;
	SquareParticleDefinition* m_particleDef = nullptr;
	Rgba8 m_tint;

	std::vector<Vertex_PCU> m_verts;
	VertexBuffer* m_vbuffer = nullptr;
};

