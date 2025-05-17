#include "Game/Game.hpp"
#include "Game/Entity.hpp"
#include "Game/CharacterCommon.hpp"
#include "Game/CharacterList.hpp"
#include "Game/PlayerController.hpp"
#include "Game/AIController.hpp"
#include "Game/ParticleSystemDFS2.hpp"
#include "Game/AudioManagerDFS2.hpp"

Game::Game()
{

}
//..............................
Game::~Game()
{

}
//..............................
void Game::Startup()
{
	g_theJobSystem->SetWorkerThreadIDJobFlags(LOADING_ASSET_JOB_BITFLAG, LOADING_THREAD_ID);

	m_vfxSystem = new ParticleSystemDFS2();

	float aspect = Window::GetMainWindowInstance()->GetAspect();
	IntVec2 screenSize = Window::GetMainWindowInstance()->GetClientDimensions();
	float x = WORLD_SIZE_Y * aspect;
	m_cameraView.m_maxs = Vec2(x, WORLD_SIZE_Y);
	m_worldCamera.SetOrthographicView(m_cameraView.m_mins, m_cameraView.m_maxs);
	m_screenCamera.SetOrthographicView(Vec2(0, 0), Vec2((float)screenSize.x, (float)screenSize.y));

	m_gameClock = new Clock(*Clock::s_theSystemClock);

	m_menuMusic.m_ID = g_theAudio->CreateOrGetSound("Data/Audios/Music/Menu.wav");

	AudioDefinitions::InitializeDefs("Data/Definitions/AudioDefinitions.xml");
	ProjectileDefinition::InitializeDefs("Data/Definitions/ProjectileDefinitions.xml");
	CharacterDefinition::InitializeDefs("Data/Definitions/CharacterDefinitions.xml");
	ItemDefinition::InitializeDefs("Data/Definitions/ItemDefinitions.xml");
	BlockDefinition::InitializeBlockDefs();
	SquareParticleDefinition::InitializeBlockDefs();

	m_endGameWaitTimer = new Timer(6.f, m_gameClock);

	m_audioManager = new AudioManagerDFS2(g_theAudio);

	m_menuCanvas = new Canvas(g_theUI, &m_screenCamera);
	m_characterCanvas = new Canvas(g_theUI, &m_screenCamera);
	m_inGameCanvas = new Canvas(g_theUI, &m_screenCamera);

	g_theEventSystem->SubscribeEventCallbackFunction("Next", Game::Command_NextTurn);
	g_theEventSystem->SubscribeEventCallbackFunction("Restart", Game::Command_GameRestart);
	g_theEventSystem->SubscribeEventCallbackFunction("test", Game::Command_TestCase);
	g_theEventSystem->SubscribeEventCallbackFunction("Demo", Game::Command_ToggleDemo);
	g_theEventSystem->SubscribeEventCallbackFunction("DebugDraw", Game::Command_ToggleDebugDraw);
	g_theEventSystem->SubscribeEventCallbackFunction("DebugDestructible", Game::Command_ToggleDestructible);
	g_theEventSystem->SubscribeEventCallbackFunction("DebugFast", Game::Command_ToggleMoveFast);
	g_theEventSystem->SubscribeEventCallbackFunction("Debug", Game::Command_ToggleDebugMode);

	UIDefinitions::InitializeDefs("Data/Definitions/UIDefinitions.xml");

	LoadUISprite();

	MainMenuUI_Init();
	SelectCharacterUI_Init();
	InGameUI_Init();

	SwitchState(GameState::MENU_MODE);
}
//..............................
void Game::Shutdown()
{
	m_isUIQuitting = true;

	g_theEventSystem->UnsubscribeEventCallbackFunction("Next", Game::Command_NextTurn);
	g_theEventSystem->UnsubscribeEventCallbackFunction("Restart", Game::Command_GameRestart);
	g_theEventSystem->UnsubscribeEventCallbackFunction("test", Game::Command_TestCase);
	g_theEventSystem->UnsubscribeEventCallbackFunction("Demo", Game::Command_ToggleDemo);
	g_theEventSystem->UnsubscribeEventCallbackFunction("DebugDraw", Game::Command_ToggleDebugDraw);
	g_theEventSystem->UnsubscribeEventCallbackFunction("DebugDestructible", Game::Command_ToggleDestructible);
	g_theEventSystem->UnsubscribeEventCallbackFunction("DebugFast", Game::Command_ToggleMoveFast);
	g_theEventSystem->UnsubscribeEventCallbackFunction("Debug", Game::Command_ToggleDebugMode);

	delete m_menuCanvas;
	delete m_characterCanvas;
	delete m_inGameCanvas;
	delete m_currentMap;

	for (auto& controller : m_controllers)
	{
		delete controller;
		controller = nullptr;
	}
	m_controllers.clear();
	m_controllerTookTurn.clear();
	m_currentMap = nullptr;

	delete m_vfxSystem;

	delete m_audioManager;

	AudioDefinitions::ClearDefinition();
	ProjectileDefinition::ClearDefinition();
	CharacterDefinition::ClearDefinition();
	ItemDefinition::ClearDefinition();
	BlockDefinition::ClearDefinition();
	SquareParticleDefinition::ClearDefinition();
	UIDefinitions::ClearDefinition();
}

//----------------------------------------------------------------------------------------------------------------------------------------
// UPDATE

void Game::Update()
{
	float deltaSecond = Clock::s_theSystemClock->GetDeltaSeconds();
	m_secondIntoMode += deltaSecond;

	m_audioManager->Update(deltaSecond);

	if (m_state == GameState::MENU_MODE)
	{
		UpdateMenuMode(deltaSecond);
	}
	if (m_state == GameState::SELECT_CHARACTERS_MODE)
	{
		UpdateCharacterSelectMode(deltaSecond);
	}
	if (m_state == GameState::PLAY_MODE)
	{
		UpdatePlayMode(deltaSecond);
	}

	if (!m_currentMap) return;
	UpdateCamera(deltaSecond);
}
//..............................
void Game::UpdatePlayMode(float deltaSecond)
{
	if (m_gameHasEnded)
	{
		if (m_endGameWaitTimer->IsStopped())
		{
			SetCooldownCamera(4.f);
			m_endGameWaitTimer->Start();
		}
		if (m_endGameWaitTimer->HasPeriodElapsed())
		{
			SwitchState(GameState::MENU_MODE);
		}
		return;
	}

	m_timerNextTurnPanel -= deltaSecond;

	m_inGameCanvas->Update(deltaSecond);
	for (auto& cont : m_controllers)
	{
		if (!cont->m_character) continue;
		if (cont->m_ID == m_playerIDTurn)
		{
			cont->UpdateUI();
		}
	}

	HandleInput();

	TurnAndCameraManaging(deltaSecond);

	if (m_timerNextTurnPanel < 0.f)
	{
		if (m_nextTurnPanel->IsActive())
		{
			m_nextTurnPanel->SetActive(false);
		}

		for (auto& cont : m_controllers)
		{
			if (!cont->m_character) continue;
			if (cont->m_ID == m_playerIDTurn)
			{
				cont->Update(deltaSecond);
				m_currentFocusCharacter = cont->m_character;
			}
		}
	}

	m_vfxSystem->Update(deltaSecond);

	m_currentMap->Update(deltaSecond);

	//DebugAddScreenText(Stringf("Time: %.1f, FPS: %.1f, Scale: %.1f", Clock::s_theSystemClock->GetTotalSeconds(), 1.f / Clock::s_theSystemClock->GetDeltaSeconds(), Clock::s_theSystemClock->GetTimeScale()), Vec2(750.f, 780.f), 15.f);
	//DebugAddScreenText(Stringf("Camera Cool down :%.2f", m_cameraMoveCooldown), Vec2(750, 750), 15.f);
	//DebugAddScreenText(Stringf("Turn: %i", m_turnHasPassed), Vec2(750, 730), 15.f);
	//if (m_isDemo) DebugAddScreenText("IS DEMO", Vec2(750, 710), 15.f, Rgba8::COLOR_GREEN);
}
//..............................
void Game::UpdateMenuMode(float deltaSecond)
{
	m_menuCanvas->Update(deltaSecond);
}

void Game::UpdateCharacterSelectMode(float deltaSecond)
{
	m_characterCanvas->Update(deltaSecond);

	ToggleVisibilityCharacterInfo();

	bool isTeam1 = m_currentSelectingPlayerTurn % 2 == 0;

	if (m_currentSelectingPlayerTurn > 5)
	{
		if (!m_playGameButton->IsActive()) // READY TO PLAY GAME
		{
			m_playGameButton->SetActive(true);
			for (size_t i = 0; i < 6; i++)
			{
				m_charactersButton[i]->SetActive(false);
			}
			m_selectVS->SetActive(true);

			m_randomSeedMapButton->SetActive(true);

			if (m_audioManager->AreAllSoundsLoaded())
			{
				g_theAudio->StopSound(m_currentMusic);
				m_audioManager->PlayMusic("AllSelected", m_currentMusic);
			}
		}
	}
	else
	{
		if (m_playGameButton->IsActive())
		{
			m_playGameButton->SetActive(false);
		}
	}

	if (m_selectCharactersPanel->IsActive())
	{
		for (size_t i = 0; i < 6; i++)
		{
			if (m_playerSelection[i] == -1)
			{
				m_playersButton[i]->SetActive(false);
				if (m_currentSelectingPlayerTurn == i)
				{
					m_currentSelectingPlayerTurn++;
				}
			}
			if (i == m_currentSelectingPlayerTurn)
			{
				m_playersButton[i]->m_borderColor = Rgba8::COLOR_YELLOW;
			}
			else
			{
				if (i < m_currentSelectingPlayerTurn)
				{
					if (i % 2 == 0)
					{
						m_playersButton[i]->m_borderColor = Rgba8::COLOR_RED;
					}
					else
					{
						m_playersButton[i]->m_borderColor = Rgba8::COLOR_BRIGHT_BLUE;
					}

				}
				else
				{
					m_playersButton[i]->m_borderColor = Rgba8::COLOR_DARK_GRAY;
				}
			}
		}
	}


	if (isTeam1)
	{
		for (size_t i = 0; i < 6; i++)
		{
			if (m_characterAvailableTeam1Selection[i])
			{
				m_charactersButton[i]->m_borderColor = Rgba8::COLOR_BRIGHT_WHITE;
				m_charactersButton[i]->m_colorUnhover = Rgba8::COLOR_GRAY;
				m_charactersButton[i]->m_colorHover = Rgba8::COLOR_WHITE;
				m_charactersButton[i]->m_colorPressed = Rgba8::COLOR_BRIGHT_WHITE;
			}
			else
			{
				m_charactersButton[i]->m_borderColor = Rgba8::COLOR_BLACK;
				m_charactersButton[i]->m_colorUnhover = Rgba8::COLOR_DARK_GRAY;
				m_charactersButton[i]->m_colorHover = Rgba8::COLOR_DARK_GRAY;
				m_charactersButton[i]->m_colorPressed = Rgba8::COLOR_DARK_GRAY;
			}
		}
	}
	else
	{
		for (size_t i = 0; i < 6; i++)
		{
			if (m_characterAvailableTeam2Selection[i])
			{
				m_charactersButton[i]->m_borderColor = Rgba8::COLOR_BRIGHT_WHITE;
				m_charactersButton[i]->m_colorUnhover = Rgba8::COLOR_GRAY;
				m_charactersButton[i]->m_colorHover = Rgba8::COLOR_WHITE;
				m_charactersButton[i]->m_colorPressed = Rgba8::COLOR_BRIGHT_WHITE;
			}
			else
			{
				m_charactersButton[i]->m_borderColor = Rgba8::COLOR_BLACK;
				m_charactersButton[i]->m_colorUnhover = Rgba8::COLOR_DARK_GRAY;
				m_charactersButton[i]->m_colorHover = Rgba8::COLOR_DARK_GRAY;
				m_charactersButton[i]->m_colorPressed = Rgba8::COLOR_DARK_GRAY;
			}
		}
	}
}

