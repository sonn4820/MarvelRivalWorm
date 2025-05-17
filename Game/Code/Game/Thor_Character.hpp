#pragma once
#include "Game/GameCommon.hpp"
#include "Game/CharacterCommon.hpp"

constexpr float THOR_NA_FORCE = 80.f;
constexpr float THOR_DASH_DISTANCE = 30.f;
constexpr int THOR_DASH_LIMIT = 3;
constexpr int THOR_S2_DAMAGE = 20;
constexpr int THOR_SHIELD_VALUE = 100;

constexpr float THOR_ULT_TOP_Y = 60.f;
constexpr float THOR_ULT_DESTRUCT_RADIUS= 24.f;
constexpr float THOR_ULT_FINDING_NEAREST_ENEMY_DISTANCE_X = 200.f;

constexpr float THOR_DASH_TIMER = 1.25f;

class Thor_Character : public Character
{
public:
	Thor_Character(Map* map);
	virtual ~Thor_Character();

	void BeginTurn() override;
	void NormalAttack() override;
	void Update(float deltaSeconds) override;
	void Passive()  override;
	void Skill_1()  override;
	void Skill_2()  override;
	void Ultimate() override;
	bool ConditionsForSkill_2() override;

	void ThrowMjolnir(float angle);

	void S2_Update();
	void Ult_Update();

	Vec2 m_initialPositon = Vec2();

	// S1
	bool m_shieldUpPreviousTurn= false;

	// S2
	int m_dashCount = 0;
	bool m_isDashing = false;
	Timer* m_dashTimer = nullptr;
	Vec2 m_dashPositon = Vec2();
	std::vector<Character*> m_charactersHitByDash;

	// ULT
	int m_ultStage = 0;
	bool m_isUsingUlt = false;
	Vec2 m_ultTargetPositon = Vec2();
	Timer* m_ultTimer = nullptr;

	Emitter* m_dashEmitter = nullptr;
};

class Thor_NA_Projectile : public Projectile
{
public:
	Thor_NA_Projectile(Map* map, Character* owner, ProjectileDefinition* def);

	void Update(float deltaSeconds) override;
	void Render() const override;
	void Die() override;

	int m_direction = 1;
	bool m_hasReturned = false;

	std::vector<Character*> m_charactersGotHit;
};

class Thor_AI_Controller : public AIController
{
public:
	Thor_AI_Controller(Game* owner, Character* character, char ID, char teamID);
	void UsingSkillLogic(float deltaSeconds) override;
	void NormalAttackLogic() override;
	void BeginTurn() override;
	bool m_dashNearEnough = false;
	bool m_hasAdjustedNAToNearestEnemy = false;
};