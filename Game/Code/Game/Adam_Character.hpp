#pragma once
#include "Game/GameCommon.hpp"
#include "Game/CharacterCommon.hpp"

class Adam_Character : public Character
{
public:
	Adam_Character(Map* map);
	virtual ~Adam_Character();

	void Update(float deltaSeconds) override;
	void EndTurn() override;
	void NormalAttack() override;
	void Passive()  override;
	void Skill_1()  override;
	void Skill_2()  override;
	void Ultimate() override;
	void Die() override;
	bool ConditionsForSkill_2() override;

	bool m_hasPassive = true;
	bool m_useHealing = false;

	Emitter* m_skillEmitter = nullptr;
};

class Adam_AI_Controller : public AIController
{
public:
	Adam_AI_Controller(Game* owner, Character* character, char ID, char teamID);
	void UsingSkillLogic(float deltaSeconds) override;

	bool m_ultfoundDeadTeammate = false;
};