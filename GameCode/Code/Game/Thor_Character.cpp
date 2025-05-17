#include "Game/Thor_Character.hpp"


Thor_Character::Thor_Character(Map* map)
	:Character(map)
{
	m_characterDef = CharacterDefinition::GetByName("Thor");
	Initialize();

	m_dashTimer = new Timer(THOR_DASH_TIMER, m_map->m_game->m_gameClock);
	m_ultTimer = new Timer(1.5f, m_map->m_game->m_gameClock);

	m_vfxIndex = { 16, 17, 18, 19 };

	m_affectedTileSet = AffectedTilesSet::THOR;

	m_dashEmitter = new Emitter(m_map->m_game->m_vfxSystem, m_position, -1.f);
	m_dashEmitter->SetEmitDirection(Vec2(-1, 0));
	m_dashEmitter->SetParticleRandomOffsetDirAngle(FloatRange(-10, 10));
	m_dashEmitter->SetParticleRandomOffsetPosition(FloatRange(-m_currentVisualHalfLength.x, m_currentVisualHalfLength.x), FloatRange(1, 3));
	m_dashEmitter->SetParticleRandomOrientation(FloatRange(0.f, 359.f));
	m_dashEmitter->SetNumParticleEachEmit(4);
	m_dashEmitter->SetParticleLifeRandomInRange(FloatRange(0.1f, 0.3f));
	m_dashEmitter->SetTimeBetweenEachEmit(0.06f);
	m_dashEmitter->SetEmitSpeed(g_theRNG->RollRandomFloatInRange(75.f, 102.f));
	m_dashEmitter->SetParticleDefs({ 16, 17, 18, 19 });
}

Thor_Character::~Thor_Character()
{

}

void Thor_Character::BeginTurn()
{
	if (m_shieldUpPreviousTurn)
	{
		if (m_shield < THOR_SHIELD_VALUE)
		{
			m_shield = 0;
		}
		else
		{
			m_shield -= THOR_SHIELD_VALUE;
		}

		m_shieldUpPreviousTurn = false;
	}

	m_dashCount = 0;

	Character::BeginTurn();
}

void Thor_Character::NormalAttack()
{
	ThrowMjolnir(m_currentAngle);
	m_currentForce = m_characterDef->m_forceLimit.m_min;
	m_fury += FURY_PER_ATTACK;
	m_map->m_game->PendingEndTurn();
}

void Thor_Character::Update(float deltaSeconds)
{
	Ult_Update();

	S2_Update();

	Character::Update(deltaSeconds);

	m_dashEmitter->m_position = m_position;
}

void Thor_Character::Passive()
{
	// in NA

	if (m_map->m_game->m_playerIDTurn == GetID())
	{
		//DebugAddScreenText(Stringf("Dash Count Left :%i", 2 - m_dashCount), Vec2(50, 750), 20.f);
	}
}

void Thor_Character::Skill_1()
{
	if (!ConditionsForSkill_1()) return;

	Character::Skill_1();

	m_shieldUpPreviousTurn = true;

	m_shield += THOR_SHIELD_VALUE;
}

void Thor_Character::Skill_2()
{
	if (!ConditionsForSkill_2()) return;

	Character::Skill_2();

	m_dashCount++;
	m_isDashing = true;
	m_initialPositon = m_position;
	m_dashTimer->Start();

	GameRaycastResult2D raycast = m_map->RaycastWorld(m_position, Vec2::MakeFromPolarDegrees(m_currentAngle), THOR_DASH_DISTANCE);
	if (raycast.m_didImpact)
	{
		m_dashPositon = raycast.m_impactPos;
	}
	else
	{
		m_dashPositon = m_position + Vec2::MakeFromPolarDegrees(m_currentAngle, THOR_DASH_DISTANCE);
	}

	m_dashEmitter->m_lifeTime = 0.2f;
}

