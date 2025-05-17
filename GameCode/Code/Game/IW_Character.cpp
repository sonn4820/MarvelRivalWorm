#include "Game/IW_Character.hpp"

IW_Character::IW_Character(Map* map)
	:Character(map)
{
	m_characterDef = CharacterDefinition::GetByName("IW");
	Initialize();

	m_vfxIndex = { 8, 9, 10, 11 };
}

IW_Character::~IW_Character()
{

}

void IW_Character::NormalAttack()
{
	if (m_usingUltimate)
	{
		IW_ShootCurrentProjectile(m_currentAngle, m_currentForce);
		m_currentForce = m_characterDef->m_forceLimit.m_min;
		m_usingUltimate = false;
	}
	else if (m_usingSkill2)
	{
		IW_ShootCurrentProjectile(m_currentAngle, m_currentForce);
		m_currentForce = m_characterDef->m_forceLimit.m_min;
		m_usingSkill2 = false;
	}
	else
	{
		m_currentProjectile = m_characterDef->m_projectiles[0];
		IW_ShootCurrentProjectile(m_currentAngle, m_currentForce);
		m_currentForce = m_characterDef->m_forceLimit.m_min;
		m_fury += FURY_PER_ATTACK;
		m_map->m_game->PendingEndTurn();
	}
}

void IW_Character::BeginTurn()
{
	Character::BeginTurn();
	m_usingSkill2 = false;
	m_usingUltimate = false;
}

void IW_Character::Passive()
{
	// ON NORMAL ATTACK
}

void  IW_Character::Skill_1()
{
	if (!ConditionsForSkill_1()) return;

	Character::Skill_1();

	m_isInvisible = true;
	m_isPendingInvisible = true;
}

void IW_Character::Skill_2()
{
	if (!ConditionsForSkill_2()) return;

	Character::Skill_2();

	m_usingSkill2 = true;
	m_currentProjectile = ProjectileDefinition::GetByName("IW_S2");
}

void IW_Character::Ultimate()
{
	if (!ConditionsForUltimate()) return;
	if (m_usingSkill2) m_usingSkill2 = false;

	Character::Ultimate();

	m_usingUltimate = true;
	m_currentProjectile = ProjectileDefinition::GetByName("IW_Ult");
}

bool IW_Character::ConditionsForSkill_1()
{
	return Character::ConditionsForSkill_1() && !m_isInvisible;
}

Projectile* IW_Character::IW_ShootCurrentProjectile(float angle, float force)
{
	m_lastForce = force;

	Projectile* proj = nullptr;
	if (m_usingSkill2)
	{
		proj = new IW_S2_Projectile(m_map, this, m_currentProjectile);
	}
	else if (m_usingUltimate)
	{
		proj = new IW_Ultimate_Projectile(m_map, this, m_currentProjectile);
	}
	else
	{
		proj = new Projectile(m_map, this, m_currentProjectile);
	}

	proj->m_position = m_position;
	Vec2 velocity = Vec2::MakeFromPolarDegrees(angle, force * 1.5f);
	proj->m_velocity = velocity;

	if (m_isFacingLeft && !m_characterDef->m_isMelee)
	{
		proj->m_orientationDegrees += (90 - (180 - angle));
	}
	else
	{
		proj->m_orientationDegrees -= (90 - angle);
	}

	if (m_usingSkill2)
	{
		proj->m_turnAliveLeft = IW_S2_ALIVE_PERIOD;
	}


	m_map->m_flyingProjectiles.push_back(proj);

	return proj;
}


IW_S2_Projectile::IW_S2_Projectile(Map* map, Character* owner, ProjectileDefinition* def)
	:Projectile(map, owner, def)
{
	m_trailEmitter = new Emitter(m_map->m_game->m_vfxSystem, m_position, 5.f);
	m_trailEmitter->SetTimeBetweenEachEmit(0.1f);
	m_trailEmitter->SetNumParticleEachEmit(2);
	m_trailEmitter->SetParticleLifeTime(0.5f);
	m_trailEmitter->SetParticleDefs(m_owner->m_vfxIndex);
}

