#pragma once
#include "Game/GameCommon.hpp"
#include "Game/CharacterCommon.hpp"

constexpr int IW_S2_ALIVE_PERIOD = 5;
constexpr int IW_ULT_ALIVE_PERIOD = 5;

class IW_Character : public Character
{
public:
	IW_Character(Map* map);
	virtual ~IW_Character();

	void NormalAttack() override;
	void BeginTurn() override;
	void Passive()  override;
	void Skill_1()  override;
	void Skill_2()  override;
	void Ultimate() override;
	bool ConditionsForSkill_1() override;

	Projectile* IW_ShootCurrentProjectile(float angle, float force);

	bool m_usingSkill2 = false;
	bool m_usingUltimate = false;
};

class IW_S2_Projectile : public Projectile
{
public:
	IW_S2_Projectile(Map* map, Character* owner, ProjectileDefinition* def);
	void Update(float deltaSeconds) override;

	float m_forceReduction = 30.f;
};

class IW_Ultimate_Projectile : public Projectile
{
public:
	IW_Ultimate_Projectile(Map* map, Character* owner, ProjectileDefinition* def);
	void Update(float deltaSeconds) override;
	void SpawnField(Chunk* initialChunk);
};

class IW_Ultimate_Field : public Projectile
{
public:
	IW_Ultimate_Field(Map* map, Character* owner, ProjectileDefinition* def);
	~IW_Ultimate_Field();
	void BeginTurn() override;
	void Update(float deltaSeconds) override;
	void SetPositionAndBound(Vec2 position);
	AABB2 m_bound;
};

class IW_AI_Controller : public AIController
{
public:
	IW_AI_Controller(Game* owner, Character* character, char ID, char teamID);
	void UsingSkillLogic(float deltaSeconds) override;
	void NormalAttackLogic() override;
	void BeginTurn() override;

	bool m_hasChosenCharacterToHeal = false;
	bool m_hasResetNAFromS2 = false;
	bool m_hasResetNAFromUlt = false;
	bool m_hasS2TouchGround = false;
	bool m_hasS2TouchGroundCheck = false;
	bool m_hasUltTouchGround = false;
	bool m_hasUltTouchGroundCheck = false;
	Character* m_characterToHeal = nullptr;

};