void Thor_Character::Ultimate()
{
	if (!ConditionsForUltimate()) return;

	Character::Ultimate();

	m_isUsingUlt = true;
	m_isFlying = true;
	m_initialPositon = m_position;


	Vec2 dir = Vec2(0.2f, 1.f).GetNormalized();
	GameRaycastResult2D checkUp = m_map->RaycastWorld(m_position, dir, THOR_ULT_TOP_Y);

	if (checkUp.m_didImpact)
	{
		m_ultTargetPositon = m_position + Vec2(0, checkUp.m_impactDist - m_currentVisualHalfLength.y - 1.f);
	}
	else
	{
		m_ultTargetPositon = m_position + dir * THOR_ULT_TOP_Y;
	}

	m_ultTimer->Start();
}

bool Thor_Character::ConditionsForSkill_2()
{
	return Character::ConditionsForSkill_2() && m_dashCount < THOR_DASH_LIMIT && !m_isDashing;
}

void Thor_Character::ThrowMjolnir(float angle)
{
	Thor_NA_Projectile* p = new Thor_NA_Projectile(m_map, this, ProjectileDefinition::GetByName("Thor_NA"));
	p->m_position = m_position;
	Vec2 velocity = Vec2::MakeFromPolarDegrees(angle, THOR_NA_FORCE);
	p->m_velocity = velocity;

	if (m_isFacingLeft)
	{
		p->m_orientationDegrees += (90 - (180 - m_currentAngle));
	}
	else
	{
		p->m_orientationDegrees -= (90 - m_currentAngle);
	}

	m_map->m_flyingProjectiles.push_back(p);

	m_canUseSKill2 = false;
}

void Thor_Character::S2_Update()
{
	if (!m_isDashing) return;

	if (!m_dashTimer->HasPeriodElapsed())
	{
		m_position = Interpolate(m_initialPositon, m_dashPositon, SmoothStop5(m_dashTimer->GetElapsedFraction()));

		m_map->ExplodeAtPosition(m_position, m_currentHitboxRadius, true, AffectedTilesSet::THOR, false);

		for (auto& character : m_map->m_characters)
		{
			if (character->m_health == -1 || character->GetTeamID() == GetTeamID()) continue;

			auto foundCharacterGotHit = std::find(m_charactersHitByDash.begin(), m_charactersHitByDash.end(), character);

			if (foundCharacterGotHit == m_charactersHitByDash.end())
			{
				if (DoDiscsOverlap2D(m_position, m_currentHitboxRadius, character->m_position, character->m_currentHitboxRadius))
				{
					m_charactersHitByDash.push_back(character);
					character->TakeDamage(THOR_S2_DAMAGE, this, false);
				}
			}
		}

		m_map->m_game->GameCameraFollow(m_position);
	}
	else
	{
		m_charactersHitByDash.clear();
		m_isDashing = false;
		m_dashTimer->Stop();
	}
}

