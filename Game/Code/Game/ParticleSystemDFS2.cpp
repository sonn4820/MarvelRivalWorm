#include "ParticleSystemDFS2.hpp"
#include "Emitter.hpp"
#include "SquareParticle.hpp"
#include "Game/Game.hpp"

ParticleSystemDFS2::ParticleSystemDFS2()
{
	Texture* texture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/VFX.png");
	m_vfxSpriteSheet = new SpriteSheet(*texture, IntVec2(4, 8));

	m_emitters.reserve(40);
	m_particles.reserve(150);
}

ParticleSystemDFS2::~ParticleSystemDFS2()
{
	for (auto& particle : m_particles)
	{
		delete particle;
	}
}

void ParticleSystemDFS2::Update(float deltaSeconds)
{
	for (auto& emitter : m_emitters)
	{
		if (emitter) emitter->Update(deltaSeconds);

	}
	for (auto& particle : m_particles)
	{
		if (particle) particle->Update(deltaSeconds);
	}

	for (auto& emitter : m_emitters)
	{
		if (emitter && !emitter->m_existForever && emitter->m_lifeTime <= 0.f)
		{
			delete emitter;
			emitter = nullptr;
		}
	}
	for (auto& particle : m_particles)
	{
		if (particle && particle->m_lifeTime <= 0.f)
		{
			delete particle;
			particle = nullptr;
		}
	}
}

void ParticleSystemDFS2::Render() const
{
	for (auto particle : m_particles)
	{
		if (particle) particle->Render();
	}
}

void ParticleSystemDFS2::AddEmitter(Emitter* emitter)
{
	for (auto& i : m_emitters)
	{
		if (i == nullptr)
		{
			i = emitter;
			return;
		}
	}

	m_emitters.push_back(emitter);
}

void ParticleSystemDFS2::AddParticle(SquareParticle* particle)
{
	for (auto& i : m_particles)
	{
		if (i == nullptr)
		{
			i = particle;
			return;
		}
	}

	m_particles.push_back(particle);
}
