#include "Controller.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Character.hpp"
#include "Game/Game.hpp"

Controller::Controller(Game* owner, Character* character, char ID, char teamID)
	:m_game(owner), m_character(character), m_ID(ID), m_teamID(teamID)
{
	AddVertsForAABB2D(m_nonTextureStaticUIVerts, AABB2(Vec2(45, 25), Vec2(550, 95)), Rgba8(230, 230, 230, 150)); // Data
	AddVertsForAABB2D(m_nonTextureStaticUIVerts, AABB2(Vec2(45, 95), Vec2(320, 125)), Rgba8::COLOR_DARK_GRAY); // Angle Box

	AddVertsForAABB2D(m_nonTextureStaticUIVerts, AABB2(Vec2(577, 25), Vec2(1083, 65)), Rgba8::COLOR_BLACK); // Force Border
	AddVertsForAABB2D(m_nonTextureStaticUIVerts, AABB2(Vec2(580, 28), Vec2(1080, 62)), Rgba8::COLOR_WHITE); // Force BG

	AddVertsForDisc2D(m_avatarBGVerts, Vec2(90, 90), 75, Rgba8::COLOR_BLACK, 32);// Avatar  border
	AddVertsForDisc2D(m_avatarBGVerts, Vec2(90, 90), 70, Rgba8::COLOR_WHITE, 32);// Avatar circle
	AddVertsForAABB2D(m_avatarVerts, AABB2(Vec2(30, 40), Vec2(150, 160)), Rgba8::COLOR_WHITE); // avatar

	AddVertsForAABB2D(m_nonTextureStaticUIVerts, AABB2(Vec2(340, 100), Vec2(390, 150)), Rgba8::COLOR_BLACK); // skill 1 border
	AddVertsForAABB2D(m_nonTextureStaticUIVerts, AABB2(Vec2(400, 100), Vec2(450, 150)), Rgba8::COLOR_BLACK); // skill 2  border
	AddVertsForAABB2D(m_nonTextureStaticUIVerts, AABB2(Vec2(460, 100), Vec2(510, 150)), Rgba8::COLOR_BLACK); // skill ultimate  border

	AddVertsForAABB2D(m_skill1UIVerts, AABB2(Vec2(343, 103), Vec2(387, 147)), Rgba8::COLOR_WHITE); // skill 1
	AddVertsForAABB2D(m_skill2UIVerts, AABB2(Vec2(403, 103), Vec2(447, 147)), Rgba8::COLOR_WHITE); // skill 2
	AddVertsForAABB2D(m_skillUltimateUIVerts, AABB2(Vec2(463, 103), Vec2(507, 147)), Rgba8::COLOR_WHITE); // skill ultimate

	if (m_character && m_character->m_characterDef->m_name == "Hela")
	{
		m_avatarVertsTexture = UIDefinitions::GetTexureByName("Character_Hela");
		m_skill1UITexture = UIDefinitions::GetTexureByName("Character_Hela_S1");
		m_skill2UITexture = UIDefinitions::GetTexureByName("Character_Hela_S2");
		m_skillUltUITexture = UIDefinitions::GetTexureByName("Character_Hela_U");
	}
	if (m_character && m_character->m_characterDef->m_name == "Iron")
	{
		m_avatarVertsTexture = UIDefinitions::GetTexureByName("Character_Iron");
		m_skill1UITexture = UIDefinitions::GetTexureByName("Character_Iron_S1");
		m_skill2UITexture = UIDefinitions::GetTexureByName("Character_Iron_S2");
		m_skillUltUITexture = UIDefinitions::GetTexureByName("Character_Iron_U");
	}
	if (m_character && m_character->m_characterDef->m_name == "Adam")
	{
		m_avatarVertsTexture = UIDefinitions::GetTexureByName("Character_Adam");
		m_skill1UITexture = UIDefinitions::GetTexureByName("Character_Adam_S1");
		m_skill2UITexture = UIDefinitions::GetTexureByName("Character_Adam_S2");
		m_skillUltUITexture = UIDefinitions::GetTexureByName("Character_Adam_U");
	}
	if (m_character && m_character->m_characterDef->m_name == "IW")
	{
		m_avatarVertsTexture = UIDefinitions::GetTexureByName("Character_IW");
		m_skill1UITexture = UIDefinitions::GetTexureByName("Character_IW_S1");
		m_skill2UITexture = UIDefinitions::GetTexureByName("Character_IW_S2");
		m_skillUltUITexture = UIDefinitions::GetTexureByName("Character_IW_U");
	}
	if (m_character && m_character->m_characterDef->m_name == "Thor")
	{
		m_avatarVertsTexture = UIDefinitions::GetTexureByName("Character_Thor");
		m_skill1UITexture = UIDefinitions::GetTexureByName("Character_Thor_S1");
		m_skill2UITexture = UIDefinitions::GetTexureByName("Character_Thor_S2");
		m_skillUltUITexture = UIDefinitions::GetTexureByName("Character_Thor_U");
	}
	if (m_character && m_character->m_characterDef->m_name == "Hulk")
	{
		m_avatarVertsTexture = UIDefinitions::GetTexureByName("Character_Hulk");
		m_skill1UITexture = UIDefinitions::GetTexureByName("Character_Hulk_S1");
		m_skill2UITexture = UIDefinitions::GetTexureByName("Character_Hulk_S2");
		m_skillUltUITexture = UIDefinitions::GetTexureByName("Character_Hulk_U");
	}

	m_skillUltimateUIColor = Rgba8::COLOR_WHITE;
}