void Thor_Character::Ult_Update()
{
	if (!m_isUsingUlt) return;

	m_map->m_game->GameCameraFollow(m_position);

	if (!m_ultTimer->HasPeriodElapsed())
	{
		float t = m_ultTimer->GetElapsedFraction();
		if (m_ultStage == 0) t = SmoothStop4(m_ultTimer->GetElapsedFraction());
		if (m_ultStage == 1) t = SmoothStep3(m_ultTimer->GetElapsedFraction());
		if (m_ultStage == 2) t = SmoothStop6(m_ultTimer->GetElapsedFraction());
		m_position = Interpolate(m_initialPositon, m_ultTargetPositon, t);

		if (m_ultStage == 2)
		{
			if (m_ultTimer->GetElapsedFraction() > 0.4f)
			{
				m_map->ExplodeAtPosition(m_position, THOR_ULT_DESTRUCT_RADIUS, true, AffectedTilesSet::THOR);


				Emitter* emitter = new Emitter(m_map->m_game->m_vfxSystem, m_position, 0.2f);
				emitter->SetEmitDirectionRandom();
				emitter->SetNumParticleEachEmit(8);
				emitter->SetParticleRandomOrientation(FloatRange(0.f, 359.f));
				emitter->SetParticleLifeRandomInRange(FloatRange(0.2f, 0.5f));
				emitter->SetTimeBetweenEachEmit(0.07f);
				emitter->SetEmitSpeed(m_velocity.GetLength() * g_theRNG->RollRandomFloatInRange(0.5f, 0.7f));
				emitter->SetParticleDefs(m_vfxIndex);
			}
			else
			{
				m_map->ExplodeAtPosition(m_position, m_currentHitboxRadius, false);

				Emitter* emitter = new Emitter(m_map->m_game->m_vfxSystem, m_position, 0.2f);
				emitter->SetEmitDirectionRandom();
				emitter->SetNumParticleEachEmit(8);
				emitter->SetParticleRandomOrientation(FloatRange(0.f, 359.f));
				emitter->SetParticleLifeRandomInRange(FloatRange(0.2f, 0.5f));
				emitter->SetTimeBetweenEachEmit(0.07f);
				emitter->SetEmitSpeed(m_velocity.GetLength() * g_theRNG->RollRandomFloatInRange(0.5f, 0.7f));
				emitter->SetParticleDefs(m_vfxIndex);
				
			}
		}
	}
	else
	{
		if (m_ultTimer->DecrementPeriodIfElapsed())
		{
			m_ultStage++;
		}

		if (m_ultStage == 1)
		{
			m_initialPositon = m_position;
			m_ultTimer->m_period = 1.0f;
			Character* nearestEnenmy = GetNearestEnemy();
			if (nearestEnenmy)
			{
				float distance = (nearestEnenmy->m_position - m_position).GetLength();
				if (distance < THOR_ULT_FINDING_NEAREST_ENEMY_DISTANCE_X)
				{
					m_ultTargetPositon.x = nearestEnenmy->m_position.x;
				}

				m_map->m_game->m_controllerTookTurn.push_back(nearestEnenmy->m_controller);
			}
		}

		if (m_ultStage == 2)
		{
			m_initialPositon = m_position;
			m_ultTimer->m_period = 0.6f;

			GameRaycastResult2D checkDown = m_map->RaycastWorld(m_initialPositon, Vec2(0, -1), CHUNK_SIZE_Y);

			if (checkDown.m_didImpact)
			{
				m_ultTargetPositon = checkDown.m_impactPos;
			}
			else
			{
				m_ultStage = 3;
			}

		}

		if (m_ultStage == 3)
		{
			m_isUsingUlt = false;
			m_isFlying = false;
			m_ultStage = 0;
			m_ultTimer->Stop();
			m_map->m_game->m_audioManager->PlaySFXSound("Thor_Ult_C", 10);
		}
	}
}

Thor_NA_Projectile::Thor_NA_Projectile(Map* map, Character* owner, ProjectileDefinition* def)
	:Projectile(map, owner, def)
{

}

