#include "Game/Hela_Character.hpp"

Hela_Character::Hela_Character(Map* map)
	:Character(map)
{
	m_characterDef = CharacterDefinition::GetByName("Hela");
	Initialize();

	m_ultTimer = new Timer(1.2f, m_map->m_game->m_gameClock);

	m_vfxIndex = { 0, 1, 2, 3 };

	m_affectedTileSet = AffectedTilesSet::HELA;

	m_skillEmitter = new Emitter(m_map->m_game->m_vfxSystem, m_position, -1.f);
	m_skillEmitter->SetEmitDirection(Vec2(0, 1));
	m_skillEmitter->SetParticleRandomOffsetPosition(FloatRange(-4, 4), FloatRange(-3, 1));
	m_skillEmitter->SetParticleRandomOffsetDirAngle(FloatRange(-40, 40));
	m_skillEmitter->SetNumParticleEachEmit(2);
	m_skillEmitter->SetParticleLifeRandomInRange(FloatRange(0.7f, 1.2f));
	m_skillEmitter->SetTimeBetweenEachEmit(0.07f);
	m_skillEmitter->SetEmitSpeed(g_theRNG->RollRandomFloatInRange(2.f, 5.f));
	m_skillEmitter->SetParticleDefs({ 0, 1, 2, 3 });
}

Hela_Character::~Hela_Character()
{

}

void Hela_Character::Update(float deltaSeconds)
{
	Character::Update(deltaSeconds);

	if (!m_usingUltimate)
	{
		NA_Update(deltaSeconds);
	}
	else
	{
		Ult_Update();
	}

	m_skillEmitter->m_position = m_position;
}

void Hela_Character::EndTurn()
{
	m_projectileStack = 0;

	m_performedNA = false;
	m_spawnTimer = 0.f;

	if (m_usingUltimate)
	{
		m_usingUltimate = false;
		m_stamina = 0;
		m_isFlying = false;
		m_ultTimer->Stop();
	}

	Character::EndTurn();
}

void Hela_Character::NormalAttack()
{
	m_critRateFromZeroToOne += HELA_CRIT_RATE_PASSIVE;
	m_attack += (m_damageStack * HELA_DAMAGE_PER_STACK);

	if (!m_usingUltimate)
	{
		m_performedNA = true;
		m_map->m_game->PendingEndTurn();
		m_fury += FURY_PER_ATTACK;
	}
	else
	{
		if (m_ultimateCount > 0)
		{
			ShootCurrentProjectile(m_currentAngle, 100);
			m_ultimateCount--;
			m_currentForce = m_characterDef->m_forceLimit.m_min;
			m_stamina += 200;

			if (m_ultimateCount == 0)
			{
				m_map->m_game->PendingEndTurn();
			}
		}
	}
}

void Hela_Character::Passive()
{
	if (m_map->m_game->m_playerIDTurn == GetID())
	{
		//DebugAddScreenText(Stringf("Damage Stack :%i", m_damageStack), Vec2(50, 750), 20.f);
		//DebugAddScreenText(Stringf("Projectile Stack :%i", m_projectileStack), Vec2(50, 720), 20.f);
	}
}

void Hela_Character::Skill_1()
{
	if (!ConditionsForSkill_1()) return;

	Character::Skill_1();

	m_canUseSKill2 = false;
	m_projectileStack++;

	m_skillEmitter->m_lifeTime = 0.3f;
}

void Hela_Character::Skill_2()
{
	if (!ConditionsForSkill_2()) return;

	Character::Skill_2();

	m_canUseSKill1 = false;
	m_currentProjectile = ProjectileDefinition::GetByName("Hela_S2");
}

void Hela_Character::Ultimate()
{
	if (!ConditionsForUltimate()) return;

	Character::Ultimate();

	m_attack = HELA_ULTIMATE_DAMAGE;
	m_ultimateCount = HELA_ULTIMATE_COUNT;
	m_isFlying = true;
	m_usingUltimate = true;
	m_canUseSKill1 = false;
	m_canUseSKill2 = false;
	m_currentProjectile = ProjectileDefinition::GetByName("Hela_Ult");

	GameRaycastResult2D checkUp = m_map->RaycastWorld(m_position, Vec2(0, 1), HELA_ULTIMATE_Y_POSITION_OFFSET);
	if (checkUp.m_didImpact)
	{
		m_ultimatePosition = m_position + Vec2(0, checkUp.m_impactDist - m_currentVisualHalfLength.y - 1);;
	}
	else
	{
		int yPos = HELA_ULTIMATE_Y_POSITION_OFFSET;
		if (m_position.y + HELA_ULTIMATE_Y_POSITION_OFFSET > 100) yPos = 100 - RoundDownToInt(m_position.y);
		m_ultimatePosition = m_position + Vec2(0, (float)yPos);
	}

	m_initialPositon = m_position;
	m_ultTimer->Start();

	m_currentAngleLimit = FloatRange(HELA_ULTIMATE_MIN_ANGLE, HELA_ULTIMATE_MAX_ANGLE);
	if (m_isFacingLeft)
	{
		m_currentAngleLimit = FloatRange(180.f - m_currentAngleLimit.m_max, 180.f - m_currentAngleLimit.m_min);
	}

	m_currentAngle = Clamp(m_currentAngle, m_currentAngleLimit.m_min, m_currentAngleLimit.m_max);

	m_currentAngle = Clamp(m_currentAngle, m_currentAngleLimit.m_min, m_currentAngleLimit.m_max);
	m_stamina = 1500;
}

