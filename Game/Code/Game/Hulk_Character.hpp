#pragma once
#include "Game/GameCommon.hpp"
#include "Game/CharacterCommon.hpp"

constexpr int HULK_JUMP_DAMAGE = 20;
constexpr float HULK_ULT_SCALE = 2.f;
constexpr int HULK_ULT_DURATION = 3;

class Hulk_Character : public Character
{
public:
	Hulk_Character(Map* map);
	virtual ~Hulk_Character();

	void BeginTurn() override;
	void EndTurn() override;
	void Update(float deltaSeconds) override;
	void Render() const override;
	void NormalAttack() override;
	void Passive()  override;
	void Skill_1()  override;
	void Skill_2()  override;
	void Ultimate() override;
	bool ConditionsForSkill_1() override;
	bool ConditionsForSkill_2() override;

	void ExplodeGround(float radius);

	bool m_isJumping = false;
	Vec2 m_jumpVelocity = Vec2();
	bool m_hasResetAfterLanded = true;
	bool m_hasUsedSkill2 = false;

	float m_currentScale = 1.f;
	int m_ultDurationCounter = 0;

	Emitter* m_jumpingAuraEmitter = nullptr;
};

class Hulk_Shield : public Item 
{
public:
	Hulk_Shield(Map* map, Character* owner, ItemDefinition* def);
	void Update(float deltaSeconds) override;
	void Render() const override;
};

class Hulk_AI_Controller : public AIController
{
public:
	Hulk_AI_Controller(Game* owner, Character* character, char ID, char teamID);
	void UsingSkillLogic(float deltaSeconds) override;
	void NormalAttackLogic() override;

	bool m_isUsingSkill1 = false;
};