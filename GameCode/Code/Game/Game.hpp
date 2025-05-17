#pragma once
#include "Game/GameCommon.hpp"
#include "Game/Map.hpp"

class PlayerController;
class AIController;
class Controller;
class ParticleSystemDFS2;
class AudioManagerDFS2;

enum class GameState
{
	MENU_MODE,
	SELECT_CHARACTERS_MODE,
	PLAY_MODE,
};

struct UIDefinitions
{
	std::string m_name = "";
	std::string m_path = "";

	Image* m_image = nullptr;
	int m_index = -1;

	UIDefinitions(XmlElement& element);
	static void InitializeDefs(char const* filePath);
	static void ClearDefinition();
	static Texture* GetTexureByName(std::string name);
	static UIDefinitions* GetByName(std::string name);

	static std::vector<UIDefinitions*> s_uiDefList;
};

class Game
{
public:
	Game();
	~Game();

	void Startup();
	void Update();
	void Render() const;
	void Shutdown();

	// STATE
	void SwitchState(GameState state);

	// GAME RESTART
	void GameRestart();
	bool CheckIfGameEnded();

	// Camera
	void GameCameraCursorZoom(Vec2 cursor, float value);
	void GameCameraCursorMove(Vec2 cursor, float value);
	void GameCameraFollow(Vec2 target, float easingValue = 0.035f);
	void SetCooldownCamera(float time);

	// UI
	void MainMenuUI_Init();
	void SelectCharacterUI_Init();
	void InGameUI_Init();

	// Player
	PlayerController* AddPlayer(std::string characterName, char teamID);
	AIController* AddAI(std::string characterName, char teamID);
	Controller* GetController(char ID);

	bool IsPendingEndTunr() const;
	void PendingEndTurn();
	bool NextTurn();
	void Players_NextTurn();
	void Projectiles_NextTurn();
	char GetNextTurnPlayerID();
	void ChangeUI();

	void SetUpMap(std::vector<char> teamID, std::vector<std::string> characterName);

	// Gameplay
	void TurnAndCameraManaging(float deltaSecond);

	static bool Command_NextTurn(EventArgs& arg);
	static bool Command_GameRestart(EventArgs& arg);
	static bool Command_ToggleDemo(EventArgs& arg);
	static bool Command_ToggleDebugMode(EventArgs& arg);
	static bool Command_TestCase(EventArgs& arg);
	static bool Command_ToggleDebugDraw(EventArgs& arg);
	static bool Command_ToggleDestructible(EventArgs& arg);
	static bool Command_ToggleMoveFast(EventArgs& arg);

public:
	Camera m_worldCamera;
	AABB2 m_cameraView = AABB2(Vec2::ZERO, Vec2(WORLD_SIZE_X, WORLD_SIZE_Y));

	Camera m_screenCamera;
	GameState m_state = GameState::MENU_MODE;
	Clock* m_gameClock = nullptr;
	Vec2 m_cursorPos;

	Map* m_currentMap = nullptr;

	std::atomic<bool> m_isUIQuitting;
	std::vector<Texture*> m_UITextures;

	bool m_teamMode = true;
	char m_playerIDTurn = -1;
	char m_turnHasPassed = 0;
	bool m_isDemo = false;
	bool m_isDebugging = false;
	float m_screenShakeAmount = 0.0f;
	int m_mapSeed = 0;

	ParticleSystemDFS2* m_vfxSystem = nullptr;
	AudioManagerDFS2* m_audioManager = nullptr;

	std::vector<Controller*> m_controllers;
	std::vector<Controller*> m_controllerTookTurn;

	Sound m_menuMusic;
	SoundPlaybackID m_currentMusic;

	// MENU
	Canvas* m_menuCanvas = nullptr;

	Text* m_menuTitleMain = nullptr;
	Text* m_menuTitleSub = nullptr;
	Panel* m_menuMainPanel = nullptr;
	Button* m_menuPlayButton = nullptr;
	Button* m_menuHTPButton = nullptr;
	Button* m_menuSettingButton = nullptr;
	Button* m_menuQuitButton = nullptr;