void IW_S2_Projectile::Update(float deltaSeconds)
{
	if (!m_owner)return;

	if (!m_hasTouchedGround)
	{
		Projectile::Movement_Update(deltaSeconds);
		Projectile::Emitter_Update(deltaSeconds);
	}

	bool isMiss = false;
	if (m_position.y > (float)CHUNK_SIZE_Y) isMiss = true;
	if (m_position.x > m_map->m_bound.m_maxs.x || m_position.x < m_map->m_bound.m_mins.x)isMiss = true;

	// CHECK IF HIT THE GROUND
	if (!isMiss && !m_hasTouchedGround)
	{
		m_currentChunkCoord = RoundDownToInt(m_position.x / (float)CHUNK_SIZE_X);
		Chunk* currentChunk = m_map->m_activeChunks[m_currentChunkCoord];
		int blockIndex = currentChunk->GetBlockIndexFromGlobalPosition(m_position);

		if (currentChunk->m_blocks[blockIndex].IsBlockSolid())
		{
			m_map->m_game->SetCooldownCamera(2.f);
			m_hasTouchedGround = true;

			if (!m_owner->m_controller->IsPlayer())
			{
				IW_AI_Controller* AI = dynamic_cast<IW_AI_Controller*>(m_owner->m_controller);
				AI->m_hasS2TouchGround = true;
			}

			auto find = std::find(m_map->m_flyingProjectiles.begin(), m_map->m_flyingProjectiles.end(), this);
			if (find != m_map->m_flyingProjectiles.end())m_map->m_flyingProjectiles.erase(find);

			m_map->m_livingProjectiles.push_back(this);
		}
	}

	// EFFECT
	if (m_hasTouchedGround)
	{
		Chunk* currentChunk = m_map->m_activeChunks[m_currentChunkCoord];

		for (auto& character : m_map->m_characters)
		{
			if (IsEnemy(character))
			{
				if (DoDiscsOverlap2D(m_position, m_projectileDef->m_hitRadius, character->m_position, character->m_currentHitboxRadius))
				{
					int indexPlayer = currentChunk->GetBlockIndexFromGlobalPosition(m_position);
					BlockIterator iterPlayer = BlockIterator(currentChunk, indexPlayer);
					while (iterPlayer.GetBlock()->IsBlockSolid())
					{
						iterPlayer = iterPlayer.GetUpNeighbor();
					}

					Vec2 targetPos = iterPlayer.GetWorldCenter() + Vec2(0, character->m_currentVisualHalfLength.y);
					character->m_position = Interpolate(character->m_position, targetPos, deltaSeconds * 4);

					character->m_currentForceLimit.m_max = character->m_characterDef->m_forceLimit.m_max - m_forceReduction;
				}
			}
		}
	}

	PlaySoundOnImpact();

}

IW_Ultimate_Projectile::IW_Ultimate_Projectile(Map* map, Character* owner, ProjectileDefinition* def)
	:Projectile(map, owner, def)
{
	m_trailEmitter = new Emitter(m_map->m_game->m_vfxSystem, m_position, 10.f);
	m_trailEmitter->SetTimeBetweenEachEmit(0.1f);
	m_trailEmitter->SetNumParticleEachEmit(8);
	m_trailEmitter->SetParticleLifeTime(0.5f);
	m_trailEmitter->SetParticleDefs(m_owner->m_vfxIndex);
}

void IW_Ultimate_Projectile::Update(float deltaSeconds)
{
	if (!m_owner)return;

	Projectile::Movement_Update(deltaSeconds);
	Projectile::Emitter_Update(deltaSeconds);

	bool isDirectHit = false;
	Chunk* chunkHit = nullptr;
	if (!m_projectileDef->m_isTeleport)
	{
		for (auto& character : m_map->m_characters)
		{
			if (character->m_health == -1) continue;

			if (m_projectileDef->m_isHealing)
			{
				if (character->m_health == -1 || character->GetTeamID() != m_owner->GetTeamID()) continue;

				if (DoDiscsOverlap2D(m_position, m_projectileDef->m_hitRadius, character->m_position, character->m_currentHitboxRadius))
				{
					character->Heal(m_projectileDef->m_healingValue, m_projectileDef->m_canOverHeal);
					chunkHit = character->m_currentChunk;
					m_isHit = true;
					SpawnField(chunkHit);
					isDirectHit = true;
				}
			}
		}
	}

	if (!isDirectHit)
	{
		bool isMiss = false;
		if (m_position.y > (float)CHUNK_SIZE_Y) isMiss = true;
		if (m_position.x > m_map->m_bound.m_maxs.x || m_position.x < m_map->m_bound.m_mins.x)isMiss = true;

		// CHECK IF HIT THE GROUND
		if (!isMiss)
		{
			m_currentChunkCoord = RoundDownToInt(m_position.x / (float)CHUNK_SIZE_X);
			Chunk* currentChunk = m_map->m_activeChunks[m_currentChunkCoord];
			int blockIndex = currentChunk->GetBlockIndexFromGlobalPosition(m_position);

			if (currentChunk->m_blocks[blockIndex].IsBlockSolid())
			{
				m_hasTouchedGround = true;

				if (!m_owner->m_controller->IsPlayer())
				{
					IW_AI_Controller* AI = dynamic_cast<IW_AI_Controller*>(m_owner->m_controller);
					AI->m_hasUltTouchGround = true;
				}

				for (auto& character : m_map->m_characters)
				{
					if (character->m_health == -1 || character->GetTeamID() != m_owner->GetTeamID()) continue;

					if (DoDiscsOverlap2D(m_position, m_projectileDef->m_hitRadius, character->m_position, character->m_currentHitboxRadius))
					{
						character->Heal(m_projectileDef->m_healingValue, m_projectileDef->m_canOverHeal);
					}
				}

				m_isHit = true;
				SpawnField(currentChunk);
			}
		}
		else
		{
			m_map->m_game->SetCooldownCamera(1.f);
		}
	}


	PlaySoundOnImpact();
}

