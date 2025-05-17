#pragma once
#include "Game/GameCommon.hpp"
#include "Game/CharacterCommon.hpp"

constexpr float HELA_SPAWN_TIMER = 0.2f;
constexpr float HELA_CRIT_RATE_PASSIVE = 0.01f;
constexpr int HELA_DAMAGE_PER_STACK = 2;
constexpr int HELA_ULTIMATE_COUNT = 3;
constexpr int HELA_ULTIMATE_DAMAGE = 150;
constexpr int HELA_ULTIMATE_MAX_ANGLE = -1;
constexpr int HELA_ULTIMATE_MIN_ANGLE = -179;
constexpr int HELA_ULTIMATE_Y_POSITION_OFFSET = 70;
constexpr int HELA_FURY_PER_ATTACK_HIT = 2;
constexpr float HELA_ULTIMATE_MOVING_RANGE = 120.f * 120.f;

class Hela_Character : public Character
{
public:
	Hela_Character(Map* map);
	virtual ~Hela_Character();

	void Update(float deltaSeconds) override;
	void EndTurn() override;
	void NormalAttack() override;
	void Passive()  override;
	void Skill_1()  override;
	void Skill_2()  override;
	void Ultimate() override;

	void NA_Update(float deltaSeconds);
	void Ult_Update();

	// NA
	int m_damageStack = 0;
	int m_projectileStack = 0;
	bool m_performedNA = false;
	float m_spawnTimer = 0.f;

	// ULTIMATE
	Timer* m_ultTimer = nullptr;
	Vec2 m_ultimatePosition = Vec2();
	Vec2 m_initialPositon = Vec2();
	bool m_usingUltimate = false;
	int m_ultimateCount = HELA_ULTIMATE_COUNT;

	Emitter* m_skillEmitter = nullptr;
};

class Hela_AI_Controller : public AIController
{
public:
	Hela_AI_Controller(Game* owner, Character* character, char ID, char teamID);
	void UsingSkillLogic(float deltaSeconds) override;
	void NormalAttackLogic() override;
	void BeginTurn() override;

	void UseSkill2();

	Character* m_ultTargetCharacter = nullptr;
	bool m_isMovingInUlt = false;
};