	Panel* m_howToPlayPanel = nullptr;
	Button* m_htpBackButton = nullptr;
	Text* m_htpTitle = nullptr;
	Text* m_htpText1= nullptr;
	Text* m_htpText2= nullptr;
	Text* m_htpText3= nullptr;
	Text* m_htpText4= nullptr;

	Panel* m_settingMainPanel = nullptr;
	Text* m_settingTitle = nullptr;
	Text* m_settingMusicText = nullptr;
	Text* m_settingSoundText = nullptr;
	Slider* m_settingMusicSlider = nullptr;
	Slider* m_settingSoundSlider = nullptr;
	Button* m_settingBackButton = nullptr;

	// SELECT
	char m_playerSelection[6];
	std::string m_characterSelection[6] ; // 0 - hela, 1 - iron, 2 - adam, 3 - iw, 4 - thor, 5 - hulk
	bool m_characterAvailableTeam1Selection[6];
	bool m_characterAvailableTeam2Selection[6];
	char m_currentSelectingPlayerTurn = 0;

	Canvas* m_characterCanvas = nullptr;

	Panel* m_selectTeamPanel = nullptr;
	Text* m_selectTeamTitle = nullptr;
	Text* m_selectCharacterTitle = nullptr;
	Text* m_selectTeam1Title = nullptr;
	Text* m_selectTeam2Title = nullptr;
	Text* m_selectVS = nullptr;
	Button* m_selectTeamConfirmButton = nullptr;
	Button* m_playersButton[6];

	Panel* m_characterInfoPanel = nullptr;
	Text* m_characterTextName = nullptr;
	Text* m_characterTextS1 = nullptr;
	Text* m_characterTextS2 = nullptr;
	Text* m_characterTextU = nullptr;
	Text* m_characterTextP = nullptr;
	Text* m_characterTextS1_Info = nullptr;
	Text* m_characterTextS2_Info = nullptr;
	Text* m_characterTextU_Info = nullptr;
	Text* m_characterTextP_Info = nullptr;
	bool m_characterButtonIsHoveringState[6];

	Panel* m_selectCharactersPanel = nullptr;
	Button* m_charactersButton[6];
	Button* m_playGameButton = nullptr;
	Button* m_randomSeedMapButton = nullptr;
	Text* m_seedMapText = nullptr;


	// IN GAME
	float m_timerNextTurnPanel = 2.f;

	Canvas* m_inGameCanvas = nullptr;

	Panel* m_nextTurnPanel = nullptr;
	Sprite* m_characterUISprite;
	Text* m_yourTurnText;
	Text* m_characterNameText;

	Panel* m_winPanel = nullptr;
	Sprite* m_MVP_UISprite;
	Text* m_TeamWinText;
	Text* m_mvpText;
	Text* m_mvpNameText;

private:
	// VARIABLES
	Character* m_currentFocusCharacter = nullptr;
	Timer* m_endGameWaitTimer = nullptr;
	int m_initNumCharacters = 0;
	float m_cameraMoveCooldown = 0.0f;
	float m_secondIntoMode = 0.f;
	bool m_isPendingEndTurn = false;
	bool m_playerControlCamera = false;
	bool m_gameHasEnded = false;

	// UPDATE
	void UpdateMenuMode(float deltaSecond);
	void UpdateCharacterSelectMode(float deltaSecond);
	void UpdatePlayMode(float deltaSecond);
	void HandleInput();
	void UpdateCamera(float deltaSecond);

	// RENDER
	void RenderMenuMode() const;
	void RenderCharacterSelectMode() const;
	void RenderPlayMode() const;
	void RenderScreen() const;

	// UI FUNCTIONS
	void LoadUISprite();
	void RandomSeed();
	void PlayGame();
	void OpenSetting();
	void OpenHowToPlay();
	void OpenMenuOptions();
	void OpenCharacterSelect();
	void SetButtonPlayer(char id);
	void SetButtonCharacter(char characterID);
	void ShowButtonCharacterInfo(char characterID);
	void RemoveCharacterInfo(char characterID);
	void ToggleVisibilityCharacterInfo();
	void ShowNextTurnPanel();
};