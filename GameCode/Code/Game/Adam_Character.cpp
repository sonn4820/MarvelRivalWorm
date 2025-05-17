#include "Game/Adam_Character.hpp"

Adam_Character::Adam_Character(Map* map)
	:Character(map)
{
	m_characterDef = CharacterDefinition::GetByName("Adam");
	Initialize();

	m_vfxIndex = { 4, 5, 6 };

	m_affectedTileSet = AffectedTilesSet::ADAM;

	m_skillEmitter = new Emitter(m_map->m_game->m_vfxSystem, m_position, -1.f);
	m_skillEmitter->SetEmitDirection(Vec2(0, 1));
	m_skillEmitter->SetParticleRandomOffsetPosition(FloatRange(-4, 4), FloatRange(-3, 1));
	m_skillEmitter->SetNumParticleEachEmit(2);
	m_skillEmitter->SetParticleLifeRandomInRange(FloatRange(0.4f, 0.7f));
	m_skillEmitter->SetTimeBetweenEachEmit(0.07f);
	m_skillEmitter->SetEmitSpeed(g_theRNG->RollRandomFloatInRange(12.f, 17.f));
	m_skillEmitter->SetParticleDefs({ 7 });
}

Adam_Character::~Adam_Character()
{

}

void Adam_Character::Update(float deltaSeconds)
{
	Character::Update(deltaSeconds);

	m_skillEmitter->m_position = m_position;
}

void Adam_Character::EndTurn()
{
	m_useHealing = false;
	Character::EndTurn();
}

void Adam_Character::NormalAttack()
{
	if (!m_useHealing)
	{
		m_currentProjectile = ProjectileDefinition::GetByName("Adam_NA");
		ShootCurrentProjectile(m_currentAngle + 5, m_currentForce);
		ShootCurrentProjectile(m_currentAngle - 5, m_currentForce);
	}
	else
	{
		m_currentProjectile = ProjectileDefinition::GetByName("Adam_S2");
	}

	ShootCurrentProjectile(m_currentAngle, m_currentForce);

	m_currentForce = 0.f;

	m_fury += FURY_PER_ATTACK;

	m_map->m_game->PendingEndTurn();
}

void Adam_Character::Passive()
{
	// In Die()
	if (m_map->m_game->m_playerIDTurn == GetID())
	{
		//if (m_hasPassive)
		//{
		//	DebugAddScreenText(Stringf("Can Revive: true"), Vec2(50, 750), 20.f);
		//}
		//else
		//{
		//	DebugAddScreenText(Stringf("Can Revive: false"), Vec2(50, 750), 20.f);
		//}

	}
}

void Adam_Character::Skill_1()
{
	if (!ConditionsForSkill_1()) return;

	Character::Skill_1();

	for (auto& character : m_map->m_characters)
	{
		if (character->GetTeamID() == GetTeamID())
		{
			character->m_attack += 25;

			Emitter* charEmitter = new Emitter(m_map->m_game->m_vfxSystem, character->m_position, 0.6f);
			charEmitter->SetEmitDirection(Vec2(0, 1));
			charEmitter->SetParticleRandomOffsetPosition(FloatRange(-4, 4), FloatRange(-3, 1));
			charEmitter->SetNumParticleEachEmit(2);
			charEmitter->SetParticleLifeRandomInRange(FloatRange(0.4f, 0.7f));
			charEmitter->SetTimeBetweenEachEmit(0.07f);
			charEmitter->SetEmitSpeed(g_theRNG->RollRandomFloatInRange(12.f, 17.f));
			charEmitter->SetParticleDefs({ 7 });
		}
	}

	m_skillEmitter->m_lifeTime = 0.6f;
}

void Adam_Character::Skill_2()
{
	if (!ConditionsForSkill_2()) return;

	Character::Skill_2();

	m_useHealing = true;
}