void Thor_NA_Projectile::Update(float deltaSeconds)
{
	if (m_distanceTravelled > m_projectileDef->m_meleeMaxRange) // FLIP DIRECTION WHEN REAHC MAX RANGE
	{
		m_direction = -1;
	}

	if (m_direction == -1 && DoDiscsOverlap2D(m_position, m_projectileDef->m_hitRadius, m_owner->m_position, m_owner->m_currentHitboxRadius)) // HAS RETURNED TO THOR
	{
		m_isHit = true;
		m_hasReturned = true;
		m_map->m_game->SetCooldownCamera(1.f);
	}

	if (!m_hasReturned)
	{
		Vec2 newPos = m_position + m_velocity * (float)m_direction * deltaSeconds;
		m_distanceTravelled = m_distanceTravelled + (newPos - m_position).GetLength() * (float)m_direction;
		m_position = newPos;

		float goalAngle = m_velocity.GetOrientationDegrees() - m_projectileDef->m_offsetAngle;
		if (m_direction == -1) goalAngle -= 180.f;
		m_orientationDegrees = GetTurnedTowardDegrees(m_orientationDegrees, goalAngle, 720 * deltaSeconds);

		bool isDirectHit = false;


		for (auto& character : m_map->m_characters)
		{
			if (character->m_health == -1) continue;

			if (IsEnemy(character))
			{
				if (DoDiscsOverlap2D(m_position, m_projectileDef->m_hitRadius, character->m_position, character->m_currentHitboxRadius))
				{
					bool isCrit = false;
					int damage = int(m_owner->CalculateDamage(isCrit,0.f, 0.f));

					auto found = std::find(m_charactersGotHit.begin(), m_charactersGotHit.end(), character);

					if (found == m_charactersGotHit.end())
					{
						if (m_direction == 1)
						{
							m_charactersGotHit.push_back(character);
							character->TakeDamage(damage, m_owner, false);
							character->m_speed -= 10;
							m_map->m_game->m_screenShakeAmount = 1.f;
							PlaySoundOnImpact(true);
						}
					}
					else
					{
						if (m_direction == -1)
						{
							m_charactersGotHit.erase(found); 
							int returnDamage = RoundDownToInt((float)damage * 0.5f);
							character->TakeDamage(returnDamage, m_owner, false);
							m_map->m_game->m_screenShakeAmount = 1.f;
							PlaySoundOnImpact(true);
						}
					}
				}

				isDirectHit = true;

			}
		}

		bool changeTerrain = !m_projectileDef->m_isTeleport && !m_projectileDef->m_isHealing;

		bool isMiss = false;
		if (m_position.y > (float)CHUNK_SIZE_Y) isMiss = true;
		if (m_position.x > m_map->m_bound.m_maxs.x || m_position.x < m_map->m_bound.m_mins.x)isMiss = true;

		bool affectEnemy = false;

		// CHECK IF HIT THE GROUND
		if (!isMiss)
		{
			m_currentChunkCoord = RoundDownToInt(m_position.x / (float)CHUNK_SIZE_X);
			Chunk* currentChunk = m_map->m_activeChunks[m_currentChunkCoord];
			int blockIndex = currentChunk->GetBlockIndexFromGlobalPosition(m_position);

			if (currentChunk->m_blocks[blockIndex].IsBlockSolid())
			{
				m_hasTouchedGround = true;
				m_hitPosition = BlockIterator(currentChunk, blockIndex).GetWorldCenter();

				if (!isDirectHit)
				{
					for (auto& character : m_map->m_characters)
					{
						if (character->m_health == -1) continue;

						if (DoDiscsOverlap2D(m_position, m_projectileDef->m_destructionRadius, character->m_position, character->m_currentHitboxRadius))
						{
							float bonusCritRate = (m_reachedSuperHigh) ? 0.15f : 0.f;
							float bonusCritDamage = (m_reachedSuperHigh) ? 0.05f : 0.f;
							bool isCrit = false;
							auto damage = int(m_owner->CalculateDamage(isCrit, bonusCritRate, bonusCritDamage) * m_projectileDef->m_damageScale);

							if (IsEnemy(character))
							{
								damage /= 2;
								affectEnemy = true;
								m_owner->m_fury += 5;
							}
							else
							{
								if (m_projectileDef->m_isHealing) // NO IMPACT DAMAGE TO TEAMMATE IF IT'S HEALING
								{
									damage = 0;
								}
								else
								{
									damage /= 3;
								}
							}

							character->TakeDamage(damage, m_owner, isCrit);
						}
					}
				}

				m_map->ExplodeAtPosition(m_position, m_projectileDef->m_destructionRadius, changeTerrain, AffectedTilesSet::THOR);
				PlayExplodeEffect();
				PlaySoundOnImpact();
				m_isHit = true;
			}
		}
		else
		{
			m_map->m_game->SetCooldownCamera(1.f);
		}

		if (m_direction == 1 && m_isHit)
		{
			m_isHit = false;
			m_direction = -1;
		}
	}
}

