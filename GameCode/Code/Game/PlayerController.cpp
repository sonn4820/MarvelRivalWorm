#include "Game/PlayerController.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Character.hpp"
#include "Game/Game.hpp"

PlayerController::PlayerController(Game* owner, Character* character, char ID, char teamID)
	:Controller(owner, character, ID, teamID)
{
	if (m_teamID != 1)
	{
		character->FaceLeft();
	}
	character->m_controller = this;
}

void PlayerController::Update(float deltaSeconds)
{
	if (!m_character) return;
	if (m_game->IsPendingEndTunr()) return;

	Controller::Update(deltaSeconds);

	m_isMoving = false;

	if (g_theInput->WasKeyJustReleased('A')
		|| g_theInput->WasKeyJustReleased('D')
		|| g_theInput->WasKeyJustReleased('Q')
		|| g_theInput->WasKeyJustReleased('E'))
	{
		m_character->m_isSpendingStamina = false;
	}

	if (g_theInput->IsKeyDown('A') && !m_isChargingForShoot)
	{
		m_isMoving = true;
		m_character->FaceLeft();
		m_character->MoveLeft(deltaSeconds);
		if (g_theInput->IsKeyDown('D'))
		{
			m_character->m_isSpendingStamina = true;
		}
		else
		{
			if (g_theInput->IsKeyDown('Q') || g_theInput->IsKeyDown('E'))
			{
				m_character->m_isSpendingStamina = false;
			}
		}
	}
	if (g_theInput->IsKeyDown('D') && !m_isChargingForShoot)
	{
		m_isMoving = true;
		m_character->FaceRight();
		m_character->MoveRight(deltaSeconds);
		if (g_theInput->IsKeyDown('A'))
		{
			m_character->m_isSpendingStamina = true;
		}
		else
		{
			if (g_theInput->IsKeyDown('Q') || g_theInput->IsKeyDown('E'))
			{
				m_character->m_isSpendingStamina = false;
			}
		}
	}
	if (g_theInput->IsKeyDown('Q') && !m_isChargingForShoot)
	{
		m_isMoving = true;
		m_character->MoveUp(deltaSeconds);
		if (g_theInput->IsKeyDown('E'))
		{
			m_character->m_isSpendingStamina = true;
		}
		else
		{
			if (g_theInput->IsKeyDown('A') || g_theInput->IsKeyDown('D'))
			{
				m_character->m_isSpendingStamina = false;
			}
		}
	}
	if (g_theInput->IsKeyDown('E') && !m_isChargingForShoot)
	{
		m_isMoving = true;
		m_character->MoveDown(deltaSeconds);
		if (g_theInput->IsKeyDown('Q'))
		{
			m_character->m_isSpendingStamina = true;
		}
		else
		{
			if (g_theInput->IsKeyDown('A') || g_theInput->IsKeyDown('D'))
			{
				m_character->m_isSpendingStamina = false;
			}
		}
	}
	if (g_theInput->IsKeyDown('W') && !m_isChargingForShoot)
	{
		m_character->AdjustAngle(1.f, deltaSeconds);
	}
	if (g_theInput->IsKeyDown('S') && !m_isChargingForShoot)
	{
		m_character->AdjustAngle(-1.f, deltaSeconds);
	}
	if (g_theInput->WasKeyJustPressed(49) && !m_isChargingForShoot)
	{
		m_character->Skill_1();
	}
	if (g_theInput->WasKeyJustPressed(50) && !m_isChargingForShoot)
	{
		m_character->Skill_2();
	}
	if (g_theInput->WasKeyJustPressed('R') && !m_isChargingForShoot)
	{
		m_character->Ultimate();
	}
	if (g_theInput->IsKeyDown(KEYCODE_SPACE) && !m_isMoving)
	{
		m_isChargingForShoot = true;
		m_character->AdjustForce(deltaSeconds);
	}
	if (g_theInput->WasKeyJustReleased(KEYCODE_SPACE) && !m_isMoving)
	{
		m_isChargingForShoot = false;
		m_character->NormalAttack();
	}
}

bool PlayerController::IsPlayer() const
{
	return true;
}