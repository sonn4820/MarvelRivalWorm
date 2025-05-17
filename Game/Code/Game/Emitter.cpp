#include "Emitter.hpp"
#include "Game/GameCommon.hpp"

Emitter::Emitter(ParticleSystemDFS2* sys, Vec2 position, float lifeTime)
	:m_system(sys), m_position(position), m_lifeTime(lifeTime)
{
	if (lifeTime == -1)
	{
		m_existForever = true;
	}

	m_system->AddEmitter(this);
}
void Emitter::Update(float deltaSeconds)
{
	if (m_lifeTime < 0.f) return;

	m_lifeTime -= deltaSeconds;

	m_triggerEmitTimer -= deltaSeconds;

	if (m_triggerEmitTimer < 0.f)
	{
		for (size_t i = 0; i < m_numParticleEachEmit; i++)
		{
			EmitParticle();
		}

		m_triggerEmitTimer = m_triggerTime;
	}
}
SquareParticle* Emitter::EmitParticle()
{
	if (m_particleDefIndexList.empty()) return nullptr;

	int randIndex = g_theRNG->RollRandomFromArray(m_particleDefIndexList);

	float orientation = 0;
	orientation += g_theRNG->RollRandomFloatInRange(m_randomOrientation);

	Vec2 dir = m_direction.GetRotatedDegrees(g_theRNG->RollRandomFloatInRange(m_randomOffsetAngle)) * m_speed;

	if (m_randomDirection)
	{
		float randomDirOrientation = g_theRNG->RollRandomFloatInRange(0.f, 359.f);
		dir = Vec2::MakeFromPolarDegrees(randomDirOrientation, m_speed);
	}

	Vec2 yDir = dir.GetNormalized();
	Vec2 xDir = yDir.GetRotatedMinus90Degrees();

	float x = g_theRNG->RollRandomFloatInRange(m_randomOffsetRangeX);
	float y = g_theRNG->RollRandomFloatInRange(m_randomOffsetRangeY);
	Vec2 pos = m_position + xDir * x + yDir * y;

	SquareParticle* newP = new SquareParticle(SquareParticleDefinition::GetByIndex(randIndex), m_system, pos, dir, orientation, m_particleLifeTime);

	return newP;
}

void Emitter::SetEmitDirection(Vec2 direction)
{
	m_randomDirection = false;
	m_direction = direction;
}

void Emitter::SetEmitDirectionRandom()
{
	m_randomDirection = true;
	m_direction = Vec2::ZERO;
}

void Emitter::SetEmitSpeed(float speed)
{
	m_speed = speed;
}

void Emitter::SetTimeBetweenEachEmit(float time)
{
	m_triggerTime = time;
	m_triggerEmitTimer = 0.f;
}

void Emitter::SetNumParticleEachEmit(int num)
{
	m_numParticleEachEmit = num;
}

void Emitter::SetParticleLifeTime(float time)
{
	m_particleLifeTime = time;
}

void Emitter::SetParticleLifeRandomInRange(FloatRange range)
{
	SetParticleLifeTime(g_theRNG->RollRandomFloatInRange(range));
}

void Emitter::SetParticleRandomOffsetDirAngle(FloatRange range)
{
	m_randomOffsetAngle = range;
}

void Emitter::SetParticleRandomOffsetPosition(FloatRange rangeX, FloatRange rangeY)
{
	m_randomOffsetRangeX = rangeX;
	m_randomOffsetRangeY = rangeY;
}

void Emitter::SetParticleRandomOrientation(FloatRange range)
{
	m_randomOrientation = range;
}

void Emitter::SetParticleDefs(std::vector<int> const& indexLists)
{
	m_particleDefIndexList.assign(indexLists.begin(), indexLists.end());
}
