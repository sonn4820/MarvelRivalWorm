#include "Game/CharacterCommon.hpp"
#include "Game/GameCommon.hpp"
#include "Game/AllCharacters.hpp"
#include "Game/AIController.hpp"


std::vector<CharacterDefinition*> CharacterDefinition::s_characterDefList;

CharacterDefinition::CharacterDefinition(XmlElement& element)
	:m_name(ParseXmlAttribute(element, "name", "")),
	m_initialHealth(ParseXmlAttribute(element, "health", 0)),
	m_initialSpeed(ParseXmlAttribute(element, "speed", 0)),
	m_initialStamina(ParseXmlAttribute(element, "stamina", 0.f)),
	m_initialArmor(ParseXmlAttribute(element, "armor", 0)),
	m_initialAttack(ParseXmlAttribute(element, "attack", 0)),
	m_angleLimit(ParseXmlAttribute(element, "angleLimit", FloatRange())),
	m_forceLimit(ParseXmlAttribute(element, "forceLimit", FloatRange())),
	m_gravitySpeed(ParseXmlAttribute(element, "gravity", 9.8f)),
	m_skill1Cost(ParseXmlAttribute(element, "skill1Cost", 0)),
	m_skill2Cost(ParseXmlAttribute(element, "skill2Cost", 0)),
	m_skill1Cooldown(ParseXmlAttribute(element, "skill1Cooldown", 0)),
	m_skill2Cooldown(ParseXmlAttribute(element, "skill2Cooldown", 0)),
	m_isMelee(ParseXmlAttribute(element, "isMelee", false))
{

}

void CharacterDefinition::SetCollision(XmlElement& element)
{
	m_hitboxRadius = ParseXmlAttribute(element, "radius", 0.f);
}

void CharacterDefinition::SetVisual(XmlElement& element)
{
	m_visualHalfLength = ParseXmlAttribute(element, "halfLength", Vec2());
	m_characterTextureName = ParseXmlAttribute(element, "image", "");
}

void CharacterDefinition::SetProjectiles(XmlElement& element)
{
	XmlElement* projectileElement = element.FirstChildElement();
	while (projectileElement)
	{
		std::string name = projectileElement->Name();
		GUARANTEE_OR_DIE(name == "Projectile", "Root child element in Weapon is in the wrong format");

		std::string projectileName = ParseXmlAttribute(*projectileElement, "name", "");
		m_projectiles.push_back(ProjectileDefinition::GetByName(projectileName));
		projectileElement = projectileElement->NextSiblingElement();
	}
}

void CharacterDefinition::SetSound(XmlElement& element)
{
	XmlElement* soundElement = element.FirstChildElement();
	while (soundElement)
	{
		std::string name = soundElement->Name();
		GUARANTEE_OR_DIE(name == "Sound", "Root child element in Sound is in the wrong format");

		int slot = ParseXmlAttribute(*soundElement, "slot", 0);
		switch (slot)
		{
		case 0:
			m_throwSound = ParseXmlAttribute(*soundElement, "name", "");
			break;
		case 1:
			m_S1Sound = ParseXmlAttribute(*soundElement, "name", "");
			break;
		case 2:
			m_S2Sound = ParseXmlAttribute(*soundElement, "name", "");
			break;
		case 3:
			m_UltSound = ParseXmlAttribute(*soundElement, "name", "");
			break;
		}

		soundElement = soundElement->NextSiblingElement();
	}
}

void CharacterDefinition::SetIcon(XmlElement& element)
{
	UNUSED(element);
}

void CharacterDefinition::InitializeDefs(char const* filePath)
{
	XmlDocument file;
	XmlError result = file.LoadFile(filePath);
	GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS, "FILE IS NOT LOADED");

	XmlElement* rootElement = file.RootElement();
	GUARANTEE_OR_DIE(rootElement, "Root Element is null");

	XmlElement* characterDefElement = rootElement->FirstChildElement();

	while (characterDefElement)
	{
		std::string name = characterDefElement->Name();
		GUARANTEE_OR_DIE(name == "CharacterDefinition", "Root child element is in the wrong format");

		CharacterDefinition* newActorDef = new CharacterDefinition(*characterDefElement);

		XmlElement* actorChildElement = characterDefElement->FirstChildElement();
		while (actorChildElement)
		{
			std::string sectionName = actorChildElement->Name();
			if (sectionName == "Collision")
			{
				newActorDef->SetCollision(*actorChildElement);
			}
			if (sectionName == "Visuals")
			{
				newActorDef->SetVisual(*actorChildElement);
			}
			if (sectionName == "Weapon")
			{
				newActorDef->SetProjectiles(*actorChildElement);
			}
			if (sectionName == "Sounds")
			{
				newActorDef->SetSound(*actorChildElement);
			}
			if (sectionName == "UI")
			{
				newActorDef->SetIcon(*actorChildElement);
			}
			actorChildElement = actorChildElement->NextSiblingElement();
		}
		s_characterDefList.push_back(newActorDef);
		characterDefElement = characterDefElement->NextSiblingElement();
	}
}

