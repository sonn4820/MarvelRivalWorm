#include "AIController.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Character.hpp"
#include "Game/Game.hpp"
#include "Game/Projectile.hpp"
#include "Engine/Math/Spline.hpp"

AIController::AIController(Game* owner, Character* character, char ID, char teamID)
	:Controller(owner, character, ID, teamID)
{
	if (m_teamID != 1)
	{
		character->FaceLeft();
	}
	character->m_controller = this;

	if (m_character->m_characterDef->m_isMelee)
	{
		m_meleeDistance = m_character->m_characterDef->m_projectiles[0]->m_meleeMaxRange;
	}

	g_theEventSystem->SubscribeEventCallbackMemberFunction(character->m_characterDef->m_name + "AI", (AIController*)this, &AIController::Command_ModifyAI);
}

AIController::~AIController()
{

}

void AIController::Update(float deltaSeconds)
{
	if (m_game->m_playerIDTurn != m_ID) return;

	Controller::Update(deltaSeconds);

	m_thinkingTimer -= deltaSeconds;

	if (!m_currentFocusTarget)
	{
		m_currentFocusTarget = FindNearestRandomEnemy();
	}
	if (!m_hasChosenNewAngleAndForce)
	{
		ChoosingForceAndAngle();
		m_hasChosenNewAngleAndForce = true;
	}

	if (!m_doneMoving)
	{
		if (m_thinkingTimer < m_movingThinkingTime)
		{
			MovingLogic(deltaSeconds);
		}
	}
	else
	{
		UsingSkillLogic(deltaSeconds);

		if (m_thinkingTimer < m_forceAndAngleThinkingTime)
		{
			if (m_thinkingTimer > m_changingAngleTime)
			{
				float t = RangeMapClamped(m_thinkingTimer - m_changingAngleTime, m_forceAndAngleThinkingTime, m_changingAngleTime, 0.f, 1.f);
				m_character->m_currentAngle = GetTurnedTowardDegrees(m_character->m_currentAngle, m_currentAngle, 30.f * deltaSeconds * SmoothStart6(t));
			}
			else
			{
				float t = RangeMapClamped(m_thinkingTimer, m_changingAngleTime, 0.f, 0.f, 1.f);
				m_character->m_currentForce = Interpolate(0.f, m_currentForce, t);
			}
		}

		NormalAttackLogic();
	}
 }

void AIController::MovingLogic(float deltaSeconds)
{
	if (m_character->m_characterDef->m_isMelee)
	{
		m_character->FaceToTarget(m_currentFocusTarget->m_position);
		Vec2 toTarget = m_currentFocusTarget->m_position - m_character->m_position;

		if (toTarget.GetLengthSquared() <= (m_meleeDistance * m_meleeDistance - 2.f * 2.f))
		{
			if (m_skillLevel > 7.f)
			{
				if (!m_character->IsPositionBehind(m_currentFocusTarget->m_position))
				{
					m_character->FaceTheOpposite();
				}
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
				m_doneMoving = true;
			}

		}
	}
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
		if (!m_doneMoving)
		{
			if (!m_wantToMove)
			{
				m_wantToMove = g_theRNG->RollRandomChance(0.7f);
			}

			if (m_wantToMove)
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
				m_doneMoving = true;
			}
		}
	}

	if (m_doneMoving)
	{
		m_thinkingTimer = m_forceAndAngleThinkingTime + 0.5f;
	}
}

void AIController::NormalAttackLogic()
{
	if (m_thinkingTimer > 0.f) return;

	if (!m_hasPerfomedNA)
	{
		m_hasPerfomedNA = true;
		m_character->NormalAttack();
	}
}

bool AIController::IsPlayer() const
{
	return false;
}

Character* AIController::FindNearestRandomEnemy()
{
	Character* chosen = nullptr;
	bool m_hasGuaranteePick = false;

	for (Character* c : m_game->m_currentMap->m_characters)
	{
		if (c->GetTeamID() == m_teamID) continue;
		if (c->m_health == -1) continue;

		if (!m_hasGuaranteePick)
		{
			m_hasGuaranteePick = true;
			chosen = c;
		}
		else
		{
			if (g_theRNG->RollRandomChance(0.5))
			{
				chosen = c;
			}
		}
	}

	return chosen;
}

Character* AIController::FindDifferentEnemy()
{
	Character* chosen = nullptr;
	bool m_hasGuaranteePick = false;

	for (Character* c : m_game->m_currentMap->m_characters)
	{
		if (c->GetTeamID() == m_teamID) continue;
		if (c->m_health == -1) continue;
		if (c == m_currentFocusTarget) continue;

		if (!m_hasGuaranteePick)
		{
			m_hasGuaranteePick = true;
			chosen = c;
		}
		else
		{
			if (g_theRNG->RollRandomChance(0.5))
			{
				chosen = c;
			}
		}
	}

	return chosen;
}