//..............................
void Game::UpdateCamera(float deltaSecond)
{
	m_cameraMoveCooldown -= deltaSecond;

	if (g_theInput->WasMouseWheelScrolledUp())
	{
		GameCameraCursorZoom(m_cursorPos, 0.9f);
	}
	if (g_theInput->WasMouseWheelScrolledDown())
	{
		GameCameraCursorZoom(m_cursorPos, 1.1f);
	}

	GameCameraCursorMove(m_cursorPos, 100.f * deltaSecond);

	AABB2 camShake = m_cameraView;
	float xReduce = 4.f;
	float yReduce = xReduce * Window::GetMainWindowInstance()->GetAspect();

	camShake.m_mins += Vec2(xReduce, yReduce);
	camShake.m_maxs -= Vec2(xReduce, yReduce);

	if (m_screenShakeAmount > 0.f)
	{
		float shakeX = g_theRNG->RollRandomFloatInRange(-m_screenShakeAmount, m_screenShakeAmount);
		float shakeY = g_theRNG->RollRandomFloatInRange(-m_screenShakeAmount, m_screenShakeAmount);
		camShake.m_mins += Vec2(shakeX, shakeY);
		camShake.m_maxs += Vec2(shakeX, shakeY);
		m_screenShakeAmount -= SHAKE_REDUCTION_PER_SEC * deltaSecond;
	}

	m_screenShakeAmount = Clamp(m_screenShakeAmount, 0.f, MAX_SHAKE);

	m_worldCamera.SetOrthographicView(camShake.m_mins, camShake.m_maxs);
}

//----------------------------------------------------------------------------------------------------------------------------------------
// RENDER 

void Game::Render() const
{
	if (m_state == GameState::MENU_MODE)
	{
		RenderMenuMode();
	}
	if (m_state == GameState::SELECT_CHARACTERS_MODE)
	{
		RenderCharacterSelectMode();
	}
	if (m_state == GameState::PLAY_MODE)
	{
		RenderPlayMode();
	}

	RenderScreen();
}
//..............................

void Game::RenderPlayMode() const
{
	g_theRenderer->ClearScreen(Rgba8(120, 120, 205, 255));
	g_theRenderer->BeginCamera(m_worldCamera);

	m_currentMap->Render();
	m_inGameCanvas->Render();
	m_vfxSystem->Render();

	if (!m_gameHasEnded)
	{
		for (auto& player : m_controllers)
		{
			if (player->m_ID == m_playerIDTurn)
			{
				player->RenderUICharacter();
			}
		}
	}

	DebugRenderWorld(m_worldCamera);
	DebugRenderScreen(m_screenCamera);

	g_theRenderer->EndCamera(m_worldCamera);
}
//..............................
void Game::RenderMenuMode() const
{
	g_theRenderer->ClearScreen(Rgba8(0, 0, 0, 255));
	g_theRenderer->BeginCamera(m_screenCamera);
	m_menuCanvas->Render();
	g_theRenderer->EndCamera(m_screenCamera);
}

void Game::RenderCharacterSelectMode() const
{
	g_theRenderer->ClearScreen(Rgba8(0, 0, 0, 255));
	g_theRenderer->BeginCamera(m_screenCamera);
	m_characterCanvas->Render();
	g_theRenderer->EndCamera(m_screenCamera);
}

//..............................
void Game::RenderScreen() const
{
	g_theRenderer->BeginCamera(m_screenCamera);
	if (m_state == GameState::PLAY_MODE)
	{
		for (auto& player : m_controllers)
		{
			if (m_timerNextTurnPanel < 0.f && player->m_ID == m_playerIDTurn)
			{
				player->RenderUIScreen();
			}
		}
	}
	g_theRenderer->EndCamera(m_screenCamera);
}


void Game::LoadUISprite()
{
	for (size_t i = 0; i < UIDefinitions::s_uiDefList.size(); i++)
	{
		Texture* newTexture = g_theRenderer->CreateOrGetTextureFromFile(UIDefinitions::s_uiDefList[i]->m_path.c_str());
		m_UITextures.push_back(newTexture);
		UIDefinitions::s_uiDefList[i]->m_index = (int)m_UITextures.size() - 1;
	}
}

void Game::RandomSeed()
{
	m_mapSeed = g_theRNG->RollRandomUnsignedIntInRange(0, 0xFFFFFFFE);
	m_seedMapText->SetText(Stringf("Map Seed : % i", m_mapSeed));
}

void Game::PlayGame()
{
	if (m_audioManager->AreAllSoundsLoaded())
	{
		SwitchState(GameState::PLAY_MODE);
		GameRestart();
	}
	else
	{
		static int count;
		g_theDevConsole->AddLine(Rgba8::COLOR_YELLOW, Stringf("Attempt %i: Sounds are not fully loaded! Please Wait", count));
	}
}

void Game::OpenSetting()
{
	m_settingMainPanel->SetActive(true);
	m_menuMainPanel->SetActive(false);
}

void Game::OpenHowToPlay()
{
	m_howToPlayPanel->SetActive(true);
	m_menuMainPanel->SetActive(false);
}

void Game::OpenMenuOptions()
{
	m_settingMainPanel->SetActive(false);
	m_howToPlayPanel->SetActive(false);
	m_menuMainPanel->SetActive(true);
}

void Game::OpenCharacterSelect()
{
	int numTeam1 = 0;
	int numTeam2 = 0;

	for (size_t i = 0; i < 6; i++)
	{
		if (m_playerSelection[i] != -1)
		{
			if (i % 2 == 0)
			{
				numTeam1++;
			}
			else
			{
				numTeam2++;
			}
		}
	}

	if (numTeam1 < 1 || numTeam2 < 1)
	{
		return;
	}

	m_selectCharactersPanel->SetActive(true);
	m_selectTeamPanel->SetActive(false);
	m_selectVS->SetActive(false);

	if (m_audioManager->AreAllSoundsLoaded())
	{
		g_theAudio->StopSound(m_currentMusic);
		m_audioManager->PlayMusic("SelectCharacter", m_currentMusic);
	}
}

void Game::SetButtonPlayer(char id)
{
	if (m_selectCharactersPanel->IsActive())return;

	m_playerSelection[id]++;
	if (m_playerSelection[id] > 1)
	{
		m_playerSelection[id] = -1;
	}

	if (m_playerSelection[id] == -1)
	{
		m_playersButton[id]->m_textureUnhover = UIDefinitions::GetTexureByName("Select_Add");
		m_playersButton[id]->m_textureHover = UIDefinitions::GetTexureByName("Select_Add");
		m_playersButton[id]->m_texturePressed = UIDefinitions::GetTexureByName("Select_Add");
	}
	if (m_playerSelection[id] == 0)
	{
		m_playersButton[id]->m_textureUnhover = UIDefinitions::GetTexureByName("Select_Player");
		m_playersButton[id]->m_textureHover = UIDefinitions::GetTexureByName("Select_Player");
		m_playersButton[id]->m_texturePressed = UIDefinitions::GetTexureByName("Select_Player");
	}
	if (m_playerSelection[id] == 1)
	{
		m_playersButton[id]->m_textureUnhover = UIDefinitions::GetTexureByName("Select_AI");
		m_playersButton[id]->m_textureHover = UIDefinitions::GetTexureByName("Select_AI");
		m_playersButton[id]->m_texturePressed = UIDefinitions::GetTexureByName("Select_AI");
	}
}

void Game::SetButtonCharacter(char characterID)
{
	if (m_currentSelectingPlayerTurn > 5) return;
	if (m_currentSelectingPlayerTurn == -1) return;

	if (m_currentSelectingPlayerTurn % 2 == 0)
	{
		if (!m_characterAvailableTeam1Selection[characterID]) return;
	}
	else
	{
		if (!m_characterAvailableTeam2Selection[characterID]) return;
	}

	m_playersButton[m_currentSelectingPlayerTurn]->m_textureUnhover = m_charactersButton[characterID]->m_textureUnhover;
	m_playersButton[m_currentSelectingPlayerTurn]->m_textureHover = m_charactersButton[characterID]->m_textureHover;
	m_playersButton[m_currentSelectingPlayerTurn]->m_texturePressed = m_charactersButton[characterID]->m_texturePressed;
	m_playersButton[m_currentSelectingPlayerTurn]->m_colorUnhover = Rgba8::COLOR_WHITE;
	m_playersButton[m_currentSelectingPlayerTurn]->m_colorHover = Rgba8::COLOR_WHITE;
	m_playersButton[m_currentSelectingPlayerTurn]->m_colorPressed = Rgba8::COLOR_WHITE;

	switch (characterID)
	{
	case 0: m_characterSelection[m_currentSelectingPlayerTurn] = "Hela"; break;
	case 1: m_characterSelection[m_currentSelectingPlayerTurn] = "Iron"; break;
	case 2: m_characterSelection[m_currentSelectingPlayerTurn] = "Adam"; break;
	case 3: m_characterSelection[m_currentSelectingPlayerTurn] = "IW"; break;
	case 4: m_characterSelection[m_currentSelectingPlayerTurn] = "Thor"; break;
	case 5: m_characterSelection[m_currentSelectingPlayerTurn] = "Hulk"; break;
	}

	if (m_currentSelectingPlayerTurn % 2 == 0)
	{
		m_characterAvailableTeam1Selection[characterID] = false;
	}
	else
	{
		m_characterAvailableTeam2Selection[characterID] = false;
	}

	m_currentSelectingPlayerTurn++;
}

