#pragma once
#include "Game/GameCommon.hpp"

class Game;
class Character;

class Controller
{
public:
	Controller(Game* owner, Character* character, char ID, char teamID);
	virtual ~Controller() = default;

	virtual void Update(float deltaSeconds);
	virtual void UpdateUI();
	virtual bool IsPlayer() const = 0;
	virtual void BeginTurn();
	virtual void EndTurn();

	void RenderUICharacter() const;
	void RenderUIScreen() const;

	Game* m_game = nullptr;
	Character* m_character = nullptr;

	char m_ID = -1;
	char m_teamID = -1;

	bool m_isMoving = false;
	bool m_isChargingForShoot = false;

	std::vector<Vertex_PCU> m_nonTextureStaticUIVerts;
	std::vector<Vertex_PCU> m_avatarBGVerts;
	std::vector<Vertex_PCU> m_avatarVerts;
	std::vector<Vertex_PCU> m_skill1UIVerts;
	std::vector<Vertex_PCU> m_skill2UIVerts;
	std::vector<Vertex_PCU> m_skillUltimateUIVerts;

	Rgba8 m_backgroundColorBasedOnHP;
	Rgba8 m_skill1UIColor;
	Rgba8 m_skill2UIColor;
	Rgba8 m_skillUltimateUIColor;

	Texture* m_avatarVertsTexture = nullptr;
	Texture* m_skill1UITexture = nullptr;
	Texture* m_skill2UITexture = nullptr;
	Texture* m_skillUltUITexture = nullptr;
};

