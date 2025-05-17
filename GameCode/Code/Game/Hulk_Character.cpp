#include "Game/Hulk_Character.hpp"

Hulk_Character::Hulk_Character(Map* map)
	:Character(map)
{
	m_characterDef = CharacterDefinition::GetByName("Hulk");
	Initialize();

	m_vfxIndex = { 20, 21, 22, 23 };

	m_affectedTileSet = AffectedTilesSet::HULK;

	m_jumpingAuraEmitter = new Emitter(m_map->m_game->m_vfxSystem, m_position, -1.f);
	m_jumpingAuraEmitter->SetEmitDirection(Vec2(0, 1));
	m_jumpingAuraEmitter->SetParticleRandomOffsetPosition(FloatRange(-4, 4), FloatRange(0, m_characterDef->m_hitboxRadius));
	m_jumpingAuraEmitter->SetParticleRandomOffsetDirAngle(FloatRange(-90, 90));
	m_jumpingAuraEmitter->SetNumParticleEachEmit(1);
	m_jumpingAuraEmitter->SetParticleLifeRandomInRange(FloatRange(0.4f, 0.7f));
	m_jumpingAuraEmitter->SetTimeBetweenEachEmit(0.15f);
	m_jumpingAuraEmitter->SetEmitSpeed(g_theRNG->RollRandomFloatInRange(1.f, 3.f));
	m_jumpingAuraEmitter->SetParticleDefs({ 20, 21, 22 });
}

Hulk_Character::~Hulk_Character()
{

}

void Hulk_Character::BeginTurn()
{
	Heal(100, false);
	m_ultDurationCounter--;
	if (m_ultDurationCounter < 0)
	{
		m_currentScale = 1.f;
		m_currentHitboxRadius = m_characterDef->m_hitboxRadius;
		m_currentVisualHalfLength = m_characterDef->m_visualHalfLength;
	}
	Character::BeginTurn();
}

void Hulk_Character::EndTurn()
{
	m_hasUsedSkill2 = false;
	Character::EndTurn();
}

void Hulk_Character::Update(float deltaSeconds)
{
	Character::Update(deltaSeconds);
	if (m_isJumping)
	{
		m_isFlying = false;
		if (!m_isGrounded)
		{
			m_jumpVelocity += Vec2(0, -69.8f) * deltaSeconds;
			m_position += m_jumpVelocity * deltaSeconds;
		}
		else
		{
			if (!m_hasResetAfterLanded)
			{
				m_hasResetAfterLanded = true;
				m_isJumping = false;
				m_isFlying = false;
				m_currentForceLimit = m_characterDef->m_forceLimit;
				ExplodeGround(m_currentHitboxRadius * 1.5f);
			}
		}

		m_jumpingAuraEmitter->m_position = m_position;
		m_jumpingAuraEmitter->m_lifeTime = 0.2f;

		m_map->m_game->GameCameraFollow(m_position);
	}
}

void Hulk_Character::Render() const
{
	if (m_isDead) return;
	if (m_isInvisible && !m_isPendingInvisible) return;

	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetDepthStencilMode(DepthMode::ENABLED);
	g_theRenderer->SetSamplerMode(SampleMode::POINT_CLAMP);
	Mat44 matrix = GetModelMatrix();
	matrix.AppendScaleUniform2D(m_currentScale);
	g_theRenderer->SetModelConstants(matrix, m_tintColor);
	g_theRenderer->BindTexture(m_sprite);
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->DrawVertexBuffer(m_vbuffer, m_verts.size());

	for (auto& currentItem : m_currentItems)
	{
		currentItem->Render();
	}

	RenderUI();
	RenderDebug();
}

void Hulk_Character::NormalAttack()
{
	if (m_isJumping)
	{
		m_jumpVelocity = Vec2::MakeFromPolarDegrees(m_currentAngle, m_currentForce * m_currentScale);
		m_hasResetAfterLanded = false;
		m_isFlying = true;
		ExplodeGround(m_currentHitboxRadius);
		m_currentForce = 0.f;
	}
	else
	{
		Character::NormalAttack();
	}
}

void Hulk_Character::Passive()
{
	// in begin turn
	if (m_map->m_game->m_playerIDTurn == GetID())
	{
		//if (m_isJumping)
		//{
		//	DebugAddScreenText(Stringf("Is Jumping : true"), Vec2(50, 750), 20.f);
		//}
		//else
		//{
		//	DebugAddScreenText(Stringf("Is Jumping : false"), Vec2(50, 750), 20.f);
		//}
	}
}

void Hulk_Character::Skill_1()
{
	if (!ConditionsForSkill_1()) return;

	Character::Skill_1();

	m_isJumping = true;
	m_currentForceLimit = FloatRange(0, 100);
}

void Hulk_Character::Skill_2()
{
	if (!ConditionsForSkill_2()) return;

	Character::Skill_2();

	m_hasUsedSkill2 = true;

	for (auto& character : m_map->m_characters)
	{
		if (character->m_health == -1 || character->GetTeamID() != GetTeamID()) continue;

		if (DoDiscsOverlap2D(m_position, 80.f, character->m_position, character->m_currentHitboxRadius))
		{
			Hulk_Shield* shield = new Hulk_Shield(m_map, character, ItemDefinition::GetByName("Hulk_S2"));
			character->ReceiveItem(shield);
		}
	}
}