void Game::ShowButtonCharacterInfo(char characterID)
{
	m_characterButtonIsHoveringState[characterID] = true;

	switch (characterID)
	{
	case 0: 
		m_characterTextName->SetText("Hela");
		m_characterTextP_Info->SetText("Deal damage gain attack bonus permanently");
		m_characterTextS1_Info->SetText("Add one extra projectile to normal attack");
		m_characterTextS2_Info->SetText("Throw teleporting pearl");
		m_characterTextU_Info->SetText("Fly up and throw powerful projectile");
		break;
	case 1: 
		m_characterTextName->SetText("Iron Man");
		m_characterTextP_Info->SetText("Can Fly");
		m_characterTextS1_Info->SetText("Gain huge bonus attack");
		m_characterTextS2_Info->SetText("Drop multiple energy bombs");
		m_characterTextU_Info->SetText("Release a giant laser beam");
		break;
	case 2: 
		m_characterTextName->SetText("Adam Warlock");
		m_characterTextP_Info->SetText("Can revive after death");
		m_characterTextS1_Info->SetText("Gain attack bonus to the whole team");
		m_characterTextS2_Info->SetText("Throw healing orb which can overheal");
		m_characterTextU_Info->SetText("Resurrect dead teammates");
		break;
	case 3: 
		m_characterTextName->SetText("Invisible Woman");
		m_characterTextP_Info->SetText("Normal attack can heal teammates");
		m_characterTextS1_Info->SetText("Activate invisible status");
		m_characterTextS2_Info->SetText("Throw gravity orb that pulls enemies");
		m_characterTextU_Info->SetText("Create a healing area");
		break;
	case 4: 
		m_characterTextName->SetText("Thor");
		m_characterTextP_Info->SetText("Normal attack can return");
		m_characterTextS1_Info->SetText("Gain shield until next turn");
		m_characterTextS2_Info->SetText("Dash forward");
		m_characterTextU_Info->SetText("Find the closest enemy and slam at them");
		break;
	case 5: 
		m_characterTextName->SetText("Hulk");
		m_characterTextP_Info->SetText("Heal himself every turn");
		m_characterTextS1_Info->SetText("Activate jumping mode");
		m_characterTextS2_Info->SetText("Gain shield to him and nearby teammates");
		m_characterTextU_Info->SetText("Become angry and turn giant, heal himself");
		break;
	}
}

void Game::RemoveCharacterInfo(char characterID)
{
	m_characterButtonIsHoveringState[characterID] = false;
}

void Game::ToggleVisibilityCharacterInfo()
{
	if (!m_selectCharactersPanel->IsActive())
	{
		m_characterInfoPanel->SetActive(false);
		return;
	}
	if (m_currentSelectingPlayerTurn > 5)
	{
		m_characterInfoPanel->SetActive(false);
		return;
	}
	bool isOneSelected = false;
	for (size_t i = 0; i < 6; i++)
	{
		if (m_characterButtonIsHoveringState[i]) 
		{
			isOneSelected = true;
			break;
		}
	}

	m_characterInfoPanel->SetActive(isOneSelected);
}

//----------------------------------------------------------------------------------------------------------------------------------------
// GAME RESTART

void Game::GameRestart()
{
	m_initNumCharacters = 0;
	m_gameHasEnded = false;
	m_endGameWaitTimer->Stop();
	m_playerIDTurn = -1;
	m_turnHasPassed = 0;
	m_winPanel->SetActive(false);
	m_nextTurnPanel->SetActive(false);

	delete m_currentMap;

	for (auto& controller : m_controllers)
	{
		delete controller;
		controller = nullptr;
	}
	m_controllers.clear();
	m_controllerTookTurn.clear();

	m_currentMap = Map::GenerateRandomMap(this, 32, m_mapSeed);

	if (m_isDemo)
	{
		AddAI("Hela", 1);
		AddAI("Thor", 1);
		AddAI("Adam", 1);
		AddAI("Iron", 2);
		AddAI("Hulk", 2);
		AddAI("IW", 2);
	}
	else
	{
		for (size_t i = 0; i < 6; i++)
		{
			if (m_playerSelection[i] == -1)
			{
				continue;
			}

			if (i % 2 == 0)
			{
				if (m_playerSelection[i] == 0)
				{
					AddPlayer(m_characterSelection[i], 1);
				}
				if (m_playerSelection[i] == 1)
				{
					AddAI(m_characterSelection[i], 1);
				}
			}
			else
			{
				if (m_playerSelection[i] == 0)
				{
					AddPlayer(m_characterSelection[i], 2);
				}
				if (m_playerSelection[i] == 1)
				{
					AddAI(m_characterSelection[i], 2);
				}
			}
		}
	}

	PendingEndTurn();
}

bool Game::CheckIfGameEnded()
{
	bool isTeam1Alive = false;
	bool isTeam2Alive = false;
	for (auto& character : m_currentMap->m_characters)
	{
		if (character->m_health == -1) continue;

		if (!character->m_isDead)
		{
			if (character->GetTeamID() == 1)
			{
				isTeam1Alive = true;
			}
			else
			{
				isTeam2Alive = true;
			}
		}
	}

	if (isTeam1Alive && isTeam2Alive)
	{
		return false;
	}

	if (isTeam1Alive && !isTeam2Alive)
	{
		m_nextTurnPanel->SetActive(false);
		m_winPanel->SetActive(true);
		m_winPanel->SetColor(Rgba8::COLOR_BRIGHT_RED);
		m_TeamWinText->SetText("Team 1\nWin");

		int highestKillCount = -1;
		Character* mvp = nullptr;
		for (size_t i = 0; i < m_currentMap->m_characters.size(); i++)
		{
			if (m_currentMap->m_characters[i]->m_health == -1) continue;
			if (m_currentMap->m_characters[i]->m_killCount > highestKillCount)
			{
				highestKillCount = m_currentMap->m_characters[i]->m_killCount;
				mvp = m_currentMap->m_characters[i];
			}
		}
		if (mvp)
		{
			CharacterDefinition* def = mvp->m_characterDef;
			m_mvpNameText->SetText(def->m_name);
			m_MVP_UISprite->SetTexture(UIDefinitions::GetTexureByName("Character_" + def->m_name));
		}

		g_theDevConsole->AddLine(Rgba8::COLOR_YELLOW, "Team 1 Win");
		m_gameHasEnded = true;
		return true;
	}

	if (!isTeam1Alive && isTeam2Alive)
	{
		m_nextTurnPanel->SetActive(false);
		m_winPanel->SetActive(true);
		m_winPanel->SetColor(Rgba8::COLOR_BRIGHT_BLUE);
		m_TeamWinText->SetText("Team 2\nWin");
		g_theDevConsole->AddLine(Rgba8::COLOR_YELLOW, "Team 2 Win");

		int highestKillCount = -1;
		Character* mvp = nullptr;
		for (size_t i = 0; i < m_currentMap->m_characters.size(); i++)
		{
			if (m_currentMap->m_characters[i]->m_health == -1) continue;
			if (m_currentMap->m_characters[i]->m_killCount > highestKillCount)
			{
				highestKillCount = m_currentMap->m_characters[i]->m_killCount;
				mvp = m_currentMap->m_characters[i];
			}
		}
		if (mvp)
		{
			CharacterDefinition* def = mvp->m_characterDef;
			m_mvpNameText->SetText(def->m_name);
			m_MVP_UISprite->SetTexture(UIDefinitions::GetTexureByName("Character_" + def->m_name));
		}

		m_gameHasEnded = true;
		return true;
	}

	return false;
}

void Game::TurnAndCameraManaging(float deltaSecond)
{
	if (!m_currentMap->m_flyingProjectiles.empty())
	{
		return;
	}
	if (m_cameraMoveCooldown > 0.f)
	{
		return;
	}
	if (m_isPendingEndTurn && NextTurn())
	{
		m_timerNextTurnPanel = 2.f;
		m_nextTurnPanel->SetActive(true);
		CharacterDefinition* def = GetController(m_playerIDTurn)->m_character->m_characterDef;
		m_characterNameText->SetText(def->m_name);
		if (GetController(m_playerIDTurn)->m_teamID == 1)
		{
			m_characterNameText->SetColor(Rgba8::COLOR_BRIGHT_RED);
		}
		else
		{
			m_characterNameText->SetColor(Rgba8::COLOR_BRIGHT_BLUE);
		}
		m_characterUISprite->SetTexture(UIDefinitions::GetTexureByName("Character_" + def->m_name));
		m_isPendingEndTurn = false;
	}

	if (m_playerControlCamera) return;
	if (m_currentFocusCharacter)
	{
		GameCameraFollow(m_currentFocusCharacter->m_position, deltaSecond * 2.f);
	}

}

bool Game::Command_NextTurn(EventArgs& arg)
{
	UNUSED(arg);
	g_theApp->m_game->NextTurn();
	g_theDevConsole->AddLine(DevConsole::SUCCESS, "Next Turn");
	return false;
}

bool Game::Command_GameRestart(EventArgs& arg)
{
	UNUSED(arg);
	g_theApp->m_game->GameRestart();
	g_theDevConsole->AddLine(DevConsole::SUCCESS, "Game Restarted");
	return false;
}

bool Game::Command_ToggleDemo(EventArgs& arg)
{
	UNUSED(arg);
	g_theApp->m_game->m_isDemo = !g_theApp->m_game->m_isDemo;
	g_theApp->m_game->GameRestart();
	if (g_theApp->m_game->m_isDemo)
	{
		g_theDevConsole->AddLine(DevConsole::SUCCESS, "Demo Mode: ON");
	}
	else
	{
		g_theDevConsole->AddLine(DevConsole::SUCCESS, "Demo Mode: OFF");
	}
	return false;
}

bool Game::Command_ToggleDebugMode(EventArgs& arg)
{
	UNUSED(arg);

	g_theApp->m_game->m_isDebugging = !g_theApp->m_game->m_isDebugging;

	if (g_theApp->m_game->m_isDebugging)
	{
		g_theDevConsole->AddLine(DevConsole::SUCCESS, "Debugging Mode: ON");
	}
	else
	{
		g_theDevConsole->AddLine(DevConsole::SUCCESS, "Debugging Mode: OFF");
	}
	return false;
}

bool Game::Command_TestCase(EventArgs& arg)
{
	UNUSED(arg);
	if (!g_theApp->m_game->m_isDebugging)
	{
		g_theDevConsole->AddLine(DevConsole::ERROR, "Not In Debug Mode");
		return false;
	}
	std::string s = "Data/Commands.xml";
	g_theDevConsole->AddLine(DevConsole::SUCCESS, "Loaded \"" + s + "\" successfully");
	g_theDevConsole->ExecuteXmlCommandScriptFile(s);
	return false;
}

bool Game::Command_ToggleDebugDraw(EventArgs& arg)
{
	UNUSED(arg);

	g_debugDraw = !g_debugDraw;
	if (g_debugDraw)
	{
		g_theDevConsole->AddLine(DevConsole::SUCCESS, "Debug Draw: ON");
	}
	else
	{
		g_theDevConsole->AddLine(DevConsole::SUCCESS, "Debug Draw: OFF");
	}
	return false;
}

bool Game::Command_ToggleDestructible(EventArgs& arg)
{
	UNUSED(arg);
	if (!g_theApp->m_game->m_isDebugging)
	{
		g_theDevConsole->AddLine(DevConsole::ERROR, "Not In Debug Mode");
		return false;
	}
	g_debugDestructible = !g_debugDestructible;
	if (g_debugDestructible)
	{
		g_theDevConsole->AddLine(DevConsole::SUCCESS, "Destructible With Mouse: ON");
	}
	else
	{
		g_theDevConsole->AddLine(DevConsole::SUCCESS, "Destructible With Mouse: OFF");
	}
	return false;
}

