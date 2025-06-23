#include "Game/CharacterCommon.hpp"
#include "Game/GameCommon.hpp"
#include "Game/AllCharacters.hpp"
#include "Game/AIController.hpp"



std::vector<ProjectileDefinition*> ProjectileDefinition::s_projectileDefList;

ProjectileDefinition::ProjectileDefinition(XmlElement& element)
	:m_name(ParseXmlAttribute(element, "name", "")),
	m_isMelee(ParseXmlAttribute(element, "isMelee", false)),
	m_meleeMaxRange(ParseXmlAttribute(element, "meleeMaxRange", 0.f)),
	m_damageScale(ParseXmlAttribute(element, "damageScale", 1.0f)),
	m_isTeleport(ParseXmlAttribute(element, "isTeleport", false)),
	m_isHealing(ParseXmlAttribute(element, "isHealing", false)),
	m_canOverHeal(ParseXmlAttribute(element, "canOverHeal", false)),
	m_healingValue(ParseXmlAttribute(element, "healingValue", 0))
{

}

void ProjectileDefinition::SetCollision(XmlElement& element)
{
	m_hitRadius = ParseXmlAttribute(element, "hitRadius", 0.f);
	m_destructionRadius = ParseXmlAttribute(element, "destructionRadius", 0.f);
}

void ProjectileDefinition::SetHoming(XmlElement& element)
{
	m_isHoming = true;
	m_homingRange = ParseXmlAttribute(element, "homingRange", 0.f);
	m_homingSpeed = ParseXmlAttribute(element, "homingSpeed", 0.f);
}

void ProjectileDefinition::SetVisual(XmlElement& element)
{
	m_visualHalfLength = ParseXmlAttribute(element, "halfLength", Vec2());
	m_projectileTextureName = ParseXmlAttribute(element, "image", "");
	m_offsetAngle = ParseXmlAttribute(element, "offsetAngle", 0.f);
}

void ProjectileDefinition::SetSound(XmlElement& element)
{
	m_soundName = ParseXmlAttribute(element, "name", "");
}

void ProjectileDefinition::InitializeDefs(char const* filePath)
{
	XmlDocument file;
	XmlError result = file.LoadFile(filePath);
	GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS, "FILE IS NOT LOADED");

	XmlElement* rootElement = file.RootElement();
	GUARANTEE_OR_DIE(rootElement, "Root Element is null");

	XmlElement* projectileDefElement = rootElement->FirstChildElement();

	while (projectileDefElement)
	{
		std::string name = projectileDefElement->Name();
		GUARANTEE_OR_DIE(name == "ProjectileDefinition", "Root child element is in the wrong format");

		ProjectileDefinition* newProjectileDef = new ProjectileDefinition(*projectileDefElement);

		XmlElement* actorChildElement = projectileDefElement->FirstChildElement();
		while (actorChildElement)
		{
			std::string sectionName = actorChildElement->Name();
			if (sectionName == "Collision")
			{
				newProjectileDef->SetCollision(*actorChildElement);
			}
			if (sectionName == "Homing")
			{
				newProjectileDef->SetHoming(*actorChildElement);
			}
			if (sectionName == "Visuals")
			{
				newProjectileDef->SetVisual(*actorChildElement);
			}
			if (sectionName == "Sounds")
			{
				newProjectileDef->SetSound(*actorChildElement);
			}
			actorChildElement = actorChildElement->NextSiblingElement();
		}
		s_projectileDefList.push_back(newProjectileDef);
		projectileDefElement = projectileDefElement->NextSiblingElement();
	}
}

void ProjectileDefinition::ClearDefinition()
{
	for (auto& i : s_projectileDefList)
	{
		if (i != nullptr)
		{
			delete i;
			i = nullptr;
		}
	}
}

ProjectileDefinition* ProjectileDefinition::GetByName(std::string name)
{
	for (auto& i : s_projectileDefList)
	{
		if (i->m_name == name)
		{
			return i;
		}
	}
	return nullptr;
}


Projectile::Projectile(Map* map, Character* owner, ProjectileDefinition* def)
	:Entity(map), m_projectileDef(def), m_owner(owner)
{
	AABB2 bound = AABB2(Vec2::ZERO, m_projectileDef->m_visualHalfLength.y, m_projectileDef->m_visualHalfLength.x);
	AddVertsForAABB2D(m_verts, bound, Rgba8::COLOR_WHITE);
	m_vbuffer = g_theRenderer->CreateVertexBuffer(sizeof(Vertex_PCU) * (int)m_verts.size());
	g_theRenderer->CopyCPUToGPU(m_verts.data(), (int)(m_verts.size() * sizeof(Vertex_PCU)), m_vbuffer);

	m_sprite = g_theRenderer->CreateOrGetTextureFromFile(m_projectileDef->m_projectileTextureName.c_str());

	m_orientationDegrees = m_projectileDef->m_offsetAngle;

	m_trailEmitter = new Emitter(m_map->m_game->m_vfxSystem, m_position, 10.f);
	m_trailEmitter->SetTimeBetweenEachEmit(0.1f);
	m_trailEmitter->SetParticleRandomOrientation(FloatRange(0.f, 359.f));
	m_trailEmitter->SetNumParticleEachEmit(1);
	m_trailEmitter->SetParticleLifeRandomInRange(FloatRange(0.3f, 0.5f));
	m_trailEmitter->SetParticleDefs(m_owner->m_vfxIndex);
}

