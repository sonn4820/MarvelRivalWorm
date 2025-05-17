#pragma once
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Math/MathUtils.hpp"

class Emitter;
class SquareParticle;

class ParticleSystemDFS2
{
public:

	ParticleSystemDFS2();
	~ParticleSystemDFS2();

	void Update(float deltaSeconds);
	void Render() const;

	void AddEmitter(Emitter* emitter);
	void AddParticle(SquareParticle* particle);


	std::vector<Emitter*> m_emitters;
	std::vector<SquareParticle*> m_particles;

	SpriteSheet* m_vfxSpriteSheet = nullptr;
};

