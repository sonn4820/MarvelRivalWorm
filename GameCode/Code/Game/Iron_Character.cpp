#include "Game/Iron_Character.hpp"

Iron_Character::Iron_Character(Map* map)
	:Character(map)
{
	m_characterDef = CharacterDefinition::GetByName("Iron");
	Initialize();

	m_vfxIndex = { 12, 13, 14};

	m_affectedTileSet = AffectedTilesSet::EXPLOSION;

	m_flyingEmitter = new Emitter(m_map->m_game->m_vfxSystem, m_position, -1.f);
	m_flyingEmitter->SetEmitDirection(Vec2(0, -1));
	m_flyingEmitter->SetParticleRandomOffsetDirAngle(FloatRange(-50, 50));
	m_flyingEmitter->SetParticleRandomOrientation(FloatRange(0.f, 359.f));
	m_flyingEmitter->SetNumParticleEachEmit(3);
	m_flyingEmitter->SetParticleLifeRandomInRange(FloatRange(1.0f, 1.6f));
	m_flyingEmitter->SetTimeBetweenEachEmit(0.12f);
	m_flyingEmitter->SetEmitSpeed(g_theRNG->RollRandomFloatInRange(15.f, 22.f));
	m_flyingEmitter->SetParticleDefs({ 12, 13, 14 });

	m_skillEmitter = new Emitter(m_map->m_game->m_vfxSystem, m_position, -1.f);
	m_skillEmitter->SetEmitDirection(Vec2(0, 1));
	m_skillEmitter->SetParticleRandomOffsetPosition(FloatRange(-4, 4), FloatRange(-3, 1));
	m_skillEmitter->SetNumParticleEachEmit(2);
	m_skillEmitter->SetParticleLifeRandomInRange(FloatRange(0.4f, 0.7f));
	m_skillEmitter->SetTimeBetweenEachEmit(0.07f);
	m_skillEmitter->SetEmitSpeed(g_theRNG->RollRandomFloatInRange(12.f, 17.f));
	m_skillEmitter->SetParticleDefs({ 15 });
}

Iron_Character::~Iron_Character()
{

}

void Iron_Character::BeginTurn()
{
	Character::BeginTurn();
	if (m_skill1CDCounter > 0)
	{
		m_stamina = m_characterDef->m_initialStamina / 2;
	}
}

void Iron_Character::EndTurn()
{
	Character::EndTurn();

	m_stamina = m_characterDef->m_initialStamina;
	m_stamina += m_bonusStamina;
	m_bonusStamina = 0;
	m_hasUsedSkill1 = false;
}


void Iron_Character::Passive()
{
	m_isFlying = true;
}

void Iron_Character::Skill_1()
{
	if (!ConditionsForSkill_1()) return;

	Character::Skill_1();

	m_hasUsedSkill1 = true;
	m_canUseSKill1 = false;
	m_attack += 100;
	m_skillEmitter->m_lifeTime = 0.6f;
	m_map->m_game->m_audioManager->PlaySFXSound("Iron_S1_U", 15);
}

void Iron_Character::Skill_2()
{
	if (!ConditionsForSkill_2()) return;

	Character::Skill_2();

	m_currentProjectile = ProjectileDefinition::GetByName("Iron_S2");

	for (size_t i = 0; i < IRON_S2_COUNT; i++)
	{
		Projectile* proj = ShootCurrentProjectile(m_currentAngle, m_currentForce);
		proj->m_velocity = Vec2(g_theRNG->RollRandomFloatMinusOneToOne(), g_theRNG->RollRandomFloatMinusOneToOne()).GetNormalized() * 20.f;
		proj->m_orientationDegrees = proj->m_velocity.GetOrientationDegrees();
	}
}

void Iron_Character::Ultimate()
{
	if (!ConditionsForUltimate()) return;

	Character::Ultimate();

	m_hasSpawnedUltimate = false;
}

void Iron_Character::NormalAttack()
{
	if (m_hasUsedSkill1)
	{
		m_currentProjectile = ProjectileDefinition::GetByName("Iron_S1");
	}
	else
	{
		m_currentProjectile = m_characterDef->m_projectiles[0];
	}

	ShootCurrentProjectile(m_currentAngle, m_currentForce);

	m_currentForce = m_characterDef->m_forceLimit.m_min;

	m_fury += FURY_PER_ATTACK;

	m_map->m_game->PendingEndTurn();
}