void IW_Ultimate_Projectile::SpawnField(Chunk* initialChunk)
{
	if (!m_owner)return;

	if (!initialChunk) return;

	Chunk* left = initialChunk->m_leftNeighbor;
	Chunk* right = initialChunk->m_rightNeighbor;

	IW_Ultimate_Field* fieldCenter = new IW_Ultimate_Field(m_map, m_owner, ProjectileDefinition::GetByName("IW_Ult_Field"));
	fieldCenter->SetPositionAndBound(initialChunk->GetWorldCenter());
	m_map->m_livingProjectiles.push_back(fieldCenter);

	if (left)
	{
		IW_Ultimate_Field* fieldLeft = new IW_Ultimate_Field(m_map, m_owner, ProjectileDefinition::GetByName("IW_Ult_Field"));
		fieldLeft->SetPositionAndBound(left->GetWorldCenter());
		m_map->m_livingProjectiles.push_back(fieldLeft);
	}
	if (right)
	{
		IW_Ultimate_Field* fieldRight = new IW_Ultimate_Field(m_map, m_owner, ProjectileDefinition::GetByName("IW_Ult_Field"));
		fieldRight->SetPositionAndBound(right->GetWorldCenter());
		m_map->m_livingProjectiles.push_back(fieldRight);
	}

}

IW_Ultimate_Field::IW_Ultimate_Field(Map* map, Character* owner, ProjectileDefinition* def)
	:Projectile(map, owner, def)
{
	m_turnAliveLeft = IW_ULT_ALIVE_PERIOD;
}

IW_Ultimate_Field::~IW_Ultimate_Field()
{
	for (auto& character : m_map->m_characters)
	{
		if (character->m_health == -1) continue;
		if (!IsEnemy(character))
		{
			if (DoDiscOverlapAABB2D(character->m_position, character->m_currentHitboxRadius, m_bound))
			{
				character->m_isInvisible = false;
			}
		}
	}
}

void IW_Ultimate_Field::BeginTurn()
{
	if (!m_owner)return;

	Projectile::BeginTurn();

	for (auto& character : m_map->m_characters)
	{
		if (character->m_health == -1) continue;
		if (!IsEnemy(character))
		{
			if (DoDiscOverlapAABB2D(character->m_position, character->m_currentHitboxRadius, m_bound))
			{
				character->Heal(200, false);
			}
		}

	}
}

void IW_Ultimate_Field::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);

	if (!m_owner)return;

	for (auto& character : m_map->m_characters)
	{
		if (character->m_health == -1) continue;
		if (!IsEnemy(character) )
		{
			if (DoDiscOverlapAABB2D(character->m_position, character->m_currentHitboxRadius, m_bound))
			{
				character->m_isInvisible = true;
				if (m_map->m_game->m_playerIDTurn == character->GetID())
				{
					character->m_isPendingInvisible = true;
				}
			}
		}
	}
}

void IW_Ultimate_Field::SetPositionAndBound(Vec2 position)
{
	if (!m_owner)return;
	m_position = position;
	m_bound = AABB2(m_position, m_projectileDef->m_visualHalfLength.y, m_projectileDef->m_visualHalfLength.x);
}

IW_AI_Controller::IW_AI_Controller(Game* owner, Character* character, char ID, char teamID)
	:AIController(owner, character, ID, teamID)
{

}