bool Game::Command_ToggleMoveFast(EventArgs& arg)
{
	UNUSED(arg);
	if (!g_theApp->m_game->m_isDebugging)
	{
		g_theDevConsole->AddLine(DevConsole::ERROR, "Not In Debug Mode");
		return false;
	}
	g_debugCharacterMoveFast = !g_debugCharacterMoveFast;

	if (g_debugCharacterMoveFast)
	{
		g_theDevConsole->AddLine(DevConsole::SUCCESS, "Player Move Fast: ON");
	}
	else
	{
		g_theDevConsole->AddLine(DevConsole::SUCCESS, "Player Move Fast: OFF");
	}
	return false;
}

void Game::GameCameraCursorZoom(Vec2 cursor, float value)
{
	if (m_cameraMoveCooldown > -0.2f) return;
	if (m_secondIntoMode < 1.f) return;
	if (!m_currentMap->m_flyingProjectiles.empty() && m_currentMap->m_flyingProjectiles[0]) return;

	m_playerControlCamera = true;

	Vec2 cursorGameCamera = Vec2(RangeMapClamped(cursor.x, 0.f, 1600.f, 0.f, m_cameraView.m_maxs.x), RangeMapClamped(cursor.y, 0.f, 800.f, 0.f, m_cameraView.m_maxs.y));
	Vec2 oldViewSize = m_cameraView.m_maxs - m_cameraView.m_mins;
	Vec2 newViewSize = oldViewSize * value;
	Vec2 mapSize = m_currentMap->m_bound.m_maxs;

	// Prevent getting out of the map
	if (newViewSize.x > mapSize.x || newViewSize.y > mapSize.y)
	{
		return;
	}
	// Prevent zoom too close
	if (newViewSize.y < mapSize.y * 0.2f)
	{
		return;
	}

	Vec2 cursorRelative = (cursorGameCamera - m_cameraView.m_mins) / oldViewSize;
	Vec2 newMins = cursorGameCamera - (cursorRelative * newViewSize);
	Vec2 newMaxs = newMins + newViewSize;

	// Apply bounds checking
	if (newMins.x < 0.f)
	{
		newMins.x = 0.f;
		newMaxs.x = newViewSize.x;
	}
	if (newMins.y < 0.f)
	{
		newMins.y = 0.f;
		newMaxs.y = newViewSize.y;
	}
	if (newMaxs.x > m_currentMap->m_bound.m_maxs.x)
	{
		newMaxs.x = m_currentMap->m_bound.m_maxs.x;
		newMins.x = newMaxs.x - newViewSize.x;
	}
	if (newMaxs.y > m_currentMap->m_bound.m_maxs.y)
	{
		newMaxs.y = m_currentMap->m_bound.m_maxs.y;
		newMins.y = newMaxs.y - newViewSize.y;
	}

	m_cameraView.m_mins = newMins;
	m_cameraView.m_maxs = newMaxs;
}

void Game::GameCameraCursorMove(Vec2 cursor, float value)
{
	if (m_cameraMoveCooldown > -0.2f) return;
	if (m_secondIntoMode < 1.f) return;
	if (!m_currentMap->m_flyingProjectiles.empty() && m_currentMap->m_flyingProjectiles[0]) return;

	if (m_cursorPos.x < 100.f)
	{
		m_cameraView.m_mins += Vec2(-1, 0) * value;
		m_cameraView.m_maxs += Vec2(-1, 0) * value;

		m_playerControlCamera = true;
	}
	if (m_cursorPos.x > 1500.f)
	{
		m_cameraView.m_mins += Vec2(1, 0) * value;
		m_cameraView.m_maxs += Vec2(1, 0) * value;

		m_playerControlCamera = true;
	}
	if (m_cursorPos.y < 100.f)
	{
		m_cameraView.m_mins += Vec2(0, -1) * value;
		m_cameraView.m_maxs += Vec2(0, -1) * value;

		m_playerControlCamera = true;
	}
	if (m_cursorPos.y > 700.f)
	{
		m_cameraView.m_mins += Vec2(0, 1) * value;
		m_cameraView.m_maxs += Vec2(0, 1) * value;

		m_playerControlCamera = true;
	}

	Vec2 viewSize = m_cameraView.m_maxs - m_cameraView.m_mins;

	if (m_cameraView.m_mins.x < 0.f)
	{
		m_cameraView.m_mins.x = 0.f;
		m_cameraView.m_maxs.x = m_cameraView.m_mins.x + viewSize.x;
	}
	if (m_cameraView.m_mins.y < 0.f)
	{
		m_cameraView.m_mins.y = 0.f;
		m_cameraView.m_maxs.y = m_cameraView.m_mins.y + viewSize.y;
	}
	if (m_cameraView.m_maxs.x > m_currentMap->m_bound.m_maxs.x)
	{
		m_cameraView.m_maxs.x = m_currentMap->m_bound.m_maxs.x;
		m_cameraView.m_mins.x = m_cameraView.m_maxs.x - viewSize.x;
	}
	if (m_cameraView.m_maxs.y > m_currentMap->m_bound.m_maxs.y)
	{
		m_cameraView.m_maxs.y = m_currentMap->m_bound.m_maxs.y;
		m_cameraView.m_mins.y = m_cameraView.m_maxs.y - viewSize.y;
	}
}

void Game::GameCameraFollow(Vec2 target, float easingValue)
{
	Vec2 currentCenter = m_cameraView.GetCenter();
	Vec2 cameraHalfDims = m_cameraView.GetDimensions() * 0.5f;

	Vec2 toTarget = target - currentCenter;
	float distance = toTarget.GetLength();

	if (distance > 10.f)
	{
		Vec2 desiredCenter = currentCenter + toTarget.GetNormalized() * Clamp(distance, 0.f, 150.f) * easingValue;

		Vec2 mins = desiredCenter - cameraHalfDims;
		Vec2 maxs = desiredCenter + cameraHalfDims;

		m_cameraView.m_mins = mins;
		m_cameraView.m_maxs = maxs;
	}

	Vec2 viewSize = m_cameraView.m_maxs - m_cameraView.m_mins;

	if (m_cameraView.m_mins.x < 0.f)
	{
		m_cameraView.m_mins.x = 0.f;
		m_cameraView.m_maxs.x = m_cameraView.m_mins.x + viewSize.x;
	}
	if (m_cameraView.m_mins.y < 0.f)
	{
		m_cameraView.m_mins.y = 0.f;
		m_cameraView.m_maxs.y = m_cameraView.m_mins.y + viewSize.y;
	}
	if (m_cameraView.m_maxs.x > m_currentMap->m_bound.m_maxs.x)
	{
		m_cameraView.m_maxs.x = m_currentMap->m_bound.m_maxs.x;
		m_cameraView.m_mins.x = m_cameraView.m_maxs.x - viewSize.x;
	}
	if (m_cameraView.m_maxs.y > m_currentMap->m_bound.m_maxs.y)
	{
		m_cameraView.m_maxs.y = m_currentMap->m_bound.m_maxs.y;
		m_cameraView.m_mins.y = m_cameraView.m_maxs.y - viewSize.y;
	}
}

void Game::SetCooldownCamera(float time)
{
	m_cameraMoveCooldown = time;
}