void Adam_Character::Ultimate()
{
	if (!ConditionsForUltimate()) return;

	Character::Ultimate();

	for (auto& character : m_map->m_characters)
	{
		if (character->GetTeamID() == GetTeamID())
		{
			if (character->m_health == -1)
			{
				Grave* deadCharacter = dynamic_cast<Grave*>(character);
				if (deadCharacter)
				{
					Character* respawnCharacter = m_map->SpawnCharacter(deadCharacter->m_previousCharacterDefinition, m_position + Vec2(0, 5));
					deadCharacter->m_controller->m_character = respawnCharacter;
					respawnCharacter->m_controller = deadCharacter->m_controller;
					respawnCharacter->m_health /= 2;
					if (respawnCharacter->GetTeamID() != 1)
					{
						respawnCharacter->FaceLeft();
					}
					deadCharacter->Die();
				}

			}
			else
			{
				character->m_shield += 400;
			}
		}
	}

	m_map->m_game->ChangeUI();
}

void Adam_Character::Die()
{
	if (!m_hasPassive)
	{
		Character::Die();
	}
	else
	{
		m_isDead = false;

		if (m_position.x > m_map->m_bound.m_maxs.x) m_position.x = m_map->m_bound.m_maxs.x - 10;
		if (m_position.x < m_map->m_bound.m_mins.x) m_position.x = m_map->m_bound.m_mins.x + 10;
		if (m_position.y < m_map->m_bound.m_mins.y) m_position.y = (float)m_currentChunk->m_terrainHeightList[RoundDownToInt(m_position.x / (float)CHUNK_SIZE_X)] + 1;

		m_health = m_characterDef->m_initialHealth / 2;

		m_hasPassive = false;
	}
}

bool Adam_Character::ConditionsForSkill_2()
{
	return Character::ConditionsForSkill_2() && !m_useHealing;
}

Adam_AI_Controller::Adam_AI_Controller(Game* owner, Character* character, char ID, char teamID)
	:AIController(owner, character, ID, teamID)
{

}

void Adam_AI_Controller::UsingSkillLogic(float deltaSeconds)
{
	if (m_character->m_fury == 100)
	{
		m_ultThinkingTimer -= deltaSeconds;

		if (!m_ultfoundDeadTeammate)
		{
			for (auto& character : m_game->m_currentMap->m_characters)
			{
				if (character->GetTeamID() == m_teamID && character->m_health == -1)
				{
					m_ultfoundDeadTeammate = true;
				}
			}
		}

		if (m_ultThinkingTimer < 0.f && m_ultfoundDeadTeammate)
		{
			m_character->Ultimate();
			m_ultfoundDeadTeammate = false;
		}
	}

	Character* characterToHeal = nullptr;

	if (m_character->m_canUseSKill2)
	{
		for (auto& character : m_game->m_currentMap->m_characters)
		{
			if (character->m_health == -1) continue;
			if (character->GetTeamID() == m_teamID)
			{
				if (character->IsCurrentHealthPercentageLowerThan(0.5f))
				{
					characterToHeal = character;
					break;
				}
			}
		}
	}

	if (characterToHeal)
	{
		if (!m_hasUsedSkill2 && m_character->ConditionsForSkill_2())
		{
			m_character->Skill_2();

			if (m_character->IsPositionBehind(characterToHeal->m_position))
			{
				m_character->FaceTheOpposite();
			}
			SetCurrentForceToHitThisTarget(g_theRNG->RollRandomFloatInRange(30.f, 60.f), characterToHeal->m_position);
			m_currentAngle += GetRandomValueBasedOnSkillLevel(-2.f, 2.f);
			m_currentForce += GetRandomValueBasedOnSkillLevel(-7.f, 7.f);
			m_hasUsedSkill2 = true;
			m_thinkingTimer = m_forceAndAngleThinkingTime;
		}
	}
	else
	{
		if (m_character->m_fury != 100)
		{
			while (m_character->ConditionsForSkill_1())
			{
				m_character->Skill_1();
				m_hasUsedSkill1 = true;
			}
		}
	}
}