void Controller::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);
}

void Controller::UpdateUI()
{
	if (!m_character) return;

	if (m_character->ConditionsForSkill_1())
	{
		m_skill1UIColor = Rgba8::COLOR_WHITE;
	}
	else
	{
		m_skill1UIColor = Rgba8(50, 50, 50, 200);
	}

	if (m_character->ConditionsForSkill_2())
	{
		m_skill2UIColor = Rgba8::COLOR_WHITE;
	}
	else
	{
		m_skill2UIColor = Rgba8(50, 50, 50, 200);
	}

	float percentHP = (float)m_character->m_health / (float)m_character->m_characterDef->m_initialHealth;
	if (percentHP > 0.7f)
	{
		m_backgroundColorBasedOnHP = Interpolate(Rgba8::COLOR_YELLOW, Rgba8::COLOR_GREEN, percentHP);
	}
	else if (percentHP > 0.3 && percentHP <= 0.7)
	{
		m_backgroundColorBasedOnHP = Interpolate(Rgba8::COLOR_ORANGE, Rgba8::COLOR_YELLOW, percentHP);
	}
	else if (percentHP <= 0.3)
	{
		m_backgroundColorBasedOnHP = Interpolate(Rgba8::COLOR_RED, Rgba8::COLOR_ORANGE, percentHP);
	}
}

void Controller::BeginTurn()
{
	m_character->BeginTurn();
}

void Controller::EndTurn()
{
	m_character->EndTurn();
}

void Controller::RenderUICharacter() const
{
	if (!m_character) return;

	std::vector<Vertex_PCU> UIVerts;

	Vec2 direction = Vec2::MakeFromPolarDegrees(m_character->m_currentAngle, 10.f);
	Vec2 pos = m_character->m_position;

	AddVertsForLineSegment2D(UIVerts, pos + direction * 0.5f, pos + direction * 1.0f, 0.5f, Rgba8::COLOR_ORANGE);

	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(UIVerts.size(), UIVerts.data());
}

