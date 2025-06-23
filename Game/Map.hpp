#pragma once
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/RaycastUtils.hpp"

constexpr int X_BITS = 4;
constexpr int Y_BITS = 7;

constexpr int CHUNK_SIZE_X = (1 << X_BITS);
constexpr int CHUNK_SIZE_Y = (1 << Y_BITS);

constexpr int TOTAL_BLOCKS_SIZE = CHUNK_SIZE_X * CHUNK_SIZE_Y;

constexpr int X_MASK = CHUNK_SIZE_X - 1;
constexpr int Y_MASK = (CHUNK_SIZE_Y - 1) << X_BITS;

constexpr int BLOCK_BIT_IS_SKY = 0;
constexpr int BLOCK_BIT_IS_VISIBLE = 1;
constexpr int BLOCK_BIT_IS_DESTRUCTIBLE = 2;
constexpr int BLOCK_BIT_IS_SOLID = 3;
constexpr int BLOCK_BIT_IS_HIT = 4;

class Map;
class Chunk;
struct Block;
struct BlockIterator;
class Entity;
class Character;
class Projectile;
class Game;
class Controller;
class Emitter;
struct CharacterDefinition;

enum Direction
{
	UP,
	DOWN,
	LEFT,
	RIGHT
};

enum BlockDataDebugDrawMode
{
	NONE,
	IS_SKY,
	IS_DESTRUCTIBLE,
	IS_SOLID,
	BlockDataDebugDrawMode_NUM_MODE
};

enum class AffectedTilesSet
{
	EXPLOSION,
	THOR,
	HELA,
	ADAM,
	HULK
};

struct BlockDefinition
{
	std::string m_name = "";
	unsigned char m_type = 0;
	bool m_isVisible = false;
	bool m_isSolid = false;
	bool m_isDestructible = false;
	IntVec2 m_spriteCoord;

	BlockDefinition() = default;
	static void InitializeBlockDefs();
	static void ClearDefinition();
	static void CreateNewBlockDef(std::string name, unsigned char type, bool visible, bool solid, bool destructible, IntVec2 spriteCoord);
	static BlockDefinition* GetByName(std::string name);
	static BlockDefinition* GetByType(unsigned int type);
	static std::vector<BlockDefinition*> s_blockDefList;
};

struct Block
{
	unsigned char m_type;
	unsigned char m_frontGroundType;
	unsigned char m_backGroundType;
	unsigned char m_bitFlags;

	bool CanSeeSky() const;
	bool IsHit() const;
	bool IsBlockVisible() const;
	bool IsBlockSolid() const;
	bool IsBlockDestructible() const;

	void SetIsSky();
	void SetIsNotSky();
	void SetIsHit();
	void SetIsNotHit();
};
class Chunk
{
public:
	Chunk(Map* map, int coord);
	~Chunk();

	void Update(float deltaSeconds);
	void Render() const;

	IntVec2 GetBlockGlobalCoord(IntVec2 local);
	IntVec2 GetBlockLocalCoord(IntVec2 global);
	Block* GetBlock(IntVec2 coord);
	Block* GetBlock(int index);
	int GetBlockIndexFromGlobalPosition(Vec2 globalPosition);

	void SetBlockType(IntVec2 blockCoord, unsigned int type);
	void SetBlockType(int index, unsigned int type);
	void SetBlockFrontgroundType(int index, unsigned int type);
	void SetBlockBackgroundType(int index, unsigned int type);

	void DestructBlocksInDisc(Vec2 pos, float radius, bool changeTerrainType, AffectedTilesSet set = AffectedTilesSet::EXPLOSION);

	void ChangeAffectedTiles(std::vector<int> listIndex, int rightType, int upType, int leftType, int highVolType, int lowVolType);
	void UpdateTerrainHeight();

	void SetMeshDirty();
	void GenerateData();
	void CreateBlockMesh(BlockIterator blockIter);
	bool BuildCPUMesh();

	Vec2 GetWorldCenter() const;

public:

	Block* m_blocks = nullptr;
	int m_coord;
	AABB2 m_bound;