void Game::MainMenuUI_Init()
{
	IntVec2 screenSize = Window::GetMainWindowInstance()->GetClientDimensions();
	float aspect = Window::GetMainWindowInstance()->GetAspect();

	m_menuMainPanel = new Panel(m_menuCanvas, false);

	TextSetting titleTextSetting1 = TextSetting("Marvel's Rival");
	titleTextSetting1.m_color = Rgba8::COLOR_WHITE;
	titleTextSetting1.m_height = 64.f * aspect;
	Vec2 titlePos = Vec2(screenSize.x * 0.5f, screenSize.y * 0.85f);
	m_menuTitleMain = new Text(m_menuCanvas, titlePos, titleTextSetting1, m_menuMainPanel);

	TextSetting titleTextSetting2 = TextSetting("by Son Nguyen");
	titleTextSetting2.m_color = Rgba8::COLOR_GREEN;
	titleTextSetting2.m_height = 20.f * aspect;
	m_menuTitleSub = new Text(m_menuCanvas, titlePos + Vec2(0, -150), titleTextSetting2, m_menuMainPanel);

	TextSetting playTS = TextSetting("Play");
	playTS.m_color = Rgba8::COLOR_WHITE;
	playTS.m_height = 22.f * aspect;
	m_menuPlayButton = new Button(m_menuCanvas, AABB2(screenSize.x * 0.5f - 200, 400, screenSize.x * 0.5f + 200, 490), playTS, Rgba8::COLOR_DARK_GRAY, Rgba8::COLOR_DARK_GREEN, Rgba8::COLOR_WHITE, true, Rgba8::COLOR_BLACK, m_menuMainPanel);
	m_menuPlayButton->m_textColorHover = Rgba8::COLOR_WHITE;
	m_menuPlayButton->OnClickEvent([this]() { 
		if (m_audioManager->AreAllSoundsLoaded())
		{
			SwitchState(GameState::SELECT_CHARACTERS_MODE);
		}
		});

	TextSetting htpTS = TextSetting("How To Play");
	htpTS.m_color = Rgba8::COLOR_WHITE;
	htpTS.m_height = 22.f * aspect;
	m_menuHTPButton = new Button(m_menuCanvas, AABB2(screenSize.x * 0.5f - 200, 300, screenSize.x * 0.5f + 200, 390), htpTS, Rgba8::COLOR_DARK_GRAY, Rgba8::COLOR_DARK_GREEN, Rgba8::COLOR_WHITE, true, Rgba8::COLOR_BLACK, m_menuMainPanel);
	m_menuHTPButton->m_textColorHover = Rgba8::COLOR_WHITE;
	m_menuHTPButton->OnClickEvent([this]() {OpenHowToPlay(); });

	TextSetting settingTS = TextSetting("Settings");
	settingTS.m_color = Rgba8::COLOR_WHITE;
	settingTS.m_height = 22.f * aspect;
	m_menuSettingButton = new Button(m_menuCanvas, AABB2(screenSize.x * 0.5f - 200, 200, screenSize.x * 0.5f + 200, 290), settingTS, Rgba8::COLOR_DARK_GRAY, Rgba8::COLOR_DARK_GREEN, Rgba8::COLOR_WHITE, true, Rgba8::COLOR_BLACK, m_menuMainPanel);
	m_menuSettingButton->m_textColorHover = Rgba8::COLOR_WHITE;
	m_menuSettingButton->OnClickEvent([this]() {OpenSetting(); });

	TextSetting quitTS = TextSetting("Quit");
	quitTS.m_color = Rgba8::COLOR_WHITE;
	quitTS.m_height = 22.f * aspect;
	m_menuQuitButton = new Button(m_menuCanvas, AABB2(screenSize.x * 0.5f - 200, 100, screenSize.x * 0.5f + 200, 190), quitTS, Rgba8::COLOR_DARK_GRAY, Rgba8::COLOR_DARK_GREEN, Rgba8::COLOR_WHITE, true, Rgba8::COLOR_BLACK, m_menuMainPanel);
	m_menuQuitButton->m_textColorHover = Rgba8::COLOR_WHITE;
	m_menuQuitButton->OnClickEvent([this]() {FireEvent("quit"); });

	m_settingMainPanel = new Panel(m_menuCanvas, false);
	m_settingMainPanel->SetActive(false);

	TextSetting titleSettingTS = TextSetting("Settings");
	titleSettingTS.m_color = Rgba8::COLOR_WHITE;
	titleSettingTS.m_height = 64.f * aspect;
	m_settingTitle = new Text(m_menuCanvas, Vec2(screenSize.x * 0.5f, screenSize.y * 0.85f), titleSettingTS, m_settingMainPanel);

	m_settingMusicText = new Text(m_menuCanvas, Vec2(screenSize.x * 0.4f, 295), TextSetting("Music", 20.f, Rgba8::COLOR_WHITE), m_settingMainPanel);
	m_settingSoundText = new Text(m_menuCanvas, Vec2(screenSize.x * 0.4f, 245), TextSetting("Sound", 20.f, Rgba8::COLOR_WHITE), m_settingMainPanel);

	m_settingMusicSlider = new Slider(m_menuCanvas, 0.f, 1.f, AABB2(screenSize.x * 0.5f - 100, 285, screenSize.x * 0.5f + 150, 305), Vec2(15, 15), m_settingMainPanel);
	m_settingMusicSlider->m_knobColorHover = Rgba8::COLOR_DARK_RED;
	m_settingMusicSlider->m_knobColorUnhover = Rgba8::COLOR_RED;
	m_settingMusicSlider->m_knobColorPressed = Rgba8::COLOR_GREEN;
	m_settingMusicSlider->SetValue(1.0f);
	m_settingMusicSlider->OnChangingValueEvent([this]() {
		m_audioManager->m_musicVolume = m_settingMusicSlider->GetValue();
		g_theAudio->SetSoundPlaybackVolume(m_currentMusic, m_settingMusicSlider->GetValue());
		});

	m_settingSoundSlider = new Slider(m_menuCanvas, 0.f, 1.f, AABB2(screenSize.x * 0.5f - 100, 235, screenSize.x * 0.5f + 150, 255), Vec2(15, 15), m_settingMainPanel);
	m_settingSoundSlider->m_knobColorHover = Rgba8::COLOR_DARK_RED;
	m_settingSoundSlider->m_knobColorUnhover = Rgba8::COLOR_RED;
	m_settingSoundSlider->m_knobColorPressed = Rgba8::COLOR_GREEN;
	m_settingSoundSlider->SetValue(1.0f);
	m_settingSoundSlider->OnChangingValueEvent([this]() {m_audioManager->m_sfxVolume = m_settingSoundSlider->GetValue(); });

	TextSetting backTS = TextSetting("Back To Menu");
	backTS.m_color = Rgba8::COLOR_WHITE;
	backTS.m_height = 22.f * aspect;
	m_menuQuitButton = new Button(m_menuCanvas, AABB2(screenSize.x * 0.5f - 200, 100, screenSize.x * 0.5f + 200, 190), backTS, Rgba8::COLOR_DARK_GRAY, Rgba8::COLOR_DARK_GREEN, Rgba8::COLOR_WHITE, true, Rgba8::COLOR_BLACK, m_settingMainPanel);
	m_menuQuitButton->m_textColorHover = Rgba8::COLOR_WHITE;
	m_menuQuitButton->OnClickEvent([this]() {OpenMenuOptions(); });

	m_howToPlayPanel = new Panel(m_menuCanvas, false);
	m_howToPlayPanel->SetActive(false);

	TextSetting titleHTPTS = TextSetting("How To Play");
	titleHTPTS.m_color = Rgba8::COLOR_WHITE;
	titleHTPTS.m_height = 64.f * aspect;
	m_htpTitle = new Text(m_menuCanvas, Vec2(screenSize.x * 0.5f, screenSize.y * 0.85f), titleHTPTS, m_howToPlayPanel);

	TextSetting htp1TS = TextSetting("A/D to move left and right\nQ/E to fly up and down (if your character can fly)");
	htp1TS.m_color = Rgba8::COLOR_WHITE;
	htp1TS.m_height = 22.f * aspect;
	m_htpText1 = new Text(m_menuCanvas, Vec2(screenSize.x * 0.5f, screenSize.y * 0.65f), htp1TS, m_howToPlayPanel);

	TextSetting htp2TS = TextSetting("W/S to change the shooting angle");
	htp2TS.m_color = Rgba8::COLOR_WHITE;
	htp2TS.m_height = 22.f * aspect;
	m_htpText2 = new Text(m_menuCanvas, Vec2(screenSize.x * 0.5f, screenSize.y * 0.5f), htp2TS, m_howToPlayPanel);

	TextSetting htp3TS = TextSetting("Hold SPACE to adjust the force and release to shoot");
	htp3TS.m_color = Rgba8::COLOR_WHITE;
	htp3TS.m_height = 22.f * aspect;
	m_htpText3 = new Text(m_menuCanvas, Vec2(screenSize.x * 0.5f, screenSize.y * 0.4f), htp3TS, m_howToPlayPanel);

	TextSetting htp4TS = TextSetting("1, 2, R to use character's skills");
	htp4TS.m_color = Rgba8::COLOR_WHITE;
	htp4TS.m_height = 22.f * aspect;
	m_htpText4 = new Text(m_menuCanvas, Vec2(screenSize.x * 0.5f, screenSize.y * 0.3f), htp4TS, m_howToPlayPanel);

	m_htpBackButton = new Button(m_menuCanvas, AABB2(screenSize.x * 0.5f - 200, 100, screenSize.x * 0.5f + 200, 190), backTS, Rgba8::COLOR_DARK_GRAY, Rgba8::COLOR_DARK_GREEN, Rgba8::COLOR_WHITE, true, Rgba8::COLOR_BLACK, m_howToPlayPanel);
	m_htpBackButton->m_textColorHover = Rgba8::COLOR_WHITE;
	m_htpBackButton->OnClickEvent([this]() {OpenMenuOptions(); });
}