void Iron_Character::UltimateUpdate(float deltaSeconds)
{
	UNUSED(deltaSeconds);

	if (m_hasSpawnedUltimate) return;
	if (g_theAudio->IsPlaying(m_map->m_game->m_audioManager->m_latestSound)) return;

	m_map->m_game->m_audioManager->PlaySFXSound("Iron_Ult_C");

	m_hasSpawnedUltimate = true;

	ProjectileDefinition* ultDef = ProjectileDefinition::GetByName("Iron_Ult");
	float capsuleLength = ultDef->m_visualHalfLength.x * 2.f - ultDef->m_visualHalfLength.y * 2.f;
	float radius = ultDef->m_visualHalfLength.y;
	Vec2 dir = Vec2::MakeFromPolarDegrees(m_currentAngle);
	Capsule2 laser = Capsule2(m_position + dir * radius, m_position + dir * (radius + capsuleLength), radius);

	Projectile* Ultimate_projectile = new Projectile(m_map, this, ultDef);
	Ultimate_projectile->m_position = laser.GetCenter();
	Ultimate_projectile->m_isStatic = true;
	Ultimate_projectile->m_deadTimer = new Timer(3.f, m_map->m_game->m_gameClock);
	Ultimate_projectile->m_deadTimer->Start();
	if (m_isFacingLeft)
	{
		Ultimate_projectile->m_orientationDegrees += (90 - (180 - m_currentAngle));
	}
	else
	{
		Ultimate_projectile->m_orientationDegrees -= (90 - m_currentAngle);
	}
	m_map->m_flyingProjectiles.push_back(Ultimate_projectile);


	for (auto chunk : m_map->m_activeChunks)
	{
		for (int i = 0; i < TOTAL_BLOCKS_SIZE; i++)
		{
			BlockIterator iter = BlockIterator(chunk.second, i);
			if (IsPointInsideCapsule2D(iter.GetWorldCenter(), laser))
			{
				chunk.second->SetBlockType(i, 0);
			}
		}

		chunk.second->SetMeshDirty();
	}

	for (auto& character : m_map->m_characters)
	{
		if (DoDiscOverlapCapsule2D(character->m_position, character->m_currentHitboxRadius, laser))
		{
			if (character == this) continue;
			character->TakeDamage(IRON_ULTIMATE_DAMAGE, this, true);
		}
	}
}

void Iron_Character::PlayMovingSound()
{
	if (m_stamina <= 0) return;

	if (m_controller->m_isMoving)
	{
		if (m_isFirstMove)
		{
			m_isFirstMove = false;
			if (g_theAudio->IsPlaying(m_firstMoveSound)) return;
			m_firstMoveSound = m_map->m_game->m_audioManager->PlayAndGetSFXSound("Iron_FirstMove", 2.f);
		}
		
		if (g_theAudio->IsPlaying(m_flyingSound)) return;
		if (g_theAudio->IsPlaying(m_firstMoveSound)) return;
		m_flyingSound = m_map->m_game->m_audioManager->PlayAndGetSFXSound("Iron_Flying", 0.5f, 1.f, 0.f, true);
	}
}

void Iron_Character::Update(float deltaSeconds)
{
	Character::Update(deltaSeconds);
	UltimateUpdate(deltaSeconds);
	m_flyingEmitter->m_position = m_position - Vec2(0, m_currentVisualHalfLength.y + 1);
	m_skillEmitter->m_position = m_position - Vec2(0, m_currentVisualHalfLength.y);

	if (!m_controller->m_isMoving)
	{
		m_isFirstMove = true;
		if (m_flyingSound != MISSING_SOUND_ID) g_theAudio->StopSound(m_flyingSound);
		if (m_firstMoveSound != MISSING_SOUND_ID) g_theAudio->StopSound(m_firstMoveSound);
	}
}

void Iron_Character::MoveUp(float deltaSeconds)
{
	Character::MoveUp(deltaSeconds);
	if (!m_flyingEmitter)
	{
		m_flyingEmitter = new Emitter(m_map->m_game->m_vfxSystem, m_position, 10.0f);
	}

	if (m_stamina > 0)
	{
		m_flyingEmitter->m_lifeTime = 0.4f;
	}
	PlayMovingSound();

}

void Iron_Character::MoveLeft(float deltaSeconds)
{
	Character::MoveLeft(deltaSeconds);
	if (!m_flyingEmitter)
	{
		m_flyingEmitter = new Emitter(m_map->m_game->m_vfxSystem, m_position, 10.0f);
	}

	if (m_stamina > 0)
	{
		m_flyingEmitter->m_lifeTime = 0.4f;
	}
	PlayMovingSound();
}