	std::vector<Vertex_PCU> m_cpuMeshVerts;
	VertexBuffer* m_gpuMeshVBO = nullptr;

	std::vector<Vertex_PCU> m_debugVerts;

	std::vector<int> m_terrainHeightList;
	std::vector<float> m_hillinessList;
	std::vector<float> m_oceannessList;
	std::vector<float> m_temperatureList;
	std::vector<float> m_humidityList;
	std::vector<float> m_forestnessList;
	std::vector<float> m_lavanessList;

	Map* m_map = nullptr;

	Chunk* m_leftNeighbor = nullptr;
	Chunk* m_rightNeighbor = nullptr;

	bool m_isDirty = true;
	bool m_isActive = false;

};

struct BlockIterator
{
	BlockIterator() = default;
	explicit BlockIterator(Chunk* chunk, int blockIndex);
	Chunk* m_chunk = nullptr;
	int m_blockIndex = 0;

	Block* GetBlock() const;
	Vec2 GetWorldCenter() const;

	BlockIterator GetUpNeighbor() const;
	BlockIterator GetDownNeighbor() const;
	BlockIterator GetLeftNeighbor() const;
	BlockIterator GetRightNeighbor() const;

	void GetNumberOfBlockAround(int bitCondition, int& out_num) const;
};

struct GameRaycastResult2D : public RaycastResult2D
{
	BlockIterator m_blockIterator;
};

class Map
{
public:
	Map(Game* game);
	~Map();

	void StartUp();
	void Update(float deltaSeconds);
	void DebugUpdate(float deltaSeconds);
	void Render() const;

	void SkyUpdate(float deltaSeconds);

	Chunk* GetChunkAtCoord(int chunkCoord) const;
	Chunk* GetChunkAtPosition(Vec2 position) const;
	IntVec2 GetBlockLocalCoord(IntVec2 global) const;
	Chunk* GetChunkAndBlockIndexFromGlobalPosition(int& out_blockIndex, Vec2 position);

	void ActivateChunk(Chunk* newChunk);
	void DeactivateChunk(Chunk* chunk);

	void BuildChunkDirtyMeshes();

	int GetNumChunks();

	void FlushEntireWorld();
	bool IsNightTime();
	float GetCurrentTimeOfDay();

	void ExplodeAtPosition(Vec2 pos, float radius, bool changeTerrainType, AffectedTilesSet set = AffectedTilesSet::EXPLOSION, bool shakeScreen = true);

	void CurrentProjectilesUpdate(float deltaSeconds);

	GameRaycastResult2D RaycastWorld(Vec2 startPos, Vec2 fwdNormal, float maxDist) const;

	Character* SpawnCharacter(CharacterDefinition* characterDef, Vec2 position);
	bool CanThisChunkSpawnCharacter(int chunkID);

	void SpawnGrave(Vec2 position, Controller* controller, CharacterDefinition* characterDef);
	void HandleDeadCharacters();

	static Map* GenerateRandomMap(Game* game, int totalNumChunk, int seed = 0);

public:
	Game* m_game = nullptr;
	ConstantBuffer* m_gameCBO = nullptr;

	Shader* m_worldShader = nullptr;
	Texture* m_worldTexture = nullptr;
	SpriteSheet* m_worldSpriteSheet = nullptr;

	std::map<int, Chunk*> m_activeChunks;
	std::vector<Chunk*> m_dirtyMeshChunks;

	std::vector<Character*> m_characters;
	std::vector<Character*> m_deadCharacters;
	std::vector <Projectile*> m_flyingProjectiles;
	std::vector <Projectile*> m_livingProjectiles;
	std::vector<int> m_spawnedCharacterChunkID;

	float m_worldDay = 0.f;
	float m_worldTimeScale = 200.f;
	Vec3 m_skyColor;

	AABB2 m_bound;

	unsigned int m_worldSeed = 0;

	//DEBUG
	BlockDataDebugDrawMode m_blockDebugMode = NONE;
};

int		GetBlockIndex(IntVec2 localBlockCoord);
IntVec2 GetBlockCoord(int index);
int		GetChunkGoord(IntVec2 globalBlockCoord);