void CharacterDefinition::ClearDefinition()
{
	for (auto& i : s_characterDefList)
	{
		if (i != nullptr)
		{
			delete i;
			i = nullptr;
		}
	}
}

CharacterDefinition* CharacterDefinition::GetByName(std::string name)
{
	for (auto& i : s_characterDefList)
	{
		if (i->m_name == name)
		{
			return i;
		}
	}
	return nullptr;
}

Character::Character(Map* map)
	:Entity(map)
{
	m_UIDamageTextTimer = new Timer(1.f,m_map->m_game->m_gameClock);
}


Character::~Character()
{

}

void Character::CleanUp()
{
	g_theEventSystem->UnsubscribeEventCallbackMemberFunction(m_characterDef->m_name, (Character*)this, &Character::Command_ModifyStats);

	if (!m_controller->IsPlayer())
	{
		g_theEventSystem->UnsubscribeEventCallbackMemberFunction(m_characterDef->m_name + "AI", (AIController*)this, &AIController::Command_ModifyAI);
	}
	m_controller->m_character = nullptr;

	auto foundControllerTookTurn = std::find(m_map->m_game->m_controllerTookTurn.begin(), m_map->m_game->m_controllerTookTurn.end(), m_controller);
	if (foundControllerTookTurn != m_map->m_game->m_controllerTookTurn.end())  m_map->m_game->m_controllerTookTurn.erase(foundControllerTookTurn);
	m_controller = nullptr;

	for (auto& controller : m_map->m_game->m_controllers)
	{
		if (!controller->IsPlayer())
		{
			AIController* ai = dynamic_cast<AIController*>(controller);
			if (ai->m_currentFocusTarget == this)
			{
				ai->m_currentFocusTarget = nullptr;
			}
		}
	}

	for (auto& livingProjectile : m_map->m_livingProjectiles)
	{
		if (livingProjectile && livingProjectile->m_owner == this)
		{
			livingProjectile->m_owner = nullptr;
			livingProjectile->Die();
		}
	}
}

void Character::Update(float deltaSeconds)
{
	if (m_isDead) return;

	if (m_isPendingInvisible)
	{
		m_tintColor = Rgba8(255, 255, 255, 100);
	}
	else
	{
		m_tintColor = Rgba8::COLOR_WHITE;
	}

	m_currentChunk = m_map->GetChunkAtPosition(m_position);
	GravityAndFloorCollision(deltaSeconds);

	if (m_health == -1) return;

	if (m_position.x > m_map->m_bound.m_maxs.x
		|| m_position.x < m_map->m_bound.m_mins.x
		|| m_position.y < m_map->m_bound.m_mins.y)
	{
		if (GetID() == m_map->m_game->m_playerIDTurn)
		{
			m_map->m_game->SetCooldownCamera(1.f);
			m_map->m_game->PendingEndTurn();
			m_map->m_game->m_playerIDTurn = -1;
		}
		Die();
	}

	Passive();

	for (auto& currentItem : m_currentItems)
	{
		currentItem->m_matrixToRender = GetModelMatrix();
		currentItem->Update(deltaSeconds);
	}

	m_fury = (int)Clamp(m_fury, 0, 100);

	if (!m_controller->m_isMoving)
	{
		if (m_currentWalkingGroundSound != MISSING_SOUND_ID) g_theAudio->StopSound(m_currentWalkingGroundSound);
		if(m_currentWalkingWaterSound!= MISSING_SOUND_ID) g_theAudio->StopSound(m_currentWalkingWaterSound);
	}

	if (!m_UIDamageTextTimer->HasPeriodElapsed())
	{
		m_UIDamageTextMatrix.AppendTranslation2D(Vec2(0, 15.f) * deltaSeconds * SmoothStop6(m_UIDamageTextTimer->GetElapsedFraction()));
	}
}

