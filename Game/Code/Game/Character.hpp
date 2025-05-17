#pragma once
#include "Game/GameCommon.hpp"
#include "Game/Entity.hpp"

class Game;
class Item;
class Controller;
class Projectile;
enum class AffectedTilesSet;
struct ProjectileDefinition;

constexpr float MOVING_SPEED = 15.f;
constexpr float ADJUST_ANGLE_SPEED = 20.f;
constexpr float ADJUST_FORCE_SPEED = 50.f;
constexpr int FURY_PER_ATTACK = 10;
constexpr int BASED_FURY_GAIN_TAKING_DAMAGE = 20;
constexpr float MOVING_STAMINA_COST = 50.f;

struct CharacterDefinition
{
	std::string m_name = "";
	int m_initialHealth = 0;
	int	m_initialSpeed = 0;
	float m_initialStamina = 0;
	int m_initialArmor = 0;
	int m_initialAttack = 0;
	int m_skill1Cost = 0;
	int m_skill2Cost = 0;
	int m_skill1Cooldown = 0;
	int m_skill2Cooldown = 0;
	FloatRange m_angleLimit = FloatRange();
	FloatRange m_forceLimit = FloatRange();
	bool m_isMelee = false;

	std::string m_throwSound = "";
	std::string m_S1Sound = "";
	std::string m_S2Sound = "";
	std::string m_UltSound = "";

	float m_hitboxRadius = 0;
	float m_gravitySpeed = 9.8f;

	Vec2 m_visualHalfLength = Vec2();
	std::string m_characterTextureName = "";

	std::vector<ProjectileDefinition*> m_projectiles; //index 0 is NA, index 1 is Ultimate

	CharacterDefinition(XmlElement& element);
	void SetCollision(XmlElement& element);
	void SetVisual(XmlElement& element);
	void SetProjectiles(XmlElement& element);
	void SetSound(XmlElement& element);
	void SetIcon(XmlElement& element);
	static void InitializeDefs(char const* filePath);
	static void ClearDefinition();
	static CharacterDefinition* GetByName(std::string name);
	static std::vector<CharacterDefinition*> s_characterDefList;
};

class Character : public Entity
{
public:
	Character(Map* map);
	virtual ~Character();

	void CleanUp();

	void Update(float deltaSeconds) override;
	void Render() const override;
	void RenderUI() const;
	void RenderDebug() const;
	void Die() override;

	void GravityAndFloorCollision(float deltaSeconds);

	virtual void MoveRight(float deltaSeconds);
	virtual void MoveLeft(float deltaSeconds);
	virtual void MoveUp(float deltaSeconds);
	virtual void MoveOnCurrentFacing(float deltaSeconds);
	virtual void MoveDown(float deltaSeconds);

	void FaceRight();
	void FaceLeft();
	void FaceTheOpposite();
	void AdjustAngle(float UpOrDown, float deltaSeconds);
	void AdjustForce(float deltaSeconds);

	bool virtual TakeDamage(int damage, Character* characterDealedDamage, bool isCrit);
	void Heal(int heal, bool overHeal);
	int CalculateDamage(bool& out_isCrit, float bonusCritRate = 0.f, float bonusCritDamage = 0.f);

	Item* GetItem(std::string name);
	void RemoveItem(Item* item);
	Character* GetNearestEnemy() const;
	Character* GetNearestAlly() const;
	int GetID() const;
	int GetTeamID() const;

	void ReceiveItem(Item* item);

	Projectile* ShootCurrentProjectile(float angle, float force);
	virtual void NormalAttack();

	virtual void Passive() = 0;
	virtual void Skill_1();
	virtual void Skill_2();
	virtual void Ultimate();

	virtual void BeginTurn();
	virtual void EndTurn();

	virtual bool ConditionsForSkill_1();
	virtual bool ConditionsForSkill_2();
	virtual bool ConditionsForUltimate();

	void Initialize();
	void InitializeEvent();
	void InitializeVisual();
	void InitializeStats();
	void FixAngleLimitToTheCorrectDirection();

	bool IsCurrentHealthPercentageLowerThan(float zeroToOne);

	bool IsPositionBehind(Vec2 position);
	bool FaceToTarget(Vec2 target);

	void PlayWalkingSound();

	virtual bool Command_ModifyStats(EventArgs& args);

	void PlayHPChangeUIEffect(int value, bool isCrit, bool isHeal = false);
public:
	CharacterDefinition* m_characterDef = nullptr;
	ProjectileDefinition* m_currentProjectile = nullptr;
	Controller* m_controller = nullptr;

	AffectedTilesSet m_affectedTileSet;

	Timer* m_UIDamageTextTimer = nullptr;
	std::vector<Vertex_PCU> m_UIDamageTextVerts;
	Mat44 m_UIDamageTextMatrix;
	Rgba8 m_UIDamageTextColor;

	int m_skill1CDCounter = 0;
	int m_skill2CDCounter = 0;
	int m_health = 0;
	int m_attack = 0;
	int m_armor = 0;
	int	m_shield = 0;
	int	m_speed = 0;
	float m_stamina = 0;
	int m_fury = 0;
	float m_critRateFromZeroToOne = 0.1f;
	float m_critDamageMultiplier = 1.4f;
	float m_currentHitboxRadius = 0.f;
	Vec2 m_currentVisualHalfLength;
	int m_killCount = 0;

	FloatRange m_currentAngleLimit = FloatRange();
	FloatRange m_currentForceLimit = FloatRange();
	float m_currentAngle = 0.f;
	float m_currentForce = 0.f;
	float m_lastForce = 0.f;

	std::vector<Item*> m_currentItems;
	std::vector<int> m_vfxIndex;

	Rgba8 m_UIColor;
	bool m_isFacingLeft = false;
	bool m_isSpendingStamina = false;
	bool m_isFlying = false;
	bool m_canUseSKill1 = true;
	bool m_canUseSKill2 = true;
	bool m_isGrounded = false;
	bool m_gotHit = false;
	bool m_isInvisible = false;
	bool m_isPendingInvisible = false;
	bool m_hasGainedFuryByTakingDamageThisTurn = false;

	SoundPlaybackID m_currentWalkingGroundSound;
	SoundPlaybackID m_currentWalkingWaterSound;
	GroundSound m_currentGroundType = GroundSound::AIR;

private:
	float m_forceDirection = 1.f;
};