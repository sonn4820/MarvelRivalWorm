#pragma once
#include "Game/SquareParticle.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "ParticleSystemDFS2.hpp"
class Emitter
{
public:
	Emitter(ParticleSystemDFS2* sys, Vec2 position, float lifeTime);

	void Update(float deltaSeconds);
	SquareParticle* EmitParticle();

	void SetEmitDirection(Vec2 direction);
	void SetEmitDirectionRandom();
	void SetEmitSpeed(float speed);
	void SetTimeBetweenEachEmit(float time);
	void SetNumParticleEachEmit(int num);
	void SetParticleLifeTime(float time);
	void SetParticleLifeRandomInRange(FloatRange range);
	void SetParticleRandomOffsetDirAngle(FloatRange range);
	void SetParticleRandomOffsetPosition(FloatRange rangeX, FloatRange rangeY);
	void SetParticleRandomOrientation(FloatRange range);
	void SetParticleDefs(std::vector<int> const& indexLists);

	ParticleSystemDFS2* m_system;
	std::vector<int> m_particleDefIndexList;

	bool m_existForever = false;
	Vec2 m_position;
	float m_lifeTime = 0.f;
	Vec2 m_direction;
	bool m_randomDirection = false;
	float m_triggerTime = 1.f;
	float m_triggerEmitTimer = 0.f;
	int m_numParticleEachEmit = 1;
	float m_particleLifeTime = 0.f;
	float m_speed = 1.f;
	FloatRange m_randomOffsetAngle;
	FloatRange m_randomOffsetRangeX;
	FloatRange m_randomOffsetRangeY;
	FloatRange m_randomOrientation;
};