void Hulk_Character::Ultimate()
{
	if (!ConditionsForUltimate()) return;

	Character::Ultimate();

	m_health = m_characterDef->m_initialHealth;
	m_ultDurationCounter = HULK_ULT_DURATION;
	m_currentScale = HULK_ULT_SCALE;
	m_map->ExplodeAtPosition(m_position, m_currentHitboxRadius * m_currentScale * 2.f, true, AffectedTilesSet::HULK);
	m_currentHitboxRadius = m_currentHitboxRadius * m_currentScale;
	m_currentVisualHalfLength = m_currentVisualHalfLength * m_currentScale;
}

bool Hulk_Character::ConditionsForSkill_1()
{
	return Character::ConditionsForSkill_1() && !m_isJumping;
}

bool Hulk_Character::ConditionsForSkill_2()
{
	return Character::ConditionsForSkill_2() && !m_isJumping && !m_hasUsedSkill2;
}

void Hulk_Character::ExplodeGround(float radius)
{
	Vec2 legPos = m_position - Vec2(0, m_currentVisualHalfLength.y - 1);
	int blockIndexAtLeg = m_currentChunk->GetBlockIndexFromGlobalPosition(legPos);
	BlockIterator iter = BlockIterator(m_currentChunk, blockIndexAtLeg);
	m_map->ExplodeAtPosition(iter.GetWorldCenter(), radius, true, AffectedTilesSet::HULK);

	for (auto& character : m_map->m_characters)
	{
		if (character->m_health == -1 || character->GetTeamID() == GetTeamID()) continue;

		if (DoDiscsOverlap2D(m_position, m_currentHitboxRadius, character->m_position, character->m_currentHitboxRadius))
		{
			character->TakeDamage(HULK_JUMP_DAMAGE, this, false);
		}
	}

	Emitter* emitter = new Emitter(m_map->m_game->m_vfxSystem, legPos, 0.5f);
	emitter->SetEmitDirectionRandom();
	emitter->SetNumParticleEachEmit(15);
	emitter->SetParticleLifeRandomInRange(FloatRange(0.2f, 0.5f));
	emitter->SetTimeBetweenEachEmit(0.07f);
	emitter->SetEmitSpeed(g_theRNG->RollRandomFloatInRange(20.f, 35.f));
	emitter->SetParticleDefs(m_vfxIndex);

	m_map->m_game->m_audioManager->PlaySFXSound(m_characterDef->m_S1Sound);
}


Hulk_Shield::Hulk_Shield(Map* map, Character* owner, ItemDefinition* def)
	:Item(map, owner, def)
{

}

void Hulk_Shield::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	m_position = m_owner->m_position;

	Vec2 scale = Vec2(m_owner->m_currentVisualHalfLength.x / m_itemDef->m_visualHalfLength.x, m_owner->m_currentVisualHalfLength.y / m_itemDef->m_visualHalfLength.y);
	scale *= 1.5f;
	m_matrixToRender.AppendScaleNonUniform2D(scale);

}

void Hulk_Shield::Render() const
{

	g_theRenderer->BindTexture(m_sprite);
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->SetModelConstants(m_matrixToRender);
	g_theRenderer->DrawVertexBuffer(m_vbuffer, m_verts.size());

	if (g_debugDraw)
	{

	}
}

Hulk_AI_Controller::Hulk_AI_Controller(Game* owner, Character* character, char ID, char teamID)
	:AIController(owner, character, ID, teamID)
{

}

void Hulk_AI_Controller::UsingSkillLogic(float deltaSeconds)
{
	m_thinkingSkillTimer -= deltaSeconds;

	if (m_thinkingSkillTimer < 0.f)
	{
		if (m_currentFocusTarget->m_position.y > m_character->m_position.y + 20.f)
		{
			m_currentFocusTarget = FindDifferentEnemy();
			m_thinkingTimer = m_forceAndAngleThinkingTime;
			m_thinkingSkillTimer = 2.f;
		}

		if (m_character->IsPositionBehind(m_currentFocusTarget->m_position))
		{
			m_character->FaceToTarget(m_currentFocusTarget->m_position);
		}

		if (m_character->ConditionsForSkill_2())
		{
			m_character->Skill_2();
			m_hasUsedSkill2 = true;
			m_thinkingSkillTimer = 1.f;
		}

		Vec2 toTarget = m_currentFocusTarget->m_position - m_character->m_position;

		if (toTarget.GetLengthSquared() > m_meleeDistance * m_meleeDistance + 5.f)
		{
			if (m_character->ConditionsForSkill_1())
			{
				m_currentAngle = 80.f;
				if (m_character->m_isFacingLeft)
				{
					m_currentAngle = 180.f - m_currentAngle;
				}
				m_currentForce = g_theRNG->RollRandomFloatInRange(70.f, 100.f);

				m_character->Skill_1();
				m_hasUsedSkill1 = true;
				m_isUsingSkill1 = true;
				m_thinkingSkillTimer = 4.f;
			}	
		}
	}
}

void Hulk_AI_Controller::NormalAttackLogic()
{
	if (m_thinkingTimer > 0.f) return;

	if (!m_hasPerfomedNA)
	{
		m_character->NormalAttack();

		if (m_isUsingSkill1)
		{
			m_hasPerfomedNA = false;
			m_isUsingSkill1 = false;
			m_thinkingTimer = m_forceAndAngleThinkingTime;
		}
		else
		{
			m_hasPerfomedNA = true;
		}

	}
}