void Game::SelectCharacterUI_Init()
{
	IntVec2 screenSize = Window::GetMainWindowInstance()->GetClientDimensions();
	float aspect = Window::GetMainWindowInstance()->GetAspect();

	m_selectTeamPanel = new Panel(m_characterCanvas, false);

	TextSetting titleTextSetting = TextSetting("Team Selection");
	titleTextSetting.m_color = Rgba8::COLOR_WHITE;
	titleTextSetting.m_height = 64.f * aspect;
	Vec2 titlePos = Vec2(screenSize.x * 0.5f, screenSize.y * 0.85f);
	m_selectTeamTitle = new Text(m_characterCanvas, titlePos, titleTextSetting, m_selectTeamPanel);

	TextSetting team1Setting = TextSetting("Team 1");
	team1Setting.m_color = Rgba8::COLOR_RED;
	team1Setting.m_height = 30.f * aspect;
	m_selectTeam1Title = new Text(m_characterCanvas, Vec2(screenSize.x * 0.2f, screenSize.y * 0.7f), team1Setting);

	TextSetting team2Setting = TextSetting("Team 2");
	team2Setting.m_color = Rgba8::COLOR_BRIGHT_BLUE;
	team2Setting.m_height = 30.f * aspect;
	m_selectTeam2Title = new Text(m_characterCanvas, Vec2(screenSize.x * 0.8f, screenSize.y * 0.7f), team2Setting);

	TextSetting vsSetting = TextSetting("V.S");
	vsSetting.m_color = Rgba8::COLOR_GREEN;
	vsSetting.m_height = 32.f * aspect;
	m_selectVS = new Text(m_characterCanvas, Vec2(screenSize.x * 0.5f, screenSize.y * 0.5f), vsSetting);

	TextSetting teamSelectButtonTS = TextSetting("Confirm");
	teamSelectButtonTS.m_color = Rgba8::COLOR_WHITE;
	teamSelectButtonTS.m_height = 22.f * aspect;
	m_selectTeamConfirmButton = new Button(m_characterCanvas, AABB2(screenSize.x * 0.5f - 150, screenSize.y * 0.08f - 40, screenSize.x * 0.5f + 150, screenSize.y * 0.08f + 40), teamSelectButtonTS, Rgba8::COLOR_DARK_GRAY, Rgba8::COLOR_DARK_GREEN, Rgba8::COLOR_WHITE, true, Rgba8::COLOR_BLACK, m_selectTeamPanel);
	m_selectTeamConfirmButton->m_textColorHover = Rgba8::COLOR_WHITE;
	m_selectTeamConfirmButton->OnClickEvent([this]() {OpenCharacterSelect(); });

	m_playersButton[0] = new Button(m_characterCanvas, AABB2(screenSize.x * 0.2f - 70, screenSize.y * 0.55f - 70, screenSize.x * 0.2f + 70, screenSize.y * 0.55f + 70), true, Rgba8::COLOR_WHITE);
	m_playersButton[0]->m_textureUnhover = UIDefinitions::GetTexureByName("Select_Add");
	m_playersButton[0]->m_textureHover = UIDefinitions::GetTexureByName("Select_Add");
	m_playersButton[0]->m_texturePressed = UIDefinitions::GetTexureByName("Select_Add");
	m_playersButton[0]->OnClickEvent([this]() {SetButtonPlayer(0); });

	m_playersButton[2] = new Button(m_characterCanvas, AABB2(screenSize.x * 0.2f - 70, screenSize.y * 0.35f - 70, screenSize.x * 0.2f + 70, screenSize.y * 0.35f + 70), true, Rgba8::COLOR_WHITE);
	m_playersButton[2]->m_textureUnhover = UIDefinitions::GetTexureByName("Select_Add");
	m_playersButton[2]->m_textureHover = UIDefinitions::GetTexureByName("Select_Add");
	m_playersButton[2]->m_texturePressed = UIDefinitions::GetTexureByName("Select_Add");
	m_playersButton[2]->OnClickEvent([this]() {SetButtonPlayer(2); });

	m_playersButton[4] = new Button(m_characterCanvas, AABB2(screenSize.x * 0.2f - 70, screenSize.y * 0.15f - 70, screenSize.x * 0.2f + 70, screenSize.y * 0.15f + 70), true, Rgba8::COLOR_WHITE);
	m_playersButton[4]->m_textureUnhover = UIDefinitions::GetTexureByName("Select_Add");
	m_playersButton[4]->m_textureHover = UIDefinitions::GetTexureByName("Select_Add");
	m_playersButton[4]->m_texturePressed = UIDefinitions::GetTexureByName("Select_Add");
	m_playersButton[4]->OnClickEvent([this]() {SetButtonPlayer(4); });

	m_playersButton[1] = new Button(m_characterCanvas, AABB2(screenSize.x * 0.8f - 70, screenSize.y * 0.55f - 70, screenSize.x * 0.8f + 70, screenSize.y * 0.55f + 70), true, Rgba8::COLOR_WHITE);
	m_playersButton[1]->m_textureUnhover = UIDefinitions::GetTexureByName("Select_Add");
	m_playersButton[1]->m_textureHover = UIDefinitions::GetTexureByName("Select_Add");
	m_playersButton[1]->m_texturePressed = UIDefinitions::GetTexureByName("Select_Add");
	m_playersButton[1]->OnClickEvent([this]() {SetButtonPlayer(1); });

	m_playersButton[3] = new Button(m_characterCanvas, AABB2(screenSize.x * 0.8f - 70, screenSize.y * 0.35f - 70, screenSize.x * 0.8f + 70, screenSize.y * 0.35f + 70), true, Rgba8::COLOR_WHITE);
	m_playersButton[3]->m_textureUnhover = UIDefinitions::GetTexureByName("Select_Add");
	m_playersButton[3]->m_textureHover = UIDefinitions::GetTexureByName("Select_Add");
	m_playersButton[3]->m_texturePressed = UIDefinitions::GetTexureByName("Select_Add");
	m_playersButton[3]->OnClickEvent([this]() {SetButtonPlayer(3); });

	m_playersButton[5] = new Button(m_characterCanvas, AABB2(screenSize.x * 0.8f - 70, screenSize.y * 0.15f - 70, screenSize.x * 0.8f + 70, screenSize.y * 0.15f + 70), true, Rgba8::COLOR_WHITE);
	m_playersButton[5]->m_textureUnhover = UIDefinitions::GetTexureByName("Select_Add");
	m_playersButton[5]->m_textureHover = UIDefinitions::GetTexureByName("Select_Add");
	m_playersButton[5]->m_texturePressed = UIDefinitions::GetTexureByName("Select_Add");
	m_playersButton[5]->OnClickEvent([this]() {SetButtonPlayer(5); });


	m_selectCharactersPanel = new Panel(m_characterCanvas, false);
	m_selectCharactersPanel->SetActive(false);

	TextSetting titleTextSetting1 = TextSetting("Character Selection");
	titleTextSetting1.m_color = Rgba8::COLOR_WHITE;
	titleTextSetting1.m_height = titleTextSetting.m_height = 80.f * aspect;
	m_selectTeamTitle = new Text(m_menuCanvas, titlePos, titleTextSetting1, m_selectCharactersPanel);

	TextSetting playButtonTS = TextSetting("Play");
	playButtonTS.m_color = Rgba8::COLOR_WHITE;
	playButtonTS.m_height = 22.f * aspect;
	m_playGameButton = new Button(m_characterCanvas, AABB2(screenSize.x * 0.5f - 150, screenSize.y * 0.08f - 40, screenSize.x * 0.5f + 150, screenSize.y * 0.08f + 40), playButtonTS, Rgba8::COLOR_DARK_GRAY, Rgba8::COLOR_DARK_GREEN, Rgba8::COLOR_WHITE, true, Rgba8::COLOR_BLACK, m_selectCharactersPanel);
	m_playGameButton->m_textColorHover = Rgba8::COLOR_WHITE;
	m_playGameButton->OnClickEvent([this]() {PlayGame(); });

	TextSetting seedButtonTS = TextSetting("Random Map Seed");
	seedButtonTS.m_color = Rgba8::COLOR_WHITE;
	seedButtonTS.m_height = 17.f * aspect;
	m_randomSeedMapButton = new Button(m_characterCanvas, AABB2(screenSize.x * 0.5f - 150, screenSize.y * 0.28f - 40, screenSize.x * 0.5f + 150, screenSize.y * 0.28f + 40), seedButtonTS, Rgba8::COLOR_DARK_GRAY, Rgba8::COLOR_DARK_GREEN, Rgba8::COLOR_WHITE, true, Rgba8::COLOR_BLACK, nullptr);
	m_randomSeedMapButton->m_textColorHover = Rgba8::COLOR_WHITE;
	m_randomSeedMapButton->OnClickEvent([this]() {RandomSeed(); });
	m_randomSeedMapButton->SetActive(false);

	TextSetting seedText = TextSetting(Stringf("Map Seed : % i", m_mapSeed));
	seedText.m_color = Rgba8::COLOR_WHITE;
	seedText.m_height = 20.f * aspect;
	m_seedMapText = new Text(m_menuCanvas, Vec2(screenSize.x * 0.5f, screenSize.y * 0.18f), seedText, m_randomSeedMapButton);

	m_charactersButton[0] = new Button(m_characterCanvas, AABB2(screenSize.x * 0.4f - 70, screenSize.y * 0.65f - 70, screenSize.x * 0.4f + 70, screenSize.y * 0.65f + 70), true, Rgba8::COLOR_WHITE, m_selectCharactersPanel);
	m_charactersButton[0]->m_textureUnhover = UIDefinitions::GetTexureByName("Character_Hela");
	m_charactersButton[0]->m_textureHover = UIDefinitions::GetTexureByName("Character_Hela");
	m_charactersButton[0]->m_texturePressed = UIDefinitions::GetTexureByName("Character_Hela");
	m_charactersButton[0]->OnClickEvent([this]() {SetButtonCharacter(0); });
	m_charactersButton[0]->OnHoverEvent([this]() {ShowButtonCharacterInfo(0); });
	m_charactersButton[0]->OnUnhoverEvent([this]() {RemoveCharacterInfo(0); });

	m_charactersButton[1] = new Button(m_characterCanvas, AABB2(screenSize.x * 0.4f - 70, screenSize.y * 0.45f - 70, screenSize.x * 0.4f + 70, screenSize.y * 0.45f + 70), true, Rgba8::COLOR_WHITE, m_selectCharactersPanel);
	m_charactersButton[1]->m_textureUnhover = UIDefinitions::GetTexureByName("Character_Iron");
	m_charactersButton[1]->m_textureHover = UIDefinitions::GetTexureByName("Character_Iron");
	m_charactersButton[1]->m_texturePressed = UIDefinitions::GetTexureByName("Character_Iron");
	m_charactersButton[1]->OnClickEvent([this]() {SetButtonCharacter(1); });
	m_charactersButton[1]->OnHoverEvent([this]() {ShowButtonCharacterInfo(1); });
	m_charactersButton[1]->OnUnhoverEvent([this]() {RemoveCharacterInfo(1); });

	m_charactersButton[2] = new Button(m_characterCanvas, AABB2(screenSize.x * 0.5f - 70, screenSize.y * 0.65f - 70, screenSize.x * 0.5f + 70, screenSize.y * 0.65f + 70), true, Rgba8::COLOR_WHITE, m_selectCharactersPanel);
	m_charactersButton[2]->m_textureUnhover = UIDefinitions::GetTexureByName("Character_Adam");
	m_charactersButton[2]->m_textureHover = UIDefinitions::GetTexureByName("Character_Adam");
	m_charactersButton[2]->m_texturePressed = UIDefinitions::GetTexureByName("Character_Adam");
	m_charactersButton[2]->OnClickEvent([this]() {SetButtonCharacter(2); });
	m_charactersButton[2]->OnHoverEvent([this]() {ShowButtonCharacterInfo(2); });
	m_charactersButton[2]->OnUnhoverEvent([this]() {RemoveCharacterInfo(2); });

	m_charactersButton[3] = new Button(m_characterCanvas, AABB2(screenSize.x * 0.5f - 70, screenSize.y * 0.45f - 70, screenSize.x * 0.5f + 70, screenSize.y * 0.45f + 70), true, Rgba8::COLOR_WHITE, m_selectCharactersPanel);
	m_charactersButton[3]->m_textureUnhover = UIDefinitions::GetTexureByName("Character_IW");
	m_charactersButton[3]->m_textureHover = UIDefinitions::GetTexureByName("Character_IW");
	m_charactersButton[3]->m_texturePressed = UIDefinitions::GetTexureByName("Character_IW");
	m_charactersButton[3]->OnClickEvent([this]() {SetButtonCharacter(3); });
	m_charactersButton[3]->OnHoverEvent([this]() {ShowButtonCharacterInfo(3); });
	m_charactersButton[3]->OnUnhoverEvent([this]() {RemoveCharacterInfo(3); });

	m_charactersButton[4] = new Button(m_characterCanvas, AABB2(screenSize.x * 0.6f - 70, screenSize.y * 0.65f - 70, screenSize.x * 0.6f + 70, screenSize.y * 0.65f + 70), true, Rgba8::COLOR_WHITE, m_selectCharactersPanel);
	m_charactersButton[4]->m_textureUnhover = UIDefinitions::GetTexureByName("Character_Thor");
	m_charactersButton[4]->m_textureHover = UIDefinitions::GetTexureByName("Character_Thor");
	m_charactersButton[4]->m_texturePressed = UIDefinitions::GetTexureByName("Character_Thor");
	m_charactersButton[4]->OnClickEvent([this]() {SetButtonCharacter(4); });
	m_charactersButton[4]->OnHoverEvent([this]() {ShowButtonCharacterInfo(4); });
	m_charactersButton[4]->OnUnhoverEvent([this]() {RemoveCharacterInfo(4); });

	m_charactersButton[5] = new Button(m_characterCanvas, AABB2(screenSize.x * 0.6f - 70, screenSize.y * 0.45f - 70, screenSize.x * 0.6f + 70, screenSize.y * 0.45f + 70), true, Rgba8::COLOR_WHITE, m_selectCharactersPanel);
	m_charactersButton[5]->m_textureUnhover = UIDefinitions::GetTexureByName("Character_Hulk");
	m_charactersButton[5]->m_textureHover = UIDefinitions::GetTexureByName("Character_Hulk");
	m_charactersButton[5]->m_texturePressed = UIDefinitions::GetTexureByName("Character_Hulk");
	m_charactersButton[5]->OnClickEvent([this]() {SetButtonCharacter(5); });
	m_charactersButton[5]->OnHoverEvent([this]() {ShowButtonCharacterInfo(5); });
	m_charactersButton[5]->OnUnhoverEvent([this]() {RemoveCharacterInfo(5); });

	m_characterInfoPanel = new Panel(m_characterCanvas, false);
	m_characterInfoPanel->SetActive(false);

	TextSetting info1TS = TextSetting("Name");
	info1TS.m_color = Rgba8::COLOR_WHITE;
	info1TS.m_height = 35.f * aspect;
	m_characterTextName = new Text(m_menuCanvas, Vec2(screenSize.x * 0.5f, screenSize.y * 0.27f), info1TS, m_characterInfoPanel);

	TextSetting info2TS = TextSetting("Passive:");
	info2TS.m_color = Rgba8::COLOR_WHITE;
	info2TS.m_height = 15.f * aspect;
	info2TS.m_alignment = Vec2(1.f, 0.5f);
	m_characterTextP = new Text(m_menuCanvas, Vec2(screenSize.x * 0.37f, screenSize.y * 0.21f), info2TS, m_characterInfoPanel);

	TextSetting info3TS = TextSetting("Skill 1:");
	info3TS.m_color = Rgba8::COLOR_WHITE;
	info3TS.m_height = 15.f * aspect;
	info3TS.m_alignment = Vec2(1.f, 0.5f);
	m_characterTextS1 = new Text(m_menuCanvas, Vec2(screenSize.x * 0.37f, screenSize.y * 0.16f), info3TS, m_characterInfoPanel);

	TextSetting info4TS = TextSetting("Skill 2:");
	info4TS.m_color = Rgba8::COLOR_WHITE;
	info4TS.m_height = 15.f * aspect;
	info4TS.m_alignment = Vec2(1.f, 0.5f);
	m_characterTextS2 = new Text(m_menuCanvas, Vec2(screenSize.x * 0.37f, screenSize.y * 0.11f), info4TS, m_characterInfoPanel);

	TextSetting info5TS = TextSetting("Ultimate:");
	info5TS.m_color = Rgba8::COLOR_WHITE;
	info5TS.m_height = 15.f * aspect;
	info5TS.m_alignment = Vec2(1.f, 0.5f);
	m_characterTextU = new Text(m_menuCanvas, Vec2(screenSize.x * 0.37f, screenSize.y * 0.06f), info5TS, m_characterInfoPanel);


	TextSetting info2InfoTS = TextSetting("Passive description");
	info2InfoTS.m_color = Rgba8::COLOR_WHITE;
	info2InfoTS.m_height = 15.f * aspect;
	info2InfoTS.m_alignment = Vec2(0.f, 0.5f);
	m_characterTextP_Info = new Text(m_menuCanvas, Vec2(screenSize.x * 0.38f, screenSize.y * 0.21f), info2InfoTS, m_characterInfoPanel);

	TextSetting info3InfoTS = TextSetting("Skill 1 description");
	info3InfoTS.m_color = Rgba8::COLOR_WHITE;
	info3InfoTS.m_height = 15.f * aspect;
	info3InfoTS.m_alignment = Vec2(0.f, 0.5f);
	m_characterTextS1_Info = new Text(m_menuCanvas, Vec2(screenSize.x * 0.38f, screenSize.y * 0.16f), info3InfoTS, m_characterInfoPanel);

	TextSetting info4InfoTS = TextSetting("Skill 2 description");
	info4InfoTS.m_color = Rgba8::COLOR_WHITE;
	info4InfoTS.m_height = 15.f * aspect;
	info4InfoTS.m_alignment = Vec2(0.f, 0.5f);
	m_characterTextS2_Info = new Text(m_menuCanvas, Vec2(screenSize.x * 0.38f, screenSize.y * 0.11f), info4InfoTS, m_characterInfoPanel);

	TextSetting info5InfoTS = TextSetting("Ultimate description");
	info5InfoTS.m_color = Rgba8::COLOR_WHITE;
	info5InfoTS.m_height = 15.f * aspect;
	info5InfoTS.m_alignment = Vec2(0.f, 0.5f);
	m_characterTextU_Info = new Text(m_menuCanvas, Vec2(screenSize.x * 0.38f, screenSize.y * 0.06f), info5InfoTS, m_characterInfoPanel);
}

