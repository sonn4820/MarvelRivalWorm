#pragma once
#include "Game/GameCommon.hpp"
#include "Game/Entity.hpp"

class Game;
class Character;

constexpr float GRAVITY_FORCE_PROJECTILE = -31.f;
struct ProjectileDefinition
{
	std::string m_name = "";
	bool m_isMelee = false;
	float m_meleeMaxRange = 0.f;
	bool m_isTeleport = false;

	bool m_isHoming = false;
	float m_homingRange = 0.f;
	float m_homingSpeed = 0.f;

	bool m_isHealing = false;
	bool m_canOverHeal = false;
	int m_healingValue = 0;

	float m_damageScale = 1.0;
	float m_hitRadius = 0;
	float m_destructionRadius = 0;

	std::string m_soundName = "";

	Vec2 m_visualHalfLength = Vec2();
	float m_offsetAngle = 0.f;
	std::string m_projectileTextureName = "";

	ProjectileDefinition(XmlElement& element);
	void SetCollision(XmlElement& element);
	void SetHoming(XmlElement& element);
	void SetVisual(XmlElement& element);
	void SetSound(XmlElement& element);

	static void InitializeDefs(char const* filePath);
	static void ClearDefinition();
	static ProjectileDefinition* GetByName(std::string name);
	static std::vector<ProjectileDefinition*> s_projectileDefList;
};

class Projectile : public Entity
{
public:
	Projectile(Map* map, Character* owner, ProjectileDefinition* def);
	virtual ~Projectile();

	void Update(float deltaSeconds) override;
	void Render() const override;
	void Die() override;
	virtual void Emitter_Update(float deltaSeconds);

	bool IsEnemy(Character* character) const;

	virtual void BeginTurn();

	Character* FindNearestEnemy(float range);
	Character* FindNearestAlly(float range);

	void Movement_Update(float deltaSeconds);
	bool DirectHitCheck();
	bool ImpactHitCheck(bool isDirectHit);

	void PlayExplodeEffect();
	void PlaySoundOnImpact(bool byPass = false);

public:
	ProjectileDefinition* m_projectileDef = nullptr;
	Character* m_owner = nullptr;

	Emitter* m_trailEmitter = nullptr;

	float m_distanceTravelled = 0.f;
	Vec2 m_hitPosition = Vec2(-1,-1);
	Character* m_characterGotHit = nullptr;

	int m_currentChunkCoord = -1;
	bool m_isHit = false;
	bool m_reachedSuperHigh = false;
	bool m_isStatic = false;

	// Living Projectile
	int m_turnAliveLeft = 0;
	bool m_hasTouchedGround = false;
	bool m_hasPlayedImpactedSound = false;

	// Animation
	Timer* m_deadTimer = nullptr;

	// Homing
	Character* m_homingTarget = nullptr;
};