Character* AIController::FindLowestInitialHPEnemy()
{
	int lowestOriginalHealthCharacter = INT_MAX;
	Character* result = nullptr;

	for (auto& character : m_game->m_currentMap->m_characters)
	{
		if (character->m_health == -1) continue;
		if (character->GetTeamID() != m_teamID)
		{
			if (character->m_characterDef->m_initialHealth < lowestOriginalHealthCharacter)
			{
				lowestOriginalHealthCharacter = character->m_characterDef->m_initialHealth;
				result = character;
			}
		}
	}

	return result;
}

Character* AIController::FindLowestCurrentHPEnemy()
{
	int lowestHealthCharacters = INT_MAX;
	Character* result = nullptr;

	for (auto& character : m_game->m_currentMap->m_characters)
	{
		if (character->m_health == -1) continue;
		if (character->GetTeamID() != m_teamID)
		{
			if (character->m_health< lowestHealthCharacters)
			{
				lowestHealthCharacters = character->m_health;
				result = character;
			}
		}
	}

	return result;
}

void AIController::SetAngleToPosition(Vec2 position)
{
	if (m_character->IsPositionBehind(position))
	{
		m_character->FaceTheOpposite();
	}

	m_currentAngle = (position - m_character->m_position).GetOrientationDegrees();
	m_currentAngle = Clamp(m_currentAngle, m_character->m_currentAngleLimit.m_min, m_character->m_currentAngleLimit.m_max);

}

void AIController::BeginTurn()
{
	m_thinkingSkillTimer = 1.f;
	m_ultThinkingTimer = 2.f;
	m_hasUsedSkill1 = false;
	m_hasUsedSkill2 = false;
	m_hasUsedUltimate = false;
	m_hasPerfomedNA = false;
	m_hasChosenNewAngleAndForce = false;

	m_forceAndAngleThinkingTime = g_theRNG->RollRandomFloatInRange(3.f, 5.f);
	m_changingAngleTime = m_forceAndAngleThinkingTime * 0.5f;
	m_movingThinkingTime = m_forceAndAngleThinkingTime + g_theRNG->RollRandomFloatInRange(1.f, 2.f);
	m_thinkingTimer = m_movingThinkingTime + 2.f;

	m_staminaMovingLimit = m_character->m_characterDef->m_initialStamina -  g_theRNG->RollRandomFloatInRange(0.f, 50.f);
	m_wantToMove = true;
	m_doneMoving = false;
	Controller::BeginTurn();
}

void AIController::EndTurn()
{
	Controller::EndTurn();
}

float AIController::RollRandomeForce(float tMin, float tMax)
{
	float range = m_character->m_currentForceLimit.m_max - m_character->m_currentForceLimit.m_min;
	float randomValue = g_theRNG->RollRandomFloatInRange(range * tMin, range * tMax);
	return m_character->m_currentForceLimit.m_min + randomValue;
}

float AIController::RollRandomeAngle(float tMin, float tMax)
{
	float range = m_character->m_currentAngleLimit.m_max - m_character->m_currentAngleLimit.m_min;
	float randomValue = g_theRNG->RollRandomFloatInRange(range * tMin, range * tMax);
	return m_character->m_currentAngleLimit.m_min + randomValue;
}

float AIController::GetRandomValueBasedOnSkillLevel(float minOffset, float maxOffset)
{
	float halfRange = (maxOffset - minOffset) * 0.5f;
	float midpoint = (minOffset + maxOffset) * 0.5f;
	float skillDeviation = 1.f - (m_skillLevel / 10.f);
	return g_theRNG->RollRandomFloatInRange((midpoint - halfRange) * skillDeviation, (midpoint + halfRange) * skillDeviation);
}

void AIController::SetCurrentForceToHitThisTarget(float angleValue, Vec2 targetPosition)
{
	Vec2 toTarget = targetPosition - m_character->m_position;
	toTarget = Vec2(fabsf(toTarget.x), toTarget.y);

	m_currentAngle = angleValue;

	float cos = CosDegrees(angleValue);
	float tan = TanDegrees(angleValue);

	float n = -GRAVITY_FORCE_PROJECTILE * toTarget.x * toTarget.x;
	float d = 2 * cos * cos * (toTarget.x * tan - toTarget.y);

	if (d <= 0)
	{
		m_currentForce = 0.f;
	}
	else
	{
		m_currentForce = sqrtf(n / d);
	}

	m_character->FaceToTarget(targetPosition);

	if (m_character->m_isFacingLeft) m_currentAngle = 180.f - m_currentAngle;

	m_currentForce *= (2.f / 3.f);
}

int AIController::GetAngryTurn()
{
	if (m_game->m_turnHasPassed > 6 && m_canLearn)
	{
		m_skillLevel += 0.1f;
		m_skillLevel = Clamp(m_skillLevel, 0.f, 10.f);
	}

	if (m_game->m_turnHasPassed > 30) return true;
	return false;
}