void Iron_Character::MoveRight(float deltaSeconds)
{
	Character::MoveRight(deltaSeconds);
	if (!m_flyingEmitter)
	{
		m_flyingEmitter = new Emitter(m_map->m_game->m_vfxSystem, m_position, 10.0f);
	}

	if (m_stamina > 0)
	{
		m_flyingEmitter->m_lifeTime = 0.4f;
	}
	PlayMovingSound();
}

void Iron_Character::MoveDown(float deltaSeconds)
{
	Character::MoveDown(deltaSeconds);
	if (!m_flyingEmitter)
	{
		m_flyingEmitter = new Emitter(m_map->m_game->m_vfxSystem, m_position, 10.0f);
	}

	if (m_stamina > 0)
	{
		m_flyingEmitter->m_lifeTime = 0.4f;
	}
	PlayMovingSound();
}

Iron_AI_Controller::Iron_AI_Controller(Game* owner, Character* character, char ID, char teamID)
	:AIController(owner, character, ID, teamID)
{
	m_targetYPos = g_theRNG->RollRandomFloatInRange(80.f, 100.f);
}

void Iron_AI_Controller::MovingLogic(float deltaSeconds)
{
	if (!IsInCurrentTargetY())
	{
		if (m_character->m_fury == 100)
		{
			m_targetYPos = m_character->GetNearestEnemy()->m_position.y + 20.f;
		}
		if (m_character->m_position.y > m_targetYPos)
		{
			m_character->MoveDown(deltaSeconds);
		}
		else
		{
 			m_character->MoveUp(deltaSeconds);
		}
		if (m_character->m_stamina <= 0.f)
		{
			m_doneMoving = true;
		}
	}
	else
	{
		if (m_character->m_gotHit)
		{
			if (m_character->m_stamina > m_staminaMovingLimit)
			{
				m_character->MoveOnCurrentFacing(deltaSeconds);
			}
			else
			{
				m_doneMoving = true;
			}
		}
		else
		{
			if (!IsInCurrentTargetX())
			{
				m_character->FaceToTarget(m_currentFocusTarget->m_position);
				m_character->MoveOnCurrentFacing(deltaSeconds);
				if (m_character->m_stamina <= 0.f)
				{
					m_doneMoving = true;
				}
			}
			else
			{
				m_isGuessing = false;
				m_guessCount = 99;
				m_hasReachedTarget = true;
				m_doneMoving = true;
				m_thinkingSkillTimer = 1.f;
			}
		}
	}

	if (m_doneMoving)
	{
		m_thinkingTimer = m_forceAndAngleThinkingTime + 0.5f;
	}
}

void Iron_AI_Controller::UsingSkillLogic(float deltaSeconds)
{
	m_thinkingSkillTimer -= deltaSeconds;

	if (m_character->m_fury == 100)
	{
		m_ultThinkingTimer -= deltaSeconds;

		Vec2 toNearestTarget = m_character->GetNearestEnemy()->m_position - m_character->m_position;
		m_currentAngle = toNearestTarget.GetOrientationDegrees();

		if (m_ultThinkingTimer < 0.f)
		{
			m_hasUsedUltimate = true;
			m_character->Ultimate();
			m_thinkingSkillTimer = 3.f;
		}
	}

	if (m_character->m_fury == 100)
	{
		return;
	}

	if (m_hasReachedTarget)
	{
		if (m_character->ConditionsForSkill_1())
		{
			m_character->Skill_1();
			m_thinkingSkillTimer = 1.f;
		}

		if (m_thinkingSkillTimer > 0.f) return;

		if (m_character->ConditionsForSkill_2())
		{
			m_character->Skill_2();
			m_hasDoneUsingSkills = false;
			m_thinkingSkillTimer = 0.2f;
			m_thinkingTimer = 3.f;
		}
		else
		{
			m_hasDoneUsingSkills = true;
		}
	}
	else
	{
		m_hasDoneUsingSkills = true;
	}
}

void Iron_AI_Controller::BeginTurn()
{
	m_hasReachedTarget = false;
	m_hasDoneUsingSkills = false;
	m_targetYPos = g_theRNG->RollRandomFloatInRange(100.f, 120.f);
	AIController::BeginTurn();
}

void Iron_AI_Controller::NormalAttackLogic()
{
	if (m_hasDoneUsingSkills)
	{
		AIController::NormalAttackLogic();
	}
}

bool Iron_AI_Controller::IsInCurrentTargetY() const
{
	return m_character->m_position.y > m_targetYPos - 1.f && m_character->m_position.y < m_targetYPos + 1.f;
}

bool Iron_AI_Controller::IsInCurrentTargetX() const
{
	return m_character->m_position.x > m_currentFocusTarget->m_position.x - 10.f && m_character->m_position.x < m_currentFocusTarget->m_position.x + 10.f;
}