Projectile::~Projectile()
{
	m_trailEmitter->m_lifeTime = -1.f;
}

void Projectile::Update(float deltaSeconds)
{
	if (m_deadTimer && m_deadTimer->HasPeriodElapsed())
	{
		Die();
		return;
	}

	if (m_isStatic) return;

	bool isDirectHit = false;
	bool affectEnemy = false;

	Movement_Update(deltaSeconds);

	Emitter_Update(deltaSeconds);

	isDirectHit = DirectHitCheck();

	affectEnemy = ImpactHitCheck(isDirectHit);

	if (isDirectHit || affectEnemy)
	{
		m_trailEmitter->m_lifeTime = -1;
	}

	if (!m_owner) return;

	// UPDATE AI LOGIC
	AIController* AI = dynamic_cast<AIController*>(m_owner->m_controller);
	if (AI)
	{
		if (isDirectHit || affectEnemy)
		{
			if (!m_owner->m_controller->IsPlayer())
			{
				AI->m_hasHitDirectly = isDirectHit;
				AI->m_hasHitExplosion = affectEnemy;
			}
		}
		if (m_characterGotHit)
		{
			AI->m_currentFocusTarget = m_characterGotHit;
		}
		AI->m_lastHitPosition = m_hitPosition;
	}


	// SPECIAL ABILTY
	auto helaPtr = dynamic_cast<Hela_Character*>(m_owner);
	if (helaPtr)
	{
		if (isDirectHit || affectEnemy)
		{
			helaPtr->m_damageStack++;
			if (isDirectHit)
			{
				helaPtr->m_fury += HELA_FURY_PER_ATTACK_HIT;
			}
		}
	}
	auto ironPtr = dynamic_cast<Iron_Character*>(m_owner);
	if (ironPtr)
	{
		if (isDirectHit || affectEnemy)
		{
			ironPtr->m_bonusStamina = 50;
		}
	}

	// SOUND
	PlaySoundOnImpact();
}

void Projectile::Render() const
{
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetDepthStencilMode(DepthMode::ENABLED);
	g_theRenderer->SetSamplerMode(SampleMode::POINT_CLAMP);
	if (m_deadTimer)
	{
		auto a = (unsigned char)((1.f - m_deadTimer->GetElapsedFraction()) * 255.f);
		g_theRenderer->SetModelConstants(GetModelMatrix(), Rgba8(255, 255, 255, a));
	}
	else
	{
		g_theRenderer->SetModelConstants(GetModelMatrix(), m_tintColor);
	}
	g_theRenderer->BindTexture(m_sprite);
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->DrawVertexBuffer(m_vbuffer, m_verts.size());

	if (g_debugDraw)
	{
		std::vector<Vertex_PCU> debugVerts;
		AABB2 bound = AABB2(m_position, m_projectileDef->m_visualHalfLength.y, m_projectileDef->m_visualHalfLength.x);
		AddVertsForAABB2DOutline(debugVerts, bound, Rgba8::COLOR_GREEN, 0.2f);
		AddVertsForRing2D(debugVerts, m_position, m_projectileDef->m_hitRadius, 0.2f, Rgba8::COLOR_RED, 64);
		AddVertsForRing2D(debugVerts, m_position, m_projectileDef->m_destructionRadius, 0.2f, Rgba8::COLOR_CYAN, 64);

		if (m_reachedSuperHigh)
		{
			AddVertsForArrow2D(debugVerts, m_position, m_position + m_velocity, 0.2f, 0.5f, Rgba8::COLOR_MAGNETA);
		}
		else
		{
			AddVertsForArrow2D(debugVerts, m_position, m_position + m_velocity, 0.2f, 0.5f, Rgba8::COLOR_YELLOW);
		}

		if (m_projectileDef->m_isHoming)
		{
			AddVertsForRing2D(debugVerts, m_position, m_projectileDef->m_homingRange, 0.2f, Rgba8::COLOR_ORANGE, 64);
		}


		g_theRenderer->SetModelConstants();
		g_theRenderer->BindTexture(nullptr);
		g_theRenderer->DrawVertexArray(debugVerts.size(), debugVerts.data());
	}
}