void Hela_Character::NA_Update(float deltaSeconds)
{
	if (m_performedNA)
	{
		m_spawnTimer -= deltaSeconds;

		if (m_spawnTimer < 0.f)
		{
			m_spawnTimer = HELA_SPAWN_TIMER;

			if (m_projectileStack >= 0)
			{
				ShootCurrentProjectile(m_currentAngle, m_currentForce);

				m_projectileStack--;
			}
			else
			{
				m_performedNA = false;
			}
		}
	}
}

void Hela_Character::Ult_Update()
{
	if (!m_ultTimer->HasPeriodElapsed())
	{
		m_position = Interpolate(m_initialPositon, m_ultimatePosition, SmoothStop4(m_ultTimer->GetElapsedFraction()));
	}
}

Hela_AI_Controller::Hela_AI_Controller(Game* owner, Character* character, char ID, char teamID)
	:AIController(owner, character, ID, teamID)
{

}

void Hela_AI_Controller::UsingSkillLogic(float deltaSeconds)
{
	if (m_character->m_fury == 100)
	{
		m_ultThinkingTimer -= deltaSeconds;
		if (m_ultThinkingTimer < 0.f)
		{
			m_character->Ultimate();
			m_hasUsedUltimate = true;
		}
	}
	if (m_hasUsedUltimate)
	{
		m_ultThinkingTimer -= deltaSeconds;

		m_ultTargetCharacter = FindLowestInitialHPEnemy();

		if (m_ultTargetCharacter)
		{
			float distanceToTargetSquared = (m_character->m_position - m_ultTargetCharacter->m_position).GetLengthSquared();
			if (distanceToTargetSquared > HELA_ULTIMATE_MOVING_RANGE && m_character->m_stamina >= 10.f)
			{
				if (m_character->IsPositionBehind(m_ultTargetCharacter->m_position))
				{
					m_character->FaceTheOpposite();
				}

				m_character->MoveOnCurrentFacing(deltaSeconds);

				m_isMovingInUlt = true;
			}
			else
			{
				m_isMovingInUlt = false;
			}

			m_currentAngle = (m_ultTargetCharacter->m_position - m_character->m_position).GetOrientationDegrees();
		}
	}
	else
	{
		if (!m_hasUsedSkill2 && m_character->m_canUseSKill2 && m_character->m_gotHit && g_theRNG->RollRandomChance(0.05f))
		{
			m_character->m_gotHit = false;
			UseSkill2();
			return;
		}
		if (!m_hasUsedSkill2 && m_character->m_canUseSKill2 && m_character->IsCurrentHealthPercentageLowerThan(0.3f))
		{
			UseSkill2();
			return;
		}
		while (!m_hasUsedSkill2 && m_character->ConditionsForSkill_1())
		{
			m_character->Skill_1();
			m_hasUsedSkill1 = true;
		}
	}


}

void Hela_AI_Controller::NormalAttackLogic()
{
	if (m_hasUsedUltimate)
	{
		if (!m_isMovingInUlt)
		{
			Hela_Character* hela = dynamic_cast<Hela_Character*>(m_character);
			if (hela->m_ultimateCount > 0 && m_ultThinkingTimer <= 0.f)
			{
				AIController::NormalAttackLogic();
				m_hasPerfomedNA = false;
				m_thinkingTimer = m_forceAndAngleThinkingTime;
				m_ultThinkingTimer = m_thinkingTimer + 2.f;
				m_currentAngle += GetRandomValueBasedOnSkillLevel(-5.f, 5.f);
			}
		}
	}
	else
	{
		AIController::NormalAttackLogic();
	}

}

void Hela_AI_Controller::BeginTurn()
{
	m_hasUsedSkill2 = false;
	AIController::BeginTurn();
}

void Hela_AI_Controller::UseSkill2()
{
	RandomTurn();
	if (m_character->m_isFacingLeft && m_currentAngle < 90.f)
	{
		m_currentAngle = 180.f - m_currentAngle;
	}

	m_currentForce = g_theRNG->RollRandomFloatInRange(10.f, 30.f);
	m_character->m_currentForce = m_currentForce;
	m_hasUsedSkill2 = true;
	m_character->Skill_2();
}