bool AIController::Command_ModifyAI(EventArgs& args)
{
	if (args.IsKeyNameValid("Skill"))
	{
		std::string result = args.GetValue<std::string>("skill", "5.0");
		m_skillLevel = (float)atof(result.c_str());
		m_skillLevel = Clamp(m_skillLevel, 0.f, 10.f);
		g_theDevConsole->AddLine(DevConsole::SUCCESS, m_character->m_characterDef->m_name + "'s AI SKill Level is set to: " + result);
	}
	if (args.IsKeyNameValid("CanLearn"))
	{
		std::string result = args.GetValue<std::string>("learn", "false");
		if (result == "true")
		{
			m_canLearn = true;
		}
		else
		{
			m_canLearn = false;
		}
		g_theDevConsole->AddLine(DevConsole::SUCCESS, m_character->m_characterDef->m_name + "'s AI can learn: " + result);
	}
	return false;
}

void AIController::ChoosingForceAndAngle()
{
	if (m_character->m_characterDef->m_isMelee) // MELEE
	{
		Vec2 toTargetPosition = m_currentFocusTarget->m_position - m_character->m_position;
		m_currentAngle = toTargetPosition.GetOrientationDegrees();
		return;
	}


	if (GetAngryTurn()) // GAME TOO LONG
	{
		SetCurrentForceToHitThisTarget(g_theRNG->RollRandomFloatInRange(25.f, 65.f), m_currentFocusTarget->m_position);
		m_currentForce = Clamp(m_currentForce, m_character->m_currentForceLimit.m_min, m_character->m_currentForceLimit.m_max);
		m_currentAngle = Clamp(m_currentAngle, m_character->m_currentAngleLimit.m_min, m_character->m_currentAngleLimit.m_max);
		return;
	}
	//---------------------------------------------------------
	if (m_isGuessing)
	{
		// TOTALLY RANDOM
		PureGuessingShot();
		m_isGuessing = false;
	}
	else
	{
		if (m_lastHitPosition != Vec2(-1, -1))
		{
			if (IsEnemyNearTheLastHit())
			{
				TryToHitTheCurrentTarget();
			}
			else
			{
				PureGuessingShot();
			}

			m_currentForce += GetRandomValueBasedOnSkillLevel(-10.f, 10.f);
			m_currentAngle += GetRandomValueBasedOnSkillLevel(-15.f, 15.f);
		}
		else
		{
			//MISS COMPLETELY -> Create a good enough formula to hit if skill high enough
			if (m_skillLevel > 5.f && m_guessCount > 3)
			{
				m_guessCount = 0;
				SetCurrentForceToHitThisTarget(g_theRNG->RollRandomFloatInRange(0.f, 90.f), m_currentFocusTarget->m_position);
			}
			else
			{
				PureGuessingShot();
			}
		}
	}

	m_currentForce = Clamp(m_currentForce, m_character->m_currentForceLimit.m_min, m_character->m_currentForceLimit.m_max);
	m_currentAngle = Clamp(m_currentAngle, m_character->m_currentAngleLimit.m_min, m_character->m_currentAngleLimit.m_max);
}


void AIController::PureGuessingShot()
{
	m_character->FaceToTarget(m_currentFocusTarget->m_position);
	m_currentForce = RollRandomeForce(0.5f, 0.85f) + GetRandomValueBasedOnSkillLevel(-10.f, 10.f);
	m_currentAngle = RollRandomeAngle(0.4f, 0.6f) + GetRandomValueBasedOnSkillLevel(-15.f, 15.f);

	m_guessCount++;
}

void AIController::TryToHitTheCurrentTarget()
{
	Vec2 toHitPosition = m_lastHitPosition - m_character->m_position;
	Vec2 toTargetPosition = m_currentFocusTarget->m_position - m_character->m_position;

	float hitLength = toHitPosition.GetLength();
	float posLength = toTargetPosition.GetLength();

	float ratio = hitLength / posLength;

	m_character->FaceToTarget(m_currentFocusTarget->m_position);

	if (ratio < 1.0f)
	{
		float forceToIncrease = posLength - hitLength;
		float ratioToMatch = forceToIncrease / hitLength;
		m_currentForce += m_currentForce * ratioToMatch;
	}
	else
	{
		float forceToDecrease = hitLength - posLength;
		float ratioToMatch = forceToDecrease / hitLength;
		m_currentForce -= m_currentForce * ratioToMatch;
	}
}

bool AIController::IsEnemyNearTheLastHit() const
{
	for (auto& character : m_game->m_currentMap->m_characters)
	{
		if (character->GetTeamID() == m_teamID) continue;
		if (DoDiscsOverlap2D(m_lastHitPosition, 16.f, character->m_position, character->m_currentHitboxRadius))
		{
			return true;
		}
	}

	return false;
}

bool AIController::RandomTurn()
{
	if (g_theRNG->RollRandomChance(0.5f))
	{
		m_character->FaceTheOpposite();
		return true;
	}
	return false;
}