void Game::InGameUI_Init()
{
	IntVec2 screenSize = Window::GetMainWindowInstance()->GetClientDimensions();
	float aspect = Window::GetMainWindowInstance()->GetAspect();

	m_nextTurnPanel = new Panel(m_inGameCanvas, AABB2(screenSize.x * 0.25f, screenSize.y * 0.4f, screenSize.x * 0.75f, screenSize.y * 0.75f), Rgba8(255, 255, 255, 50), true, Rgba8::COLOR_BLACK);
	m_nextTurnPanel->SetActive(false);

	m_characterUISprite = new Sprite(m_inGameCanvas, AABB2(screenSize.x * 0.28f, screenSize.y * 0.45f, screenSize.x * 0.28f + 200.f, screenSize.y * 0.45f + 200.f), nullptr, m_nextTurnPanel);

	TextSetting ts1 = TextSetting("Your Turn");
	ts1.m_color = Rgba8::COLOR_WHITE;
	ts1.m_height = ts1.m_height = 55.f * aspect;
	m_yourTurnText = new Text(m_menuCanvas, Vec2(screenSize.x * 0.57f, screenSize.y * 0.6f), ts1, m_nextTurnPanel);

	TextSetting ts2 = TextSetting("Hela");
	ts2.m_color = Rgba8::COLOR_RED;
	ts2.m_height = ts2.m_height = 45.f * aspect;
	m_characterNameText = new Text(m_menuCanvas, Vec2(screenSize.x * 0.57f, screenSize.y * 0.57f - 90.f), ts2, m_nextTurnPanel);

	m_winPanel = new Panel(m_inGameCanvas, AABB2(screenSize.x * 0.25f, screenSize.y * 0.4f, screenSize.x * 0.75f, screenSize.y * 0.75f), Rgba8::COLOR_BRIGHT_RED, true, Rgba8::COLOR_BLACK);
	m_winPanel->SetActive(false);

	TextSetting ws1 = TextSetting("Team X\nWon");
	ws1.m_color = Rgba8::COLOR_WHITE;
	ws1.m_height = ws1.m_height = 55.f * aspect;
	m_TeamWinText = new Text(m_menuCanvas, Vec2(screenSize.x * 0.57f, screenSize.y * 0.6f), ws1, m_winPanel);

	m_MVP_UISprite = new Sprite(m_inGameCanvas, AABB2(screenSize.x * 0.28f, screenSize.y * 0.48f, screenSize.x * 0.28f + 200.f, screenSize.y * 0.48f + 200.f), nullptr, m_winPanel);

	TextSetting ws2 = TextSetting("MVP");
	ws2.m_color = Rgba8::COLOR_YELLOW;
	ws2.m_height = ws2.m_height = 35.f * aspect;
	m_mvpText = new Text(m_menuCanvas, Vec2(screenSize.x * 0.28f + 100.f, screenSize.y * 0.7f), ws2, m_winPanel);

	TextSetting ws3 = TextSetting("Hela");
	ws3.m_color = Rgba8::COLOR_BRIGHT_WHITE;
	ws3.m_height = ws3.m_height = 25.f * aspect;
	m_mvpNameText = new Text(m_menuCanvas, Vec2(screenSize.x * 0.28f + 100.f, screenSize.y * 0.435f), ws3, m_winPanel);
}

PlayerController* Game::AddPlayer(std::string characterName, char teamID)
{
	CharacterDefinition* def = CharacterDefinition::GetByName(characterName);

	int minChunkIndex = 3;
	int maxChunkIndex = m_currentMap->GetNumChunks() - 3;
	IntRange chunkTeamRange = (teamID == 1) ? IntRange(minChunkIndex, maxChunkIndex / 2) : IntRange(maxChunkIndex / 2, maxChunkIndex);

	int halfRange = (chunkTeamRange.m_max - chunkTeamRange.m_min) / 2;

	if (def->m_isMelee)
	{
		if (teamID == 1)
		{
			chunkTeamRange.m_min += halfRange;
		}
		else
		{
			chunkTeamRange.m_max -= halfRange;
		}
	}
	else
	{
		if (teamID == 1)
		{
			chunkTeamRange.m_max -= halfRange;
		}
		else
		{
			chunkTeamRange.m_min += halfRange;
		}
	}


	int randomChunkID = g_theRNG->RollRandomIntInRange(chunkTeamRange.m_min, chunkTeamRange.m_max);
	int numTry = 0;
	while (!m_currentMap->CanThisChunkSpawnCharacter(randomChunkID))
	{
		randomChunkID = g_theRNG->RollRandomIntInRange(chunkTeamRange.m_min, chunkTeamRange.m_max);
		numTry++;

		if (numTry > 5) break;
	}
	int spawnXCoord = g_theRNG->RollRandomIntInRange(0, CHUNK_SIZE_X - 1);
	int spawnYCoord = m_currentMap->GetChunkAtCoord(randomChunkID)->m_terrainHeightList[spawnXCoord];

	Vec2 spawnPosition = Vec2((float)spawnXCoord + (float)randomChunkID * CHUNK_SIZE_X + 0.5f, (float)spawnYCoord + def->m_visualHalfLength.y + 1.f);

	Character* characterPtr = m_currentMap->SpawnCharacter(def, spawnPosition);
	auto newPlayer = new PlayerController(this, characterPtr, (char)m_controllers.size(), teamID);
	m_controllers.push_back(newPlayer);
	m_initNumCharacters++;
	return newPlayer;
}

AIController* Game::AddAI(std::string characterName, char teamID)
{
	CharacterDefinition* def = CharacterDefinition::GetByName(characterName);

	IntRange chunkTeamRange = (teamID == 1) ? IntRange(0, m_currentMap->GetNumChunks() / 2) : IntRange(m_currentMap->GetNumChunks() / 2, m_currentMap->GetNumChunks() - 1);

	int halfRange = (chunkTeamRange.m_max - chunkTeamRange.m_min) / 2;

	if (def->m_isMelee)
	{
		if (teamID == 1)
		{
			chunkTeamRange.m_min += halfRange;
		}
		else
		{
			chunkTeamRange.m_max -= halfRange;
		}
	}
	else
	{
		if (teamID == 1)
		{
			chunkTeamRange.m_max -= halfRange;
		}
		else
		{
			chunkTeamRange.m_min += halfRange;
		}
	}


	int randomChunkID = g_theRNG->RollRandomIntInRange(chunkTeamRange.m_min, chunkTeamRange.m_max);
	int numTry = 0;
	while (!m_currentMap->CanThisChunkSpawnCharacter(randomChunkID))
	{
		randomChunkID = g_theRNG->RollRandomIntInRange(chunkTeamRange.m_min, chunkTeamRange.m_max);
		numTry++;

		if (numTry > 5) break;
	}
	int spawnXCoord = g_theRNG->RollRandomIntInRange(0, CHUNK_SIZE_X - 1);
	int spawnYCoord = m_currentMap->GetChunkAtCoord(randomChunkID)->m_terrainHeightList[spawnXCoord];

	Vec2 spawnPosition = Vec2((float)spawnXCoord + (float)randomChunkID * CHUNK_SIZE_X + 0.5f, (float)spawnYCoord + def->m_visualHalfLength.y + 1.f);

	Character* characterPtr = m_currentMap->SpawnCharacter(def, spawnPosition);
	AIController* newAI = nullptr;

	if (def->m_name == "Hela")
	{
		newAI = new Hela_AI_Controller(this, characterPtr, (char)m_controllers.size(), teamID);
	}
	if (def->m_name == "Iron")
	{
		newAI = new Iron_AI_Controller(this, characterPtr, (char)m_controllers.size(), teamID);
	}
	if (def->m_name == "Hulk")
	{
		newAI = new Hulk_AI_Controller(this, characterPtr, (char)m_controllers.size(), teamID);
	}
	if (def->m_name == "Thor")
	{
		newAI = new Thor_AI_Controller(this, characterPtr, (char)m_controllers.size(), teamID);
	}
	if (def->m_name == "IW")
	{
		newAI = new IW_AI_Controller(this, characterPtr, (char)m_controllers.size(), teamID);
	}
	if (def->m_name == "Adam")
	{
		newAI = new Adam_AI_Controller(this, characterPtr, (char)m_controllers.size(), teamID);
	}

	m_controllers.push_back(newAI);
	m_initNumCharacters++;
	return newAI;
}