void Projectile::Die()
{
	m_isDead = true;
}

void Projectile::Emitter_Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	m_trailEmitter->m_position = m_position;
	Vec2 dir = m_velocity.GetNormalized();
	float speed = m_velocity.GetLength() * 0.5f;
	dir.RotateDegrees(g_theRNG->RollRandomFloatInRange(-25.f, 25.f));
	dir.Normalize();
	m_trailEmitter->SetEmitDirection(-dir);
	m_trailEmitter->SetEmitSpeed(speed);
}

bool Projectile::IsEnemy(Character* character) const
{
	if (!m_owner) return false;
	return m_owner->m_controller->m_teamID != character->m_controller->m_teamID;
}

void Projectile::BeginTurn()
{
	if (m_turnAliveLeft < 0) return;
}

Character* Projectile::FindNearestEnemy(float range)
{
	float nearestDistance = FLT_MAX;
	Character* nearestCharacter = nullptr;
	for (auto& character : m_map->m_characters)
	{
		float distance = (m_position - character->m_position).GetLength();
		if (distance > range)
		{
			continue;
		}
		if (m_owner->m_controller && character->m_controller)
		{
			if (IsEnemy(character) && distance < nearestDistance)
			{
				nearestDistance = distance;
				nearestCharacter = character;
			}
		}
	}

	return nearestCharacter;
}

Character* Projectile::FindNearestAlly(float range)
{
	float nearestDistance = FLT_MAX;
	Character* nearestCharacter = nullptr;
	for (auto& character : m_map->m_characters)
	{
		float distance = (m_position - character->m_position).GetLength();
		if (distance > range)
		{
			continue;
		}
		if (m_owner->m_controller->m_teamID == character->m_controller->m_teamID && distance < nearestDistance)
		{
			nearestDistance = distance;
			nearestCharacter = character;
		}
	}

	return nearestCharacter;
}

void Projectile::Movement_Update(float deltaSeconds)
{
	if (m_projectileDef->m_isMelee)
	{
		if (m_distanceTravelled > m_projectileDef->m_meleeMaxRange)
		{
			m_isHit = true;
			return;
		}

	}
	else
	{
		m_velocity.y += GRAVITY_FORCE_PROJECTILE * deltaSeconds;
	}

	if (m_projectileDef->m_isHoming)
	{
		m_homingTarget = FindNearestEnemy(m_projectileDef->m_homingRange);
		if (m_homingTarget)	m_velocity += (m_homingTarget->m_position - m_position).GetNormalized() * m_projectileDef->m_homingSpeed * deltaSeconds;
	}

	Vec2 newPos = m_position + m_velocity * deltaSeconds;
	m_distanceTravelled += (newPos - m_position).GetLength();
	m_position = newPos;

	float goalAngle = m_velocity.GetOrientationDegrees() - m_projectileDef->m_offsetAngle;
	m_orientationDegrees = GetTurnedTowardDegrees(m_orientationDegrees, goalAngle, 90 * deltaSeconds);

	if (m_position.y >= (float)CHUNK_SIZE_Y && !m_reachedSuperHigh)
	{
		m_reachedSuperHigh = true;
	}
}

bool Projectile::DirectHitCheck()
{
	bool changeTerrain = !m_projectileDef->m_isTeleport && !m_projectileDef->m_isHealing;

	bool isDirectHit = false;

	if (!m_projectileDef->m_isTeleport)
	{
		for (auto& character : m_map->m_characters)
		{
			if (!character || character->m_health == -1) continue;

			if (IsEnemy(character))
			{
				if (DoDiscsOverlap2D(m_position, m_projectileDef->m_hitRadius, character->m_position, character->m_currentHitboxRadius))
				{
					float bonusCritRate = (m_reachedSuperHigh) ? 0.5f : 0.f;
					float bonusCritDamage = (m_reachedSuperHigh) ? 0.2f : 0.f;
					bool isCrit = false;
					int damage = int(m_owner->CalculateDamage(isCrit, bonusCritRate, bonusCritDamage) * m_projectileDef->m_damageScale);

					m_hitPosition = character->m_position;
					bool isAlive = character->TakeDamage(damage, m_owner, isCrit);
					m_characterGotHit = (isAlive) ? character : nullptr;
					isDirectHit = true;

					m_map->ExplodeAtPosition(m_position, m_projectileDef->m_destructionRadius, changeTerrain, m_owner->m_affectedTileSet);
				}
			}

			if (m_projectileDef->m_isHealing)
			{
				if (character != m_owner && character->m_health != -1 && character->GetTeamID() == m_owner->GetTeamID())
				{
					if (DoDiscsOverlap2D(m_position, m_projectileDef->m_hitRadius, character->m_position, character->m_currentHitboxRadius))
					{
						character->Heal(m_projectileDef->m_healingValue, m_projectileDef->m_canOverHeal);
						isDirectHit = true;
					}
				}
			}
		}
	}

	if (isDirectHit)
	{
		m_map->m_game->SetCooldownCamera(2.f);
		m_isHit = true;
	}

	return isDirectHit;
}

