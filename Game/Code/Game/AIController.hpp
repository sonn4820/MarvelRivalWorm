#pragma once
#include "Game/Controller.hpp"
#include "Game/GameCommon.hpp"

constexpr float GUESSING_THRESHOLD = 20.f;

class AIController : public Controller
{
public:
	AIController(Game* owner, Character* character, char ID, char teamID);
	virtual ~AIController();

	void Update(float deltaSeconds) override;

	virtual void MovingLogic(float deltaSeconds);
	virtual void UsingSkillLogic(float deltaSeconds) = 0;
	virtual void NormalAttackLogic();


	bool IsPlayer() const override;

	Character* FindNearestRandomEnemy();
	Character* FindDifferentEnemy();
	Character* FindLowestInitialHPEnemy();
	Character* FindLowestCurrentHPEnemy();
	void SetAngleToPosition(Vec2 position);

	void BeginTurn() override;
	void EndTurn() override;

	void ChoosingForceAndAngle();
	void PureGuessingShot();
	void TryToHitTheCurrentTarget();
	bool IsEnemyNearTheLastHit() const;
	bool RandomTurn();

	float RollRandomeForce(float tMin, float tMax);
	float RollRandomeAngle(float tMin, float tMax);
	float GetRandomValueBasedOnSkillLevel(float minOffset, float maxOffset);
	void SetCurrentForceToHitThisTarget(float angleValue, Vec2 target);

	int GetAngryTurn();

	virtual bool Command_ModifyAI(EventArgs& args);

public:

	// LEVEL 
	bool m_canLearn = false;
	float m_skillLevel = 10.f; // 1-10

	// THINKING TIMER
	float m_ultThinkingTimer = 2.f;
	float m_forceAndAngleThinkingTime = 0.f;
	float m_movingThinkingTime = 0.f;
	float m_changingAngleTime = 0.f;
	float m_thinkingTimer = 0.f;

	// MOVING LOGIC
	float m_staminaMovingLimit = 0.f;
	bool m_wantToMove = true;
	bool m_doneMoving = false;
	bool m_hasTurned = false;
	float m_meleeDistance = 0.f;

	// SKILL LOGIC 
	float m_thinkingSkillTimer = 1.f;
	bool m_hasUsedSkill1 = false;
	bool m_hasUsedSkill2 = false;
	bool m_hasUsedUltimate = false;

	// NORMAL ATTACK LOGIC
	Character* m_currentFocusTarget = nullptr;
	int m_guessCount = 0;
	Vec2 m_lastHitPosition = Vec2(-1, -1);
	bool m_isGuessing = true;
	bool m_hasHitDirectly = false;
	bool m_hasHitExplosion = false;
	bool m_hasChosenNewAngleAndForce = false;
	bool m_hasPerfomedNA = false;
	float m_currentAngle = 0.f;
	float m_currentForce = 0.f;
};