Controller* Game::GetController(char ID)
{
	for (auto& player : m_controllers)
	{
		if (player->m_ID == ID)
		{
			return player;
		}
	}

	return nullptr;
}

bool Game::IsPendingEndTunr() const
{
	return m_isPendingEndTurn;
}

void Game::PendingEndTurn()
{
	m_isPendingEndTurn = true;
}

bool Game::NextTurn()
{
	if (CheckIfGameEnded()) return false;

	m_turnHasPassed++;

	g_theInput->ResetCursorPosition();
	m_playerControlCamera = false;

	Players_NextTurn();

	Projectiles_NextTurn();

	ChangeUI();

	return true;
}

void Game::Players_NextTurn()
{
	// END TURN
	if (m_playerIDTurn != -1 && GetController(m_playerIDTurn) && GetController(m_playerIDTurn)->m_character)
	{
		GetController(m_playerIDTurn)->EndTurn();
	}
	int deadCharacter = 0;
	for (size_t i = 0; i < m_controllers.size(); i++)
	{
		if (!m_controllers[i]->m_character)
		{
			deadCharacter++;
		}
		else
		{
			m_controllers[i]->m_character->m_hasGainedFuryByTakingDamageThisTurn = false;
		}
	}

	if (m_playerIDTurn == -1 && deadCharacter == m_controllers.size())
	{
		//It's a draw
		SwitchState(GameState::MENU_MODE);
		return;
	}

	// NEXT TURN
	m_playerIDTurn = GetNextTurnPlayerID();

	GetController(m_playerIDTurn)->BeginTurn();

	m_controllerTookTurn.push_back(GetController(m_playerIDTurn));

	m_currentFocusCharacter = GetController(m_playerIDTurn)->m_character;
}

void Game::Projectiles_NextTurn()
{
	for (auto& proj : m_currentMap->m_livingProjectiles)
	{
		if (proj)
		{
			proj->BeginTurn();
			proj->m_turnAliveLeft--;
			if (proj->m_turnAliveLeft < 0)
			{
				proj->Die();
				delete proj;
				proj = nullptr;
			}
		}
	}

	m_currentMap->m_flyingProjectiles.clear();
}

char Game::GetNextTurnPlayerID()
{
	int numCharacterAlive = 0;
	for (auto& character : m_currentMap->m_characters)
	{
		if (character->m_health != -1) numCharacterAlive++;
	}

	if (m_controllerTookTurn.size() >= numCharacterAlive || m_controllerTookTurn.size() == m_initNumCharacters)
	{
		m_controllerTookTurn.clear();
	}

	int highestSpeed = 0;
	char potentialNextTurnID = -1;

	for (auto& player : m_controllers)
	{
		if (player->m_ID == m_playerIDTurn)
		{
			continue;
		}
		if (!player->m_character)
		{
			continue;
		}
		if (player->m_character->m_health == -1)
		{
			continue;
		}

		auto find = std::find(m_controllerTookTurn.begin(), m_controllerTookTurn.end(), player);

		if (find != m_controllerTookTurn.end())
		{
			continue;
		}

		if (player->m_character->m_speed > highestSpeed)
		{
			highestSpeed = player->m_character->m_speed;
			potentialNextTurnID = player->m_ID;
		}
		else if (player->m_character->m_speed == highestSpeed)
		{
			if (g_theRNG->RollRandomChance(0.5))
			{
				highestSpeed = player->m_character->m_speed;
				potentialNextTurnID = player->m_ID;
			}
		}
	}

	return potentialNextTurnID;
}

void Game::ChangeUI()
{
	for (auto& player : m_controllers)
	{
		if (!player->m_character) continue;
		if (player->m_character->m_health == -1) continue;

		if (player->m_teamID == 1) //!= GetController(m_playerIDTurn)->m_teamID
		{
			player->m_character->m_UIColor = Rgba8::COLOR_BRIGHT_RED;
		}
		else
		{
			player->m_character->m_UIColor = Rgba8::COLOR_BRIGHT_BLUE;
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------------------------
// HANDLE INPUT

void Game::HandleInput()
{
	IntVec2 screenSize = Window::GetMainWindowInstance()->GetClientDimensions();
	m_cursorPos = g_theInput->GetCursorNormalizedPosition() * Vec2((float)screenSize.x, (float)screenSize.y);

	if (m_isDebugging)
	{
		if (g_theInput->WasKeyJustPressed(KEYCODE_F1))
		{
			g_debugDraw = !g_debugDraw;
		}
		if (g_theInput->WasKeyJustPressed(KEYCODE_F2))
		{
			g_debugDestructible = !g_debugDestructible;
		}
		if (g_theInput->WasKeyJustPressed(KEYCODE_F3))
		{
			g_debugCharacterMoveFast = !g_debugCharacterMoveFast;
		}
		if (g_theInput->WasKeyJustPressed(KEYCODE_F4))
		{
			m_currentFocusCharacter->m_fury = 100;
		}
		if (g_theInput->WasKeyJustPressed(KEYCODE_F6))
		{
			m_currentFocusCharacter->m_stamina = 999999;
		}
		if (g_theInput->WasKeyJustPressed('P'))
		{
			PendingEndTurn();
		}
		if (g_debugDestructible)
		{
			Vec2 cursorRelative;
			cursorRelative.x = RangeMapClamped(m_cursorPos.x, 0.f, 1600.f, m_cameraView.m_mins.x, m_cameraView.m_maxs.x);
			cursorRelative.y = RangeMapClamped(m_cursorPos.y, 0.f, 800.f, m_cameraView.m_mins.y, m_cameraView.m_maxs.y);

			if (g_theInput->WasKeyJustPressed(KEYCODE_LEFT_MOUSE))
			{
				m_currentMap->ExplodeAtPosition(cursorRelative, 5.f, true);
			}
		}
		if (g_theInput->WasKeyJustPressed(KEYCODE_F9))
		{
			GameRestart();
		}
	}
}

void Game::SwitchState(GameState state)
{
	m_state = state;
	m_secondIntoMode = 0.f;

	if (g_theAudio->IsPlaying(m_menuMusic.m_playbackID))
	{
		g_theAudio->StopSound(m_menuMusic.m_playbackID);
	}
	g_theAudio->StopSound(m_currentMusic);
	m_currentMusic = MISSING_SOUND_ID;

	if (state == GameState::MENU_MODE)
	{
		m_menuMusic.m_playbackID = g_theAudio->StartSound(m_menuMusic.m_ID);
		m_currentMusic = m_menuMusic.m_playbackID;

		m_currentSelectingPlayerTurn = 0;
		if (m_selectCharactersPanel) m_selectCharactersPanel->SetActive(false);
		if (m_selectTeamPanel) m_selectTeamPanel->SetActive(true);
		if (m_selectVS) m_selectVS->SetActive(true);
		if (m_randomSeedMapButton)m_randomSeedMapButton->SetActive(false);
		for (size_t i = 0; i < 6; i++)
		{
			m_characterAvailableTeam1Selection[i] = true;
			m_characterAvailableTeam2Selection[i] = true;
			m_playerSelection[i] = -1;
			m_characterSelection[i] = "";

			if (m_playersButton[i])
			{
				m_playersButton[i]->SetActive(true);
				m_playersButton[i]->m_textureUnhover = UIDefinitions::GetTexureByName("Select_Add");
				m_playersButton[i]->m_textureHover = UIDefinitions::GetTexureByName("Select_Add");
				m_playersButton[i]->m_texturePressed = UIDefinitions::GetTexureByName("Select_Add");
				m_playersButton[i]->m_borderColor = Rgba8::COLOR_WHITE;
				m_playersButton[i]->m_colorUnhover = Rgba8(200, 200, 200);
				m_playersButton[i]->m_colorHover = Rgba8(255, 255, 255);
				m_playersButton[i]->m_colorPressed = Rgba8(150, 150, 150);
			}

			if (m_charactersButton[i])
			{
				m_charactersButton[i]->SetActive(true);
				m_charactersButton[i]->m_borderColor = Rgba8::COLOR_WHITE;
				m_charactersButton[i]->m_colorUnhover = Rgba8::COLOR_BRIGHT_WHITE;
				m_charactersButton[i]->m_colorHover = Rgba8::COLOR_WHITE;
				m_charactersButton[i]->m_colorPressed = Rgba8::COLOR_WHITE;
			}
		}
	}

	if (state == GameState::SELECT_CHARACTERS_MODE)
	{
		if (m_audioManager->AreAllSoundsLoaded())
		{
			m_audioManager->PlayMusic("SelectTeam", m_currentMusic);
		}
	}

	if (state == GameState::PLAY_MODE)
	{
		if (m_audioManager->AreAllSoundsLoaded())
		{
			m_audioManager->PlayMusic("Game", m_currentMusic);
		}
	}
}

UIDefinitions::UIDefinitions(XmlElement& element)
	:m_name(ParseXmlAttribute(element, "name", "")),
	m_path(ParseXmlAttribute(element, "path", ""))
{

}

void UIDefinitions::InitializeDefs(char const* filePath)
{
	XmlDocument file;
	XmlError result = file.LoadFile(filePath);
	GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS, "FILE IS NOT LOADED");

	XmlElement* rootElement = file.RootElement();
	GUARANTEE_OR_DIE(rootElement, "Root Element is null");

	XmlElement* uiDefElement = rootElement->FirstChildElement();

	while (uiDefElement)
	{
		std::string name = uiDefElement->Name();
		GUARANTEE_OR_DIE(name == "UIDefinition", "Root child element is in the wrong format");

		UIDefinitions* newAudioDef = new UIDefinitions(*uiDefElement);

		s_uiDefList.push_back(newAudioDef);
		uiDefElement = uiDefElement->NextSiblingElement();
	}
}

void UIDefinitions::ClearDefinition()
{
	for (auto& i : s_uiDefList)
	{
		if (i != nullptr)
		{
			delete i;
			i = nullptr;
		}
	}
}

Texture* UIDefinitions::GetTexureByName(std::string name)
{
	for (auto& i : s_uiDefList)
	{
		if (i->m_name == name)
		{
			return 	g_theApp->m_game->m_UITextures[i->m_index];
		}
	}
	return nullptr;
}

UIDefinitions* UIDefinitions::GetByName(std::string name)
{
	for (auto& i : s_uiDefList)
	{
		if (i->m_name == name)
		{
			return i;
		}
	}
	return nullptr;
}

std::vector<UIDefinitions*> UIDefinitions::s_uiDefList;