void Character::Render() const
{
	if (m_isDead) return;

	if (m_isInvisible && !m_isPendingInvisible) return;

	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetDepthStencilMode(DepthMode::ENABLED);
	g_theRenderer->SetSamplerMode(SampleMode::POINT_CLAMP);
	g_theRenderer->SetModelConstants(GetModelMatrix(), m_tintColor);
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

void Character::RenderUI() const
{
	if (m_health != -1)
	{
		std::vector<Vertex_PCU> UIVerts;
		float minBox = -3;
		float maxBox = 3;
		float minY = m_currentVisualHalfLength.y + 1.f;
		float maxY = minY + 1.f;
		float offsetBorder = 0.15f;
		AddVertsForAABB2D(UIVerts, AABB2(m_position + Vec2(minBox - offsetBorder, minY - offsetBorder), m_position + Vec2(maxBox + offsetBorder, maxY + offsetBorder)), Rgba8::COLOR_BLACK);
		float hp = RangeMapClamped((float)m_health, 0.f, (float)m_characterDef->m_initialHealth, minBox, maxBox);
		AddVertsForAABB2D(UIVerts, AABB2(m_position + Vec2(minBox, minY), m_position + Vec2(hp, maxY)), m_UIColor);
		float shield = RangeMapClamped((float)m_shield, 0.f, (float)m_characterDef->m_initialHealth, maxBox, minBox);
		AddVertsForAABB2D(UIVerts, AABB2(m_position + Vec2(shield, minY), m_position + Vec2(maxBox, maxY)), Rgba8::COLOR_YELLOW);
		g_theRenderer->SetModelConstants();
		g_theRenderer->BindTexture(nullptr);
		g_theRenderer->DrawVertexArray(UIVerts.size(), UIVerts.data());

		if (!m_UIDamageTextTimer->HasPeriodElapsed())
		{
			g_theRenderer->SetModelConstants(m_UIDamageTextMatrix, m_UIDamageTextColor);
			g_theRenderer->BindTexture(&g_font->GetTexture());
			g_theRenderer->DrawVertexArray(m_UIDamageTextVerts.size(), m_UIDamageTextVerts.data());
		}
	}
}

void Character::RenderDebug() const
{
	if (g_debugDraw)
	{
		std::vector<Vertex_PCU> debugVerts;
		AABB2 bound = AABB2(m_position, m_currentVisualHalfLength.y, m_currentVisualHalfLength.x);
		AddVertsForAABB2DOutline(debugVerts, bound, Rgba8::COLOR_GREEN, 0.2f);
		AddVertsForRing2D(debugVerts, m_position, m_currentHitboxRadius, 0.2f, Rgba8::COLOR_RED, 64);

		if (m_currentChunk)
		{
			int blockIndexBR = -1;
			Chunk* chunkBR = m_map->GetChunkAndBlockIndexFromGlobalPosition(blockIndexBR, m_position + Vec2(m_currentVisualHalfLength.x, -m_currentVisualHalfLength.y));
			if (chunkBR)
			{
				BlockIterator BR = BlockIterator(chunkBR, blockIndexBR);
				AABB2 BR_bound = AABB2(BR.GetWorldCenter(), 0.5f, 0.5f);
				AddVertsForAABB2DOutline(debugVerts, BR_bound, Rgba8::COLOR_BLUE, 0.2f);
			}

			int blockIndexBL = -1;
			Chunk* chunkBL = m_map->GetChunkAndBlockIndexFromGlobalPosition(blockIndexBL, m_position + Vec2(-m_currentVisualHalfLength.x, -m_currentVisualHalfLength.y));
			if (chunkBL)
			{
				BlockIterator BL = BlockIterator(chunkBL, blockIndexBL);
				AABB2 BL_bound = AABB2(BL.GetWorldCenter(), 0.5f, 0.5f);
				AddVertsForAABB2DOutline(debugVerts, BL_bound, Rgba8::COLOR_MAGNETA, 0.2f);
			}
		}

		g_theRenderer->SetModelConstants();
		g_theRenderer->BindTexture(nullptr);
		g_theRenderer->DrawVertexArray(debugVerts.size(), debugVerts.data());

	}
}

void Character::Die()
{
	m_isDead = true;
	for (auto& currentItem : m_currentItems)
	{
		currentItem->Die();
		delete currentItem;
		currentItem = nullptr;
	}
	m_map->m_deadCharacters.push_back(this);
}

void Character::GravityAndFloorCollision(float deltaSeconds)
{
	if (!m_isFlying)
	{
		m_position.y -= m_characterDef->m_gravitySpeed * deltaSeconds;
	}

	if (m_currentChunk)
	{
		bool isBlockUnderSolid = false;

		int blockIndexAtLeg = m_currentChunk->GetBlockIndexFromGlobalPosition(m_position - Vec2(0, m_currentVisualHalfLength.y - 1));
		BlockIterator iter = BlockIterator(m_currentChunk, blockIndexAtLeg);
		int numberToExtend = RoundDownToInt(m_currentVisualHalfLength.x) - 2;

		BlockIterator* left = new BlockIterator[numberToExtend];

		for (size_t i = 0; i < numberToExtend; i++)
		{
			if (i == 0)
			{
				left[i] = iter.GetLeftNeighbor();
			}
			else
			{
				left[i] = left[i - 1].GetLeftNeighbor();
			}
		}

		BlockIterator* right = new BlockIterator[numberToExtend + 1];

		for (size_t i = 0; i < numberToExtend + 1; i++)
		{
			if (i == 0)
			{
				right[i] = iter.GetRightNeighbor();
			}
			else
			{
				right[i] = right[i - 1].GetRightNeighbor();
			}
		}

		for (size_t i = 0; i < numberToExtend + 1; i++)
		{
			if (i < numberToExtend)
			{
				isBlockUnderSolid |= left[i].GetDownNeighbor().GetBlock() && left[i].GetDownNeighbor().GetBlock()->IsBlockSolid();
			}

			isBlockUnderSolid |= right[i].GetDownNeighbor().GetBlock() && right[i].GetDownNeighbor().GetBlock()->IsBlockSolid();
		}


		if (isBlockUnderSolid && !m_isFlying)
		{
			m_position.y += m_characterDef->m_gravitySpeed * deltaSeconds;
		}

		m_isGrounded = isBlockUnderSolid;
	}
}

void Character::MoveRight(float deltaSeconds)
{
	Vec2 movement = Vec2(MOVING_SPEED, 0) * deltaSeconds;
	if (g_debugCharacterMoveFast) movement *= 10.f;

	int blockIndex = -1;
	Chunk* chunk = m_map->GetChunkAndBlockIndexFromGlobalPosition(blockIndex, m_position + Vec2(m_currentVisualHalfLength.x, -m_currentVisualHalfLength.y));

	BlockIterator iter = BlockIterator(chunk, blockIndex);
	if (iter.GetBlock())
	{
		int terrainHeight = 0;

		if (iter.GetBlock()->IsBlockSolid())
		{
			while (iter.GetBlock()->IsBlockSolid())
			{
				terrainHeight++;
				iter = iter.GetUpNeighbor();
			}

			if (terrainHeight != 0 && terrainHeight < RoundDownToInt(m_currentVisualHalfLength.y * 1.5f))
			{
				if (m_stamina > 0)
				{
					m_position.y += 1.f;
				}
			}
			else
			{
				movement = Vec2::ZERO;
			}
		}
	}
	if (m_stamina > 0)
	{
		m_position += movement;
		if (!m_isSpendingStamina)
		{
			m_stamina -= MOVING_STAMINA_COST * deltaSeconds;
		}
	}


	PlayWalkingSound();
}

void Character::MoveLeft(float deltaSeconds)
{
	Vec2 movement = -Vec2(MOVING_SPEED, 0) * deltaSeconds;
	if (g_debugCharacterMoveFast) movement *= 10.f;

	int blockIndex = -1;
	Chunk* chunk = m_map->GetChunkAndBlockIndexFromGlobalPosition(blockIndex, m_position + Vec2(-m_currentVisualHalfLength.x, -m_currentVisualHalfLength.y));

	BlockIterator iter = BlockIterator(chunk, blockIndex);
	if (iter.GetBlock())
	{
		int terrainHeight = 0;

		if (iter.GetBlock()->IsBlockSolid())
		{
			while (iter.GetBlock()->IsBlockSolid())
			{
				terrainHeight++;
				iter = iter.GetUpNeighbor();
			}

			if (terrainHeight != 0 && terrainHeight < RoundDownToInt(m_currentVisualHalfLength.y * 1.5f))
			{
				if (m_stamina > 0)
				{
					m_position.y += 1.f;
				}
			}
			else
			{
				movement = Vec2::ZERO;
			}
		}
	}

	if (m_stamina > 0)
	{
		m_position += movement;
		if (!m_isSpendingStamina)
		{
			m_stamina -= MOVING_STAMINA_COST * deltaSeconds;
		}
	}

	PlayWalkingSound();
}

void Character::MoveUp(float deltaSeconds)
{
	if (!m_isFlying) return;

	Vec2 movement = Vec2(0, MOVING_SPEED) * deltaSeconds;
	if (m_position.y > 110.f) movement = Vec2::ZERO;
	if (g_debugCharacterMoveFast) movement *= 10.f;

	int blockIndexHead = m_currentChunk->GetBlockIndexFromGlobalPosition(m_position + Vec2(0, m_currentVisualHalfLength.y));
	BlockIterator iter = BlockIterator(m_currentChunk, blockIndexHead);
	if (iter.GetBlock() && !iter.GetBlock()->IsBlockSolid())
	{
		if (m_stamina > 0)
		{
			m_position += movement;
			if (!m_isSpendingStamina)
			{
				m_stamina -= MOVING_STAMINA_COST * deltaSeconds;
			}
		}
	}
}

void Character::MoveOnCurrentFacing(float deltaSeconds)
{
	if (m_isFacingLeft)
	{
		MoveLeft(deltaSeconds);
	}
	else
	{
		MoveRight(deltaSeconds);
	}
}

void Character::MoveDown(float deltaSeconds)
{
	if (!m_isFlying) return;

	Vec2 movement = -Vec2(0, MOVING_SPEED) * deltaSeconds;
	if (g_debugCharacterMoveFast) movement *= 10.f;

	int blockIndexHead = m_currentChunk->GetBlockIndexFromGlobalPosition(m_position - Vec2(0, m_currentVisualHalfLength.y));
	BlockIterator iter = BlockIterator(m_currentChunk, blockIndexHead);
	if (iter.GetBlock() && !iter.GetBlock()->IsBlockSolid())
	{
		if (m_stamina > 0)
		{
			m_position += movement;
			if (!m_isSpendingStamina)
			{
				m_stamina -= MOVING_STAMINA_COST * deltaSeconds;
			}
		}
	}
}

void Character::FaceRight()
{
	if (m_isFacingLeft)
	{
		m_currentAngle = 180 - m_currentAngle;
		FloatRange oldLimit = m_currentAngleLimit;
		m_currentAngleLimit.m_min = 180 - oldLimit.m_max;
		m_currentAngleLimit.m_max = 180 - oldLimit.m_min;
	}
	m_isFacingLeft = false;
}

void Character::FaceLeft()
{
	if (!m_isFacingLeft)
	{
		m_currentAngle = 180 - m_currentAngle;
		FloatRange oldLimit = m_currentAngleLimit;
		m_currentAngleLimit.m_min = 180 - oldLimit.m_max;
		m_currentAngleLimit.m_max = 180 - oldLimit.m_min;
	}
	m_isFacingLeft = true;
}

void Character::FaceTheOpposite()
{
	if (m_isFacingLeft)
	{
		FaceRight();
	}
	else
	{
		FaceLeft();
	}
}

void Character::AdjustAngle(float UpOrDown, float deltaSeconds)
{
	if (m_isFacingLeft) UpOrDown *= -1;
	m_currentAngle += UpOrDown * ADJUST_ANGLE_SPEED * deltaSeconds;
	m_currentAngle = Clamp(m_currentAngle, m_currentAngleLimit.m_min, m_currentAngleLimit.m_max);
}

void Character::AdjustForce(float deltaSeconds)
{
	if (m_currentForce < m_currentForceLimit.m_min || m_currentForce > m_currentForceLimit.m_max)
	{
		m_forceDirection *= -1;
	}

	m_currentForce += m_forceDirection * ADJUST_FORCE_SPEED * deltaSeconds;
}

void Character::BeginTurn()
{
	m_currentProjectile = m_characterDef->m_projectiles[0];
	m_skill1CDCounter--;
	m_skill2CDCounter--;
	m_isInvisible = false;
}

void Character::EndTurn()
{
	m_isPendingInvisible = false;

	m_stamina = m_characterDef->m_initialStamina + RoundDownToInt(m_stamina * 0.2f);

	m_attack = m_characterDef->m_initialAttack;
	m_speed = m_characterDef->m_initialSpeed;
	m_armor = m_characterDef->m_initialArmor;
	m_attack = m_characterDef->m_initialAttack;

	m_currentForceLimit = m_characterDef->m_forceLimit;
	m_currentForce = m_currentForceLimit.m_min;

	FixAngleLimitToTheCorrectDirection();

	m_canUseSKill1 = true;
	m_canUseSKill2 = true;
}

bool Character::ConditionsForSkill_1()
{
	if (m_stamina < m_characterDef->m_skill1Cost || m_skill1CDCounter > 0)
	{
		m_canUseSKill1 = false;
	}
	if (!m_canUseSKill1) return false;
	if (m_controller->m_isChargingForShoot) return false;

	return true;
}

bool Character::ConditionsForSkill_2()
{
	if (m_stamina < m_characterDef->m_skill2Cost || m_skill2CDCounter > 0)
	{
		m_canUseSKill2 = false;
	}
	if (!m_canUseSKill2) return false;
	if (m_controller->m_isChargingForShoot) return false;

	return true;
}

bool Character::ConditionsForUltimate()
{
	if (m_fury < 100) return false;

	return true;
}

void Character::Initialize()
{
	InitializeEvent();
	InitializeVisual();
	InitializeStats();
}

bool Character::TakeDamage(int damage, Character* characterDealedDamage, bool isCrit)
{
	if (m_health == -1) return true;

	m_isInvisible = false;

	if (damage > 0 && GetItem("Hulk_S2"))
	{
		RemoveItem(GetItem("Hulk_S2"));
		m_map->m_game->m_audioManager->PlaySFXSound("Hulk_Shield");
		return true;
	}

	int realDamage = damage - m_armor;

	if (realDamage < 0) realDamage = 0;

	if (!m_hasGainedFuryByTakingDamageThisTurn && characterDealedDamage != this)
	{
		m_hasGainedFuryByTakingDamageThisTurn = true;
		m_fury += (BASED_FURY_GAIN_TAKING_DAMAGE + (int)((realDamage / m_health) * 100.f * 0.5f));
	}

	int afterBreakDamage = 0;
	int shieldDamage = 0;
	if (m_shield > 0)
	{
		afterBreakDamage = realDamage - m_shield;

		if (afterBreakDamage >= 0)
		{
			realDamage = afterBreakDamage;
			m_shield = 0;
		}
		else
		{
			m_shield -= damage;
			shieldDamage = damage;
			realDamage = 0;
		}
	}

	PlayHPChangeUIEffect(realDamage + shieldDamage, isCrit);

	m_health -= realDamage;
	m_health = Clamp(m_health, 0, m_characterDef->m_initialHealth);

	m_gotHit = true;

	if (m_health == 0)
	{
		if (characterDealedDamage)
		{
			characterDealedDamage->m_killCount++;
		}
		Die();
		return false;
	}
	else
	{
		Emitter* emitter = new Emitter(m_map->m_game->m_vfxSystem, m_position, 0.6f);
		emitter->SetEmitDirection(Vec2(0, 1));
		emitter->SetParticleRandomOffsetDirAngle(FloatRange(-50, 50));
		emitter->SetParticleRandomOrientation(FloatRange(0.f, 359.f));
		emitter->SetNumParticleEachEmit(5);
		emitter->SetParticleLifeRandomInRange(FloatRange(1.0f, 1.6f));
		emitter->SetTimeBetweenEachEmit(0.2f);
		emitter->SetEmitSpeed(g_theRNG->RollRandomFloatInRange(15.f, 22.f));
		emitter->SetParticleDefs({ 24, 25, 26 });
	}

	return true;
}

void Character::Heal(int heal, bool overHeal)
{
	if (m_health == m_characterDef->m_initialHealth) return;

	m_map->m_game->m_audioManager->PlaySFXSound("Heal");

	int afterHeal = m_health + heal;
	if (overHeal)
	{
		if (afterHeal > m_characterDef->m_initialHealth)
		{
			m_shield = afterHeal - m_characterDef->m_initialHealth;
		}
	}

	m_health = Clamp(afterHeal, 0, m_characterDef->m_initialHealth);

	Emitter* emitter = new Emitter(m_map->m_game->m_vfxSystem, m_position - Vec2(0, m_currentVisualHalfLength.y - 1), 0.6f);
	emitter->SetEmitDirection(Vec2(0, 1));
	emitter->SetParticleRandomOffsetPosition(FloatRange(-4, 4), FloatRange(-3, 1));
	emitter->SetNumParticleEachEmit(3);
	emitter->SetParticleLifeRandomInRange(FloatRange(0.7f, 1.2f));
	emitter->SetTimeBetweenEachEmit(0.2f);
	emitter->SetEmitSpeed(g_theRNG->RollRandomFloatInRange(15.f, 22.f));
	emitter->SetParticleDefs({ 27 });

	PlayHPChangeUIEffect(heal, overHeal, true);
}

int Character::CalculateDamage(bool& out_isCrit, float bonusCritRate, float bonusCritDamage)
{
	int damage = m_attack;
	out_isCrit = g_theRNG->RollRandomChance(m_critRateFromZeroToOne + bonusCritRate);
	if (out_isCrit)
	{
		damage = (int)(damage * (m_critDamageMultiplier + bonusCritDamage));
	}

	return damage;
}

Item* Character::GetItem(std::string name)
{
	for (auto& currentItem : m_currentItems)
	{
		if (currentItem->m_itemDef->m_name == name)
		{
			return currentItem;
		}
	}

	return nullptr;
}

void Character::RemoveItem(Item* item)
{
	if (!item) return;
	auto found = std::find(m_currentItems.begin(), m_currentItems.end(), item);
	if (found != m_currentItems.end())
	{
		m_currentItems.erase(found);
	}
}

Character* Character::GetNearestEnemy() const
{
	float nearestDistance = FLT_MAX;
	Character* nearestCharacter = nullptr;
	for (auto& character : m_map->m_characters)
	{
		float distance = (m_position - character->m_position).GetLengthSquared();
		if (GetTeamID() != character->GetTeamID() && distance < nearestDistance)
		{
			nearestDistance = distance;
			nearestCharacter = character;
		}
	}

	return nearestCharacter;
}

Character* Character::GetNearestAlly() const
{
	float nearestDistance = FLT_MAX;
	Character* nearestCharacter = nullptr;
	for (auto& character : m_map->m_characters)
	{
		float distance = (m_position - character->m_position).GetLengthSquared();
		if (GetTeamID() == character->GetTeamID() && distance < nearestDistance)
		{
			nearestDistance = distance;
			nearestCharacter = character;
		}
	}

	return nearestCharacter;
}

int Character::GetID() const
{
	return m_controller->m_ID;
}

int Character::GetTeamID() const
{
	return m_controller->m_teamID;
}

void Character::ReceiveItem(Item* item)
{
	m_currentItems.push_back(item);
}

Projectile* Character::ShootCurrentProjectile(float angle, float force)
{
	m_lastForce = force;

	Projectile* NA_projectile = new Projectile(m_map, this, m_currentProjectile);
	NA_projectile->m_position = m_position;
	float adjustedForce = (m_characterDef->m_isMelee) ? 80.f : force * 1.5f;
	Vec2 velocity = Vec2::MakeFromPolarDegrees(angle, adjustedForce);
	NA_projectile->m_velocity = velocity;

	if (m_isFacingLeft && !m_characterDef->m_isMelee)
	{
		NA_projectile->m_orientationDegrees += (90 - (180 - angle));
	}
	else
	{
		NA_projectile->m_orientationDegrees -= (90 - angle);
	}

	m_map->m_flyingProjectiles.push_back(NA_projectile);

	m_map->m_game->m_audioManager->PlaySFXSound(m_characterDef->m_throwSound);

	return NA_projectile;
}

void Character::NormalAttack()
{
	m_currentProjectile = m_characterDef->m_projectiles[0];

	ShootCurrentProjectile(m_currentAngle, m_currentForce);

	m_currentForce = m_characterDef->m_forceLimit.m_min;

	m_fury += FURY_PER_ATTACK;

	m_map->m_game->PendingEndTurn();
}

void Character::Skill_1()
{
	m_skill1CDCounter = m_characterDef->m_skill1Cooldown;
	m_stamina -= m_characterDef->m_skill1Cost;
	m_map->m_game->m_audioManager->PlaySFXSound(m_characterDef->m_S1Sound);
}

void Character::Skill_2()
{
	m_skill2CDCounter = m_characterDef->m_skill2Cooldown;
	m_stamina -= m_characterDef->m_skill2Cost;
	m_map->m_game->m_audioManager->PlaySFXSound(m_characterDef->m_S2Sound);
}

void Character::Ultimate()
{
	m_fury = 0;
	m_map->m_game->m_audioManager->PlaySFXSound(m_characterDef->m_UltSound);
}

void Character::InitializeEvent()
{
	g_theEventSystem->SubscribeEventCallbackMemberFunction(m_characterDef->m_name, (Character*)this, &Character::Command_ModifyStats);
}

void Character::InitializeVisual()
{
	AABB2 bound = AABB2(Vec2::ZERO, m_characterDef->m_visualHalfLength.y, m_characterDef->m_visualHalfLength.x);
	AddVertsForAABB2D(m_verts, bound, Rgba8::COLOR_WHITE);
	m_vbuffer = g_theRenderer->CreateVertexBuffer(sizeof(Vertex_PCU) * (int)m_verts.size());
	g_theRenderer->CopyCPUToGPU(m_verts.data(), (int)(m_verts.size() * sizeof(Vertex_PCU)), m_vbuffer);

	m_sprite = g_theRenderer->CreateOrGetTextureFromFile(m_characterDef->m_characterTextureName.c_str());
}

void Character::InitializeStats()
{
	m_health = m_characterDef->m_initialHealth;
	m_speed = m_characterDef->m_initialSpeed;
	m_stamina = m_characterDef->m_initialStamina;
	m_armor = m_characterDef->m_initialArmor;
	m_attack = m_characterDef->m_initialAttack;
	m_currentAngleLimit = m_characterDef->m_angleLimit;
	m_currentForceLimit = m_characterDef->m_forceLimit;
	m_currentHitboxRadius = m_characterDef->m_hitboxRadius;
	m_currentVisualHalfLength = m_characterDef->m_visualHalfLength;
	m_skill1CDCounter = 0;
	m_skill2CDCounter = 0;

	m_currentAngle = m_characterDef->m_angleLimit.m_min + (m_characterDef->m_angleLimit.m_max - m_characterDef->m_angleLimit.m_min) * 0.5f;
	m_currentForce = m_characterDef->m_forceLimit.m_min;

	if (!m_characterDef->m_projectiles.empty())
	{
		m_currentProjectile = m_characterDef->m_projectiles[0];
	}
}

void Character::FixAngleLimitToTheCorrectDirection()
{
	if (m_isFacingLeft)
	{
		m_currentAngleLimit = FloatRange(180.f - m_characterDef->m_angleLimit.m_max, 180.f - m_characterDef->m_angleLimit.m_min);
	}
	else
	{
		m_currentAngleLimit = m_characterDef->m_angleLimit;
	}

	m_currentAngle = Clamp(m_currentAngle, m_currentAngleLimit.m_min, m_currentAngleLimit.m_max);
}

bool Character::IsCurrentHealthPercentageLowerThan(float zeroToOne)
{
	return (m_health / m_characterDef->m_initialHealth) < zeroToOne;
}

bool Character::IsPositionBehind(Vec2 position)
{
	Vec2 currentFaceDir = (m_isFacingLeft) ? Vec2(-1, 0) : Vec2(1, 0);
	
	Plane2 characterPlane = Plane2(currentFaceDir, m_position.GetLength());

	return characterPlane.GetAltitudeOfPoint(position) <= 0.f;
}

bool Character::FaceToTarget(Vec2 target)
{
	Vec2 toPos = (m_position - target).GetNormalized();
	Vec2 currentFaceDir = (m_isFacingLeft) ? Vec2(-1, 0) : Vec2(1, 0);

	float d = currentFaceDir.Dot(toPos);

	if (d > 0)
	{
		FaceTheOpposite();
		return true;
	}
	return false;
}

void Character::PlayWalkingSound()
{
	if (m_stamina <= 0) return;
	if (m_isFlying) return;
	if (m_currentChunk)
	{
		int blockIndexAtAnkle = m_currentChunk->GetBlockIndexFromGlobalPosition(m_position - Vec2(0, m_currentVisualHalfLength.y));
		int ankleBlockType = m_currentChunk->m_blocks[blockIndexAtAnkle].m_type;

		if (ankleBlockType == 8)
		{
			if (!g_theAudio->IsPlaying(m_currentWalkingWaterSound))
			{
				g_theAudio->StopSound(m_currentWalkingGroundSound);
				m_map->m_game->m_audioManager->PlayAndGetSFXSound_2("Walk_Water", m_currentWalkingWaterSound, 5);
			}
		}

		int blockIndexAtLeg = m_currentChunk->GetBlockIndexFromGlobalPosition(m_position - Vec2(0, m_currentVisualHalfLength.y + 1));
		int legBlockType = m_currentChunk->m_blocks[blockIndexAtLeg].m_type;

		GroundSound soundType = GroundSound::AIR;
		if (legBlockType == 0)
		{
			return;
		}
		else if (legBlockType == 1)
		{
			soundType = GroundSound::GRASS;
		}
		else if (legBlockType == 9)
		{
			soundType = GroundSound::SNOW;
		}
		else if (legBlockType == 2 || legBlockType == 10)
		{
			soundType = GroundSound::SAND;
		}
		else if (legBlockType == 11 || legBlockType == 12)
		{
			soundType = GroundSound::ICE;
		}
		else
		{
			soundType = GroundSound::STONE;
		}

		if (g_theAudio->IsPlaying(m_currentWalkingGroundSound) && m_currentGroundType == soundType) return;

		switch (soundType)
		{
		case GroundSound::GRASS:
			m_currentWalkingGroundSound = m_map->m_game->m_audioManager->PlayAndGetSFXSound("Walk_Grass", 2);
			break;
		case GroundSound::SNOW:
			m_currentWalkingGroundSound = m_map->m_game->m_audioManager->PlayAndGetSFXSound("Walk_Snow");
			break;
		case GroundSound::SAND:
			m_currentWalkingGroundSound = m_map->m_game->m_audioManager->PlayAndGetSFXSound("Walk_Sand");
			break;
		case GroundSound::ICE:
			m_currentWalkingGroundSound = m_map->m_game->m_audioManager->PlayAndGetSFXSound("Walk_Ice");
			break;
		case GroundSound::STONE:
			m_currentWalkingGroundSound = m_map->m_game->m_audioManager->PlayAndGetSFXSound("Walk_Stone");
			break;
		}

		m_currentGroundType = soundType;
	}
}

bool Character::Command_ModifyStats(EventArgs& args)
{
	if (args.IsKeyNameValid("hp"))
	{
		std::string result = args.GetValue<std::string>("hp", "1.0");
		m_health = atoi(result.c_str());
		if (m_health <= 0.f)
		{
			Die();

			if (m_map->m_game->m_playerIDTurn == GetID())
			{
				m_map->m_game->SetCooldownCamera(1.f);
				m_map->m_game->NextTurn();
			}
		}
		g_theDevConsole->AddLine(DevConsole::SUCCESS, m_characterDef->m_name + "'s HP is set to: " + result);
	}
	if (args.IsKeyNameValid("atk"))
	{
		std::string result = args.GetValue<std::string>("atk", "1.0");
		m_attack = atoi(result.c_str());
		g_theDevConsole->AddLine(DevConsole::SUCCESS, m_characterDef->m_name + "'s ATK is set to: " + result);
	}
	if (args.IsKeyNameValid("s"))
	{
		std::string result = args.GetValue<std::string>("s", "1.0");
		m_stamina = (float)atof(result.c_str());
		g_theDevConsole->AddLine(DevConsole::SUCCESS, m_characterDef->m_name + "'s Stamina is set to: " + result);
	}
	if (args.IsKeyNameValid("f"))
	{
		std::string result = args.GetValue<std::string>("f", "1.0");
		m_fury = atoi(result.c_str());
		g_theDevConsole->AddLine(DevConsole::SUCCESS, m_characterDef->m_name + "'s Fury is set to: " + result);
	}
	if (args.IsKeyNameValid("fly"))
	{
		std::string result = args.GetValue<std::string>("f", "true");
		if (result == " true")
		{
			m_isFlying = true;
		}
		else
		{
			m_isFlying = false;
		}
		g_theDevConsole->AddLine(DevConsole::SUCCESS, m_characterDef->m_name + "is flying: " + result);
	}
	return false;
}

void Character::PlayHPChangeUIEffect(int value, bool isCrit, bool isHeal)
{
	m_UIDamageTextVerts.clear();

	g_font->AddVertsForText2D(m_UIDamageTextVerts, Vec2(-2.5f, 0.f), 2.5f, Stringf("%i", value), Rgba8::COLOR_WHITE, 1.f, 0.f, 0.5f);

	m_UIDamageTextMatrix = Mat44();
	m_UIDamageTextMatrix.SetTranslation2D(m_position);

	if (isHeal)
	{
		if (isCrit)
		{
			m_UIDamageTextMatrix.AppendScaleUniform2D(1.5f);
		}

		m_UIDamageTextColor = Rgba8::COLOR_GREEN;
	}
	else
	{
		if (isCrit)
		{
			m_UIDamageTextMatrix.AppendScaleUniform2D(1.5f);
			m_UIDamageTextColor = Rgba8::COLOR_ORANGE;
		}
		else
		{
			m_UIDamageTextColor = Rgba8::COLOR_WHITE;
		}
	}

	m_UIDamageTextTimer->Start();
}