void IW_AI_Controller::UsingSkillLogic(float deltaSeconds)
{
	m_thinkingSkillTimer -= deltaSeconds;

	if (m_character->m_fury == 100)
	{
		m_ultThinkingTimer -= deltaSeconds;

		if (!m_characterToHeal)
		{
			m_characterToHeal = nullptr;
			for (auto& character : m_game->m_currentMap->m_characters)
			{
				if (character->m_health == -1) continue;
				if (character->GetTeamID() == m_teamID)
				{
					if (character->IsCurrentHealthPercentageLowerThan(0.7f))
					{
						m_characterToHeal = character;
						break;
					}
				}
			}
		}

		if (m_ultThinkingTimer < 0.f && m_characterToHeal)
		{
			SetCurrentForceToHitThisTarget(g_theRNG->RollRandomFloatInRange(55.f, 70.f), m_characterToHeal->m_position);
			m_hasUsedUltimate = true;
			m_character->Ultimate();
		}
	}

	if (m_hasUsedUltimate && !m_hasUltTouchGround)
	{
		return;
	}

	if (m_thinkingSkillTimer < 0.f)
	{
		if (!m_hasUsedSkill1 && m_character->ConditionsForSkill_1())
		{
			m_character->Skill_1();
			m_hasUsedSkill1 = true;
			m_thinkingSkillTimer = 1.f;
		}
	}

	if (m_thinkingSkillTimer < 0.f)
	{
		if (!m_hasUsedSkill2 && m_character->ConditionsForSkill_2())
		{
			m_character->Skill_2();
			if (m_character->IsPositionBehind(m_currentFocusTarget->m_position))
			{
				m_character->FaceTheOpposite();
			}
			SetCurrentForceToHitThisTarget(g_theRNG->RollRandomFloatInRange(30.f, 60.f), m_currentFocusTarget->m_position);
			m_currentAngle += GetRandomValueBasedOnSkillLevel(-2.f, 2.f);
			m_currentForce += GetRandomValueBasedOnSkillLevel(-7.f, 7.f);

			m_hasUsedSkill2 = true;
			m_thinkingSkillTimer = 2.f;
		}
	}
}

void IW_AI_Controller::NormalAttackLogic()
{
	if (m_hasS2TouchGround && !m_hasS2TouchGroundCheck)
	{
		m_hasS2TouchGroundCheck = true;
		m_thinkingTimer = 4.5f;
	}

	if (m_hasUltTouchGround && !m_hasUltTouchGroundCheck)
	{
		m_hasUltTouchGroundCheck = true;
		m_thinkingTimer = 4.5f;
	}

	if (m_thinkingTimer > 0.f) return;

	if (!m_hasChosenCharacterToHeal && !(m_hasUsedUltimate ^ m_hasUltTouchGround) && !(m_hasUsedSkill2 ^ m_hasS2TouchGround))
	{
		Character* characterToHeal = nullptr;
		for (auto& character : m_game->m_currentMap->m_characters)
		{
			if (character->m_health == -1) continue;
			if (character->GetTeamID() == m_teamID)
			{
				if (character->IsCurrentHealthPercentageLowerThan(0.7f))
				{
					characterToHeal = character;
					break;
				}
			}
		}

		if (characterToHeal)
		{
			m_hasChosenCharacterToHeal = true;
			SetCurrentForceToHitThisTarget(g_theRNG->RollRandomFloatInRange(30.f, 60.f), characterToHeal->m_position);
			m_currentAngle += GetRandomValueBasedOnSkillLevel(-2.f, 2.f);
			m_currentForce += GetRandomValueBasedOnSkillLevel(-7.f, 7.f);
			m_thinkingTimer = m_forceAndAngleThinkingTime;
			return;
		}
	}

	AIController::NormalAttackLogic();

	if (!m_hasResetNAFromS2 && m_hasUsedSkill2)
	{
		m_hasPerfomedNA = false;
		m_hasResetNAFromS2 = true;
		m_thinkingTimer = 15.f;
	}
	if (!m_hasResetNAFromUlt && m_hasUsedUltimate)
	{
		m_hasPerfomedNA = false;
		m_hasResetNAFromUlt = true;
		m_thinkingTimer = 15.f;
	}
}

void IW_AI_Controller::BeginTurn()
{
	m_hasChosenCharacterToHeal = false;

	m_hasResetNAFromS2 = false;
	m_hasS2TouchGround = false;
	m_hasS2TouchGroundCheck = false;

	m_hasResetNAFromUlt = false;
	m_hasUltTouchGround = false;
	m_hasUltTouchGroundCheck = false;

	AIController::BeginTurn();
}