void Thor_NA_Projectile::Render() const
{
	if (m_hasReturned) return;
	Projectile::Render();
}

void Thor_NA_Projectile::Die()
{
	m_charactersGotHit.clear();
	Projectile::Die();
}

Thor_AI_Controller::Thor_AI_Controller(Game* owner, Character* character, char ID, char teamID)
	:AIController(owner, character, ID, teamID)
{

}

void Thor_AI_Controller::UsingSkillLogic(float deltaSeconds)
{
	m_thinkingSkillTimer -= deltaSeconds;

	if (m_thinkingSkillTimer < 0.f)
	{
		if (m_currentFocusTarget->m_position.y > m_character->m_position.y + THOR_DASH_DISTANCE)
		{
			m_currentFocusTarget = FindDifferentEnemy();
			m_thinkingTimer = m_forceAndAngleThinkingTime;
			m_thinkingSkillTimer = 2.f;
		}

		if (m_character->IsPositionBehind(m_currentFocusTarget->m_position))
		{
			m_character->FaceToTarget(m_currentFocusTarget->m_position);
		}

		Vec2 toTarget = m_currentFocusTarget->m_position - m_character->m_position;

		if (toTarget.GetLengthSquared() > m_meleeDistance * m_meleeDistance + 5.f)
		{
			float angleToTarget = toTarget.GetOrientationDegrees();
			GameRaycastResult2D rayCheck = m_game->m_currentMap->RaycastWorld(m_character->m_position, Vec2::MakeFromPolarDegrees(angleToTarget), THOR_DASH_DISTANCE);

			if (m_thinkingTimer > 0.f)
			{
				while (m_character->ConditionsForSkill_2() && !m_dashNearEnough)
				{
					rayCheck = m_game->m_currentMap->RaycastWorld(m_character->m_position, Vec2::MakeFromPolarDegrees(angleToTarget), THOR_DASH_DISTANCE);

					if (rayCheck.m_didImpact)
					{
						if (m_character->m_isFacingLeft)
						{
							angleToTarget -= 25.f;
							angleToTarget = Clamp(angleToTarget, m_character->m_currentAngleLimit.m_max, m_character->m_currentAngleLimit.m_min);
						}
						else
						{
							angleToTarget += 25.f;
							angleToTarget = Clamp(angleToTarget, m_character->m_currentAngleLimit.m_min, m_character->m_currentAngleLimit.m_max);
						}

						m_currentAngle = angleToTarget;
						m_thinkingSkillTimer = THOR_DASH_TIMER + .5f;
					}
					else
					{
						m_character->Skill_2();
					}
				}
			}

		}
		else
		{
			m_dashNearEnough = true;
			m_character->Skill_1();
		}
	}

}

void Thor_AI_Controller::NormalAttackLogic()
{
	if (m_thinkingSkillTimer > 0.f || m_thinkingTimer > 0.f) return;

	if (!m_hasAdjustedNAToNearestEnemy)
	{
		Character* nearestE = m_character->GetNearestEnemy();
		Vec2 toTarget = m_currentFocusTarget->m_position - m_character->m_position;

		if (m_currentFocusTarget != nearestE && toTarget.GetLengthSquared() > m_meleeDistance * m_meleeDistance)
		{
			SetAngleToPosition(nearestE->m_position);
			m_thinkingTimer = m_forceAndAngleThinkingTime;
			m_thinkingSkillTimer = 1.f;
		}
		m_hasAdjustedNAToNearestEnemy = true;
	}

	AIController::NormalAttackLogic();
}

void Thor_AI_Controller::BeginTurn()
{
	m_hasAdjustedNAToNearestEnemy = false;
	m_dashNearEnough = false;
	AIController::BeginTurn();

	m_currentFocusTarget = FindLowestInitialHPEnemy();
}
