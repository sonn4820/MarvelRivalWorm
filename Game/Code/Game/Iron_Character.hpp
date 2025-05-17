#pragma once
#include "Game/GameCommon.hpp"
#include "Game/CharacterCommon.hpp"

constexpr int IRON_S2_COUNT = 6;
constexpr int IRON_ULTIMATE_DAMAGE = 100;

class Iron_Character : public Character
{
public:
	Iron_Character(Map* map);
	virtual ~Iron_Character();

	void BeginTurn() override;
	void EndTurn() override;
	void Passive()  override;
	void Skill_1()  override;
	void Skill_2()  override;
	void Ultimate() override;
	void NormalAttack() override;

	void UltimateUpdate(float deltaSeconds);
	void PlayMovingSound();

	void Update(float deltaSeconds) override;
	void MoveUp(float deltaSeconds) override;
	void MoveLeft(float deltaSeconds) override;
	void MoveRight(float deltaSeconds) override;
	void MoveDown(float deltaSeconds) override;

	int m_bonusStamina = 0;
	bool m_hasUsedSkill1 = false;
	bool m_hasSpawnedUltimate = true;

	Emitter* m_flyingEmitter = nullptr;
	Emitter* m_skillEmitter = nullptr;

	bool m_isFirstMove = true;
	SoundPlaybackID m_firstMoveSound;
	SoundPlaybackID m_flyingSound;
};

class Iron_AI_Controller : public AIController
{
public:
	Iron_AI_Controller(Game* owner, Character* character, char ID, char teamID);
	void MovingLogic(float deltaSeconds) override;
	void UsingSkillLogic(float deltaSeconds) override;
	void BeginTurn() override;
	void NormalAttackLogic() override;

	bool IsInCurrentTargetY() const;
	bool IsInCurrentTargetX() const;

	float m_targetYPos = 0.f;
	bool m_hasReachedTarget = false;
	bool m_hasDoneUsingSkills = false;
};