void Controller::RenderUIScreen() const
{
	if (!m_character) return;

	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(m_nonTextureStaticUIVerts.size(), m_nonTextureStaticUIVerts.data());

	g_theRenderer->SetModelConstants(Mat44(), m_backgroundColorBasedOnHP);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(m_avatarBGVerts.size(), m_avatarBGVerts.data());

	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture(m_avatarVertsTexture);
	g_theRenderer->DrawVertexArray(m_avatarVerts.size(), m_avatarVerts.data());

	g_theRenderer->SetModelConstants(Mat44(), m_skill1UIColor);
	g_theRenderer->BindTexture(m_skill1UITexture);
	g_theRenderer->DrawVertexArray(m_skill1UIVerts.size(), m_skill1UIVerts.data());

	g_theRenderer->SetModelConstants(Mat44(), m_skill2UIColor);
	g_theRenderer->BindTexture(m_skill2UITexture);
	g_theRenderer->DrawVertexArray(m_skill2UIVerts.size(), m_skill2UIVerts.data());

	g_theRenderer->SetModelConstants(Mat44(), m_skillUltimateUIColor);
	g_theRenderer->BindTexture(m_skillUltUITexture);
	g_theRenderer->DrawVertexArray(m_skillUltimateUIVerts.size(), m_skillUltimateUIVerts.data());


	// FORCE BAR
	std::vector<Vertex_PCU> dynamicUIVerts;
	float forceRange = RangeMapClamped(m_character->m_currentForce, 0.f, 100.f, 580.f, 1080.f);
	float oldForceMark = RangeMapClamped(m_character->m_lastForce, 0.f, 100.f, 580.f, 1080.f);
	AddVertsForAABB2D2Color(dynamicUIVerts, AABB2(Vec2(580, 28), Vec2(forceRange, 62)), Rgba8::COLOR_ORANGE, Rgba8::COLOR_RED);
	AddVertsForAABB2D(dynamicUIVerts, AABB2(Vec2(630, 50), Vec2(633, 62)), Rgba8::COLOR_BLACK);
	AddVertsForAABB2D(dynamicUIVerts, AABB2(Vec2(680, 50), Vec2(683, 62)), Rgba8::COLOR_BLACK);
	AddVertsForAABB2D(dynamicUIVerts, AABB2(Vec2(730, 50), Vec2(733, 62)), Rgba8::COLOR_BLACK);
	AddVertsForAABB2D(dynamicUIVerts, AABB2(Vec2(780, 50), Vec2(783, 62)), Rgba8::COLOR_BLACK);
	AddVertsForAABB2D(dynamicUIVerts, AABB2(Vec2(830, 50), Vec2(833, 62)), Rgba8::COLOR_BLACK);
	AddVertsForAABB2D(dynamicUIVerts, AABB2(Vec2(880, 50), Vec2(883, 62)), Rgba8::COLOR_BLACK);
	AddVertsForAABB2D(dynamicUIVerts, AABB2(Vec2(930, 50), Vec2(933, 62)), Rgba8::COLOR_BLACK);
	AddVertsForAABB2D(dynamicUIVerts, AABB2(Vec2(980, 50), Vec2(983, 62)), Rgba8::COLOR_BLACK);
	AddVertsForAABB2D(dynamicUIVerts, AABB2(Vec2(1030, 50), Vec2(1033, 62)), Rgba8::COLOR_BLACK);

	AddVertsForAABB2D(dynamicUIVerts, AABB2(Vec2(605, 56), Vec2(608, 62)), Rgba8::COLOR_BLACK);
	AddVertsForAABB2D(dynamicUIVerts, AABB2(Vec2(705, 56), Vec2(708, 62)), Rgba8::COLOR_BLACK);
	AddVertsForAABB2D(dynamicUIVerts, AABB2(Vec2(805, 56), Vec2(808, 62)), Rgba8::COLOR_BLACK);
	AddVertsForAABB2D(dynamicUIVerts, AABB2(Vec2(905, 56), Vec2(908, 62)), Rgba8::COLOR_BLACK);
	AddVertsForAABB2D(dynamicUIVerts, AABB2(Vec2(1005, 56), Vec2(1008, 62)), Rgba8::COLOR_BLACK);
	AddVertsForAABB2D(dynamicUIVerts, AABB2(Vec2(655, 56), Vec2(658, 62)), Rgba8::COLOR_BLACK);
	AddVertsForAABB2D(dynamicUIVerts, AABB2(Vec2(755, 56), Vec2(758, 62)), Rgba8::COLOR_BLACK);
	AddVertsForAABB2D(dynamicUIVerts, AABB2(Vec2(855, 56), Vec2(858, 62)), Rgba8::COLOR_BLACK);
	AddVertsForAABB2D(dynamicUIVerts, AABB2(Vec2(955, 56), Vec2(958, 62)), Rgba8::COLOR_BLACK);

	AddVertsForAABB2D(dynamicUIVerts, AABB2(Vec2(oldForceMark - 1.5f, 28), Vec2(oldForceMark + 1.5f, 62)), Rgba8::COLOR_DARK_RED);
	Vec2 markBottom(oldForceMark, 30);
	AddVertsForTriagle2D(dynamicUIVerts, markBottom - Vec2(6.5, 10.f), markBottom + Vec2(6.5f, -10.f), markBottom, Rgba8::COLOR_DARK_RED);

	if (m_character->m_fury < 100)
	{
		AddVertsForAABB2D(dynamicUIVerts, AABB2(Vec2(463, 103 + (float)m_character->m_fury / 100.f * (147 - 103)), Vec2(507, 147)), Rgba8(0, 0, 0, 220)); // skill ultimate percentage
	}

	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(dynamicUIVerts.size(), dynamicUIVerts.data());


	std::vector<Vertex_PCU> textVerts;

	g_font->AddVertsForText2D(textVerts, Vec2(380.f, 30.f), 15.f, Stringf("Attack %i", m_character->m_attack), Rgba8::COLOR_BLACK);
	g_font->AddVertsForText2D(textVerts, Vec2(380.f, 50.f), 15.f, Stringf("Armor %i", m_character->m_armor), Rgba8::COLOR_BLACK);
	g_font->AddVertsForText2D(textVerts, Vec2(380.f, 70.f), 15.f, Stringf("Speed %i", m_character->m_speed), Rgba8::COLOR_BLACK);

	float staminaUI = Clamp(m_character->m_stamina, 0.f, 9999.f);
	g_font->AddVertsForText2D(textVerts, Vec2(180.f, 30.f), 15.f, Stringf("Stamina %.0f", staminaUI), Rgba8::COLOR_DARK_GREEN);
	if (m_character->m_shield > 0)
	{
		g_font->AddVertsForText2D(textVerts, Vec2(180.f, 50.f), 15.f, Stringf("Health %i", m_character->m_health + m_character->m_shield), Rgba8::COLOR_DARK_YELLOW);
	}
	else
	{
		g_font->AddVertsForText2D(textVerts, Vec2(180.f, 50.f), 15.f, Stringf("Health %i", m_character->m_health), Rgba8::COLOR_DARK_RED);
	}

	g_font->AddVertsForText2D(textVerts, Vec2(180.f, 70.f), 15.f, Stringf("Fury %i", m_character->m_fury), Rgba8::COLOR_DARK_BLUE);

	g_font->AddVertsForText2D(textVerts, Vec2(170.f, 100.f), 15.f, Stringf("Angle:%.0f", m_character->m_currentAngle), Rgba8::COLOR_BRIGHT_WHITE);

	g_font->AddVertsForText2D(textVerts, Vec2(375.f, 135.f), 10.f, "1", Rgba8::COLOR_ORANGE);
	g_font->AddVertsForText2D(textVerts, Vec2(435.f, 135.f), 10.f, "2", Rgba8::COLOR_ORANGE);
	g_font->AddVertsForText2D(textVerts, Vec2(495.f, 135.f), 10.f, "R", Rgba8::COLOR_ORANGE);

	if (m_character->m_skill1CDCounter > 0)
	{
		g_font->AddVertsForText2D(textVerts, Vec2(357.5f, 115.f), 15.f, Stringf("%i", m_character->m_skill1CDCounter), Rgba8::COLOR_BRIGHT_WHITE);
	}
	else
	{
		if (m_character->ConditionsForSkill_1())
		{
			g_font->AddVertsForText2D(textVerts, Vec2(345.f, 105.f), 10.f, Stringf("%i", m_character->m_characterDef->m_skill1Cost), Rgba8::COLOR_GREEN);
		}
	}

	if (m_character->m_skill2CDCounter > 0)
	{
		g_font->AddVertsForText2D(textVerts, Vec2(417.5f, 115.f), 15.f, Stringf("%i", m_character->m_skill2CDCounter), Rgba8::COLOR_BRIGHT_WHITE);
	}
	else
	{
		if (m_character->ConditionsForSkill_2())
		{
			g_font->AddVertsForText2D(textVerts, Vec2(405.f, 105.f), 10.f, Stringf("%i", m_character->m_characterDef->m_skill2Cost), Rgba8::COLOR_GREEN);
		}
	}

	g_font->AddVertsForText2D(textVerts, Vec2(465.f, 105.f), 10.f, Stringf("%.0f%s", (float)m_character->m_fury, "%"), Rgba8::COLOR_CYAN);

	g_font->AddVertsForText2D(textVerts, Vec2(580, 70), 15.f, Stringf("Last Turn Force %.0f", m_character->m_lastForce), Rgba8::COLOR_WHITE);
	g_font->AddVertsForText2D(textVerts, Vec2(616.5f, 35), 13.f, "10", Rgba8::COLOR_BLACK);
	g_font->AddVertsForText2D(textVerts, Vec2(666.5f, 35), 13.f, "20", Rgba8::COLOR_BLACK);
	g_font->AddVertsForText2D(textVerts, Vec2(716.5f, 35), 13.f, "30", Rgba8::COLOR_BLACK);
	g_font->AddVertsForText2D(textVerts, Vec2(766.5f, 35), 13.f, "40", Rgba8::COLOR_BLACK);
	g_font->AddVertsForText2D(textVerts, Vec2(816.5f, 35), 13.f, "50", Rgba8::COLOR_BLACK);
	g_font->AddVertsForText2D(textVerts, Vec2(866.5f, 35), 13.f, "60", Rgba8::COLOR_BLACK);
	g_font->AddVertsForText2D(textVerts, Vec2(916.5f, 35), 13.f, "70", Rgba8::COLOR_BLACK);
	g_font->AddVertsForText2D(textVerts, Vec2(966.5f, 35), 13.f, "80", Rgba8::COLOR_BLACK);
	g_font->AddVertsForText2D(textVerts, Vec2(1016.5f, 35), 13.f, "90", Rgba8::COLOR_BLACK);

	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture(&g_font->GetTexture());
	g_theRenderer->DrawVertexArray(textVerts.size(), textVerts.data());

}