bool Projectile::ImpactHitCheck(bool isDirectHit)
{
	bool changeTerrain = !m_projectileDef->m_isTeleport && !m_projectileDef->m_isHealing;

	bool isMiss = false;
	if (m_position.y > (float)CHUNK_SIZE_Y) isMiss = true;
	if (m_position.x > m_map->m_bound.m_maxs.x || m_position.x < m_map->m_bound.m_mins.x)isMiss = true;

	bool affectEnemy = false;

	// CHECK IF HIT THE GROUND
	if (!isMiss)
	{
		m_currentChunkCoord = RoundDownToInt(m_position.x / (float)CHUNK_SIZE_X);
		if (m_currentChunkCoord < 0)
		{
			ERROR_AND_DIE("SOMETHING WITH PROJECTILE WRONG");
		}
		Chunk* currentChunk = m_map->m_activeChunks[m_currentChunkCoord];
		int blockIndex = currentChunk->GetBlockIndexFromGlobalPosition(m_position);

		if (currentChunk->m_blocks[blockIndex].IsBlockSolid())
		{
			m_hasTouchedGround = true;

			if (m_projectileDef->m_isTeleport)
			{
				BlockIterator iter = BlockIterator(currentChunk, blockIndex);
				while (iter.GetBlock() && iter.GetBlock()->IsBlockSolid())
				{
					iter = iter.GetUpNeighbor();
				}

				m_owner->m_position = iter.GetWorldCenter() + Vec2(0, m_owner->m_currentVisualHalfLength.y);
			}
			else
			{
				m_hitPosition = BlockIterator(currentChunk, blockIndex).GetWorldCenter();
			}
			if (!isDirectHit)
			{
				if (m_projectileDef->m_isHealing)
				{
					for (auto& character : m_map->m_characters)
					{
						if (character->m_health == -1 || character->GetTeamID() != m_owner->GetTeamID()) continue;

						if (DoDiscsOverlap2D(m_position, m_projectileDef->m_hitRadius, character->m_position, character->m_currentHitboxRadius))
						{
							character->PlayHPChangeUIEffect(m_projectileDef->m_healingValue, m_projectileDef->m_canOverHeal, true);
							character->Heal(m_projectileDef->m_healingValue, m_projectileDef->m_canOverHeal);
						}
					}
				}

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
						}
						else
						{
							if (m_projectileDef->m_isHealing) // NO IMPACT DAMAGE TO TEAMMATE IF IT'S HEALING
							{
								continue;
							}

							damage /= 3;
						}
						bool isAlive = character->TakeDamage(damage, m_owner, isCrit);
						if (IsEnemy(character))
						{
							m_characterGotHit = (isAlive) ? character : nullptr;
						}
					}
				}
			}
			
			if (m_owner)
			{
				m_map->ExplodeAtPosition(m_position, m_projectileDef->m_destructionRadius, changeTerrain, m_owner->m_affectedTileSet);
				PlayExplodeEffect();
			}

			m_isHit = true;
		}
	}
	else
	{
		m_map->m_game->SetCooldownCamera(1.f);
	}

	return affectEnemy;
}

void Projectile::PlayExplodeEffect()
{
	Emitter* emitter = new Emitter(m_map->m_game->m_vfxSystem, m_hitPosition, 0.2f);
	emitter->SetEmitDirectionRandom();
	emitter->SetNumParticleEachEmit(8);
	emitter->SetParticleRandomOrientation(FloatRange(0.f, 359.f));
	emitter->SetParticleLifeRandomInRange(FloatRange(0.2f, 0.5f));
	emitter->SetTimeBetweenEachEmit(0.07f);
	emitter->SetEmitSpeed(m_velocity.GetLength() * g_theRNG->RollRandomFloatInRange(0.5f, 0.7f));
	emitter->SetParticleDefs(m_owner->m_vfxIndex);
}

void Projectile::PlaySoundOnImpact(bool byPass)
{
	if (!byPass)
	{
		if (!m_hasTouchedGround && !m_isHit) return;
		if (m_hasPlayedImpactedSound) return;
		m_hasPlayedImpactedSound = true;
	}

	m_map->m_game->m_audioManager->PlaySFXSound(m_projectileDef->m_soundName);
}
