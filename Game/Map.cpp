#include "Game/Map.hpp"
#include "Game/App.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/PlayerController.hpp"
#include "Game/AIController.hpp"
#include "Game/AllCharacters.hpp"

std::vector<BlockDefinition*> BlockDefinition::s_blockDefList;


int GetBlockIndex(IntVec2 localBlockCoord)
{
	return localBlockCoord.x | localBlockCoord.y << X_BITS;
}

IntVec2 GetBlockCoord(int index)
{
	return IntVec2(index & X_MASK, (index & Y_MASK) >> X_BITS);
}

int GetChunkGoord(IntVec2 globalBlockCoord)
{
	return globalBlockCoord.x / CHUNK_SIZE_X;
}

void BlockDefinition::InitializeBlockDefs()
{
	BlockDefinition::CreateNewBlockDef("air", 0, false, false, false, IntVec2(0, 0));
	BlockDefinition::CreateNewBlockDef("grass", 1, true, true, true, IntVec2(1, 0));
	BlockDefinition::CreateNewBlockDef("dirt", 2, true, true, true, IntVec2(2, 0));
	BlockDefinition::CreateNewBlockDef("stone", 3, true, true, true, IntVec2(3, 0));
	BlockDefinition::CreateNewBlockDef("coal", 4, true, true, true, IntVec2(4, 0));
	BlockDefinition::CreateNewBlockDef("iron", 5, true, true, true, IntVec2(5, 0));
	BlockDefinition::CreateNewBlockDef("gold", 6, true, true, true, IntVec2(6, 0));
	BlockDefinition::CreateNewBlockDef("diamond", 7, true, true, true, IntVec2(7, 0));

	BlockDefinition::CreateNewBlockDef("water", 8, true, false, false, IntVec2(0, 1));
	BlockDefinition::CreateNewBlockDef("snowgrass", 9, true, true, true, IntVec2(1, 1));
	BlockDefinition::CreateNewBlockDef("sand", 10, true, true, true, IntVec2(2, 1));
	BlockDefinition::CreateNewBlockDef("ice", 11, true, true, true, IntVec2(3, 1));
	BlockDefinition::CreateNewBlockDef("glacier", 12, true, true, true, IntVec2(4, 1));

	BlockDefinition::CreateNewBlockDef("explosion_right", 13, true, true, true, IntVec2(0, 2));
	BlockDefinition::CreateNewBlockDef("explosion_up", 14, true, true, true, IntVec2(1, 2));
	BlockDefinition::CreateNewBlockDef("explosion_left", 15, true, true, true, IntVec2(2, 2));
	BlockDefinition::CreateNewBlockDef("explosion_high", 16, true, true, true, IntVec2(3, 2));
	BlockDefinition::CreateNewBlockDef("explosion_low", 17, true, true, true, IntVec2(4, 2));

	BlockDefinition::CreateNewBlockDef("thorDash_right", 18, true, true, true, IntVec2(0, 3));
	BlockDefinition::CreateNewBlockDef("thorDash_up", 19, true, true, true, IntVec2(1, 3));
	BlockDefinition::CreateNewBlockDef("thorDash_left", 20, true, true, true, IntVec2(2, 3));
	BlockDefinition::CreateNewBlockDef("thorDash_high", 21, true, true, true, IntVec2(3, 3));
	BlockDefinition::CreateNewBlockDef("thorDash_low", 22, true, true, true, IntVec2(4, 3));

	BlockDefinition::CreateNewBlockDef("hela_1", 23, true, true, true, IntVec2(0, 4));
	BlockDefinition::CreateNewBlockDef("hela_2", 24, true, true, true, IntVec2(1, 4));
	BlockDefinition::CreateNewBlockDef("hela_3", 25, true, true, true, IntVec2(2, 4));

	BlockDefinition::CreateNewBlockDef("adam_1", 26, true, true, true, IntVec2(0, 5));
	BlockDefinition::CreateNewBlockDef("adam_2", 27, true, true, true, IntVec2(1, 5));

	BlockDefinition::CreateNewBlockDef("hulk_1", 28, true, true, true, IntVec2(0, 6));
	BlockDefinition::CreateNewBlockDef("hulk_2", 29, true, true, true, IntVec2(1, 6));
	BlockDefinition::CreateNewBlockDef("hulk_3", 30, true, true, true, IntVec2(2, 6));
	BlockDefinition::CreateNewBlockDef("hulk_3", 31, true, true, true, IntVec2(3, 6));
}

void BlockDefinition::ClearDefinition()
{
	for (auto& i : s_blockDefList)
	{
		if (i != nullptr)
		{
			delete i;
			i = nullptr;
		}
	}
}

void BlockDefinition::CreateNewBlockDef(std::string name, unsigned char type, bool visible, bool solid, bool destructible, IntVec2 spriteCoord)
{
	BlockDefinition* blockDef = new BlockDefinition();
	blockDef->m_name = name;
	blockDef->m_type = type;
	blockDef->m_isVisible = visible;
	blockDef->m_isSolid = solid;
	blockDef->m_isDestructible = destructible;
	blockDef->m_spriteCoord = spriteCoord;

	s_blockDefList.push_back(blockDef);
}

BlockDefinition* BlockDefinition::GetByName(std::string name)
{
	for (auto& i : s_blockDefList)
	{
		if (i->m_name == name)
		{
			return i;
		}
	}
	return nullptr;
}

BlockDefinition* BlockDefinition::GetByType(unsigned int type)
{
	for (auto& i : s_blockDefList)
	{
		if (i->m_type == type)
		{
			return i;
		}
	}
	return nullptr;
}

bool Block::CanSeeSky() const
{
	return  (m_bitFlags >> BLOCK_BIT_IS_SKY) & 1;
}

bool Block::IsHit() const
{
	return  (m_bitFlags >> BLOCK_BIT_IS_HIT) & 1;
}

bool Block::IsBlockVisible() const
{
	return  (m_bitFlags >> BLOCK_BIT_IS_VISIBLE) & 1;
}

bool Block::IsBlockSolid() const
{
	return  (m_bitFlags >> BLOCK_BIT_IS_SOLID) & 1;
}

bool Block::IsBlockDestructible() const
{
	return  (m_bitFlags >> BLOCK_BIT_IS_DESTRUCTIBLE) & 1;
}

void Block::SetIsSky()
{
	m_bitFlags |= (1 << BLOCK_BIT_IS_SKY);
}

void Block::SetIsNotSky()
{
	m_bitFlags &= ~(1 << BLOCK_BIT_IS_SKY);
}

void Block::SetIsHit()
{
	m_bitFlags |= (1 << BLOCK_BIT_IS_HIT);
}

void Block::SetIsNotHit()
{
	m_bitFlags &= ~(1 << BLOCK_BIT_IS_HIT);
}

Chunk::Chunk(Map* map, int coord)
	:m_map(map), m_coord(coord)
{
	m_blocks = new Block[TOTAL_BLOCKS_SIZE];
	m_cpuMeshVerts.reserve(25000);

	m_bound = AABB2((float)(m_coord * CHUNK_SIZE_X), 0.f, (float)((m_coord + 1) * CHUNK_SIZE_X), CHUNK_SIZE_Y);

	AABB2 boundDebug = m_bound;
	boundDebug.m_mins += Vec2(0.5f, 0.5f);
	boundDebug.m_maxs += Vec2(0.5f, 0.5f);
	AddVertsForAABB2DOutline(m_debugVerts, boundDebug, Rgba8(148, 0, 211, 100), 1.f);
	m_cpuMeshVerts.clear();
}

Chunk::~Chunk()
{
	if (m_leftNeighbor)
	{
		m_leftNeighbor->m_rightNeighbor = nullptr;
		m_leftNeighbor = nullptr;
	}

	if (m_rightNeighbor)
	{
		m_rightNeighbor->m_leftNeighbor = nullptr;
		m_rightNeighbor = nullptr;
	}

	if (m_gpuMeshVBO)
	{
		delete m_gpuMeshVBO;
	}
	m_gpuMeshVBO = nullptr;

	delete[] m_blocks;
	m_blocks = nullptr;
}

void Chunk::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);
}

void Chunk::Render() const
{
	if (!m_isActive)
	{
		return;
	}

	if (m_gpuMeshVBO && !m_cpuMeshVerts.empty())
	{
		g_theRenderer->DrawVertexBuffer(m_gpuMeshVBO, (int)m_cpuMeshVerts.size(), 0, VertexType::Vertex_PCU);
	}
}

IntVec2 Chunk::GetBlockGlobalCoord(IntVec2 local)
{
	return IntVec2(m_coord * CHUNK_SIZE_X + local.x, local.y);
}

IntVec2 Chunk::GetBlockLocalCoord(IntVec2 global)
{
	int x = global.x - m_coord * CHUNK_SIZE_X;
	return IntVec2(x, global.y);
}

Block* Chunk::GetBlock(IntVec2 coord)
{
	int blockIndex = GetBlockIndex(coord);
	GUARANTEE_OR_DIE(blockIndex >= 0 && blockIndex < TOTAL_BLOCKS_SIZE, "Bad block index");
	return &m_blocks[blockIndex];
}

Block* Chunk::GetBlock(int index)
{
	GUARANTEE_OR_DIE(index >= 0 && index < TOTAL_BLOCKS_SIZE, "Bad block index");
	return &m_blocks[index];
}

int Chunk::GetBlockIndexFromGlobalPosition(Vec2 globalPosition)
{
	int x = RoundDownToInt(globalPosition.x) - m_coord * CHUNK_SIZE_X;
	if (x < 0)
	{
		x = RoundDownToInt(globalPosition.x);
	}
	int index = GetBlockIndex(IntVec2(x, (int)globalPosition.y));

	return index;
}

void Chunk::SetBlockType(IntVec2 blockCoord, unsigned int type)
{
	SetBlockType(GetBlockIndex(blockCoord), type);
}

void Chunk::SetBlockType(int index, unsigned int type)
{
	m_blocks[index].m_bitFlags = 0;

	BlockDefinition* blockDef = BlockDefinition::GetByType(type);
	m_blocks[index].m_type = blockDef->m_type;
	if (blockDef->m_isVisible)
	{
		m_blocks[index].m_bitFlags |= (1 << BLOCK_BIT_IS_VISIBLE);
	}
	if (blockDef->m_isSolid)
	{
		m_blocks[index].m_bitFlags |= (1 << BLOCK_BIT_IS_SOLID);
	}
	if (blockDef->m_isDestructible)
	{
		m_blocks[index].m_bitFlags |= (1 << BLOCK_BIT_IS_DESTRUCTIBLE);
	}
}

void Chunk::SetBlockFrontgroundType(int index, unsigned int type)
{
	BlockDefinition* blockDef = BlockDefinition::GetByType(type);
	m_blocks[index].m_frontGroundType = blockDef->m_type;
}

void Chunk::SetBlockBackgroundType(int index, unsigned int type)
{
	BlockDefinition* blockDef = BlockDefinition::GetByType(type);
	m_blocks[index].m_backGroundType = blockDef->m_type;
}

void Chunk::DestructBlocksInDisc(Vec2 pos, float radius, bool changeTerrainType, AffectedTilesSet set)
{
	std::vector<int> affectedTiles;
	for (int i = 0; i < TOTAL_BLOCKS_SIZE; i++)
	{
		IntVec2 blockLocallCoord = GetBlockCoord(i);
		IntVec2 blockGlobalCoord = GetBlockGlobalCoord(blockLocallCoord);
		Vec2 blockGlobalCenter = Vec2((float)blockGlobalCoord.x + 0.5f, (float)blockGlobalCoord.y + 0.5f);

		if (IsPointInsideDisc2D(blockGlobalCenter, pos, radius))
		{
			if (m_blocks[i].IsBlockDestructible())
			{
				SetBlockType(i, 0);

				BlockIterator iter = BlockIterator(this, i);
				if (iter.GetUpNeighbor().GetBlock() && iter.GetUpNeighbor().GetBlock()->CanSeeSky())
				{
					BlockIterator downwardBlock = iter;
					while (downwardBlock.GetBlock() && !downwardBlock.GetBlock()->IsBlockSolid())
					{
						downwardBlock.GetBlock()->SetIsSky();
						downwardBlock = downwardBlock.GetDownNeighbor();
					}
				}
			}
		}

		if (IsPointInsideDisc2D(blockGlobalCenter, pos, radius + 2.f))
		{
			if (m_blocks[i].IsBlockSolid() && m_blocks[i].IsBlockDestructible() && m_blocks[i].IsBlockVisible())
			{
				m_blocks[i].SetIsHit();
				affectedTiles.push_back(i);
			}
		}
	}

	if (changeTerrainType)
	{
		if (set == AffectedTilesSet::EXPLOSION) ChangeAffectedTiles(affectedTiles, 13, 14, 15, 16, 17);
		if (set == AffectedTilesSet::THOR) ChangeAffectedTiles(affectedTiles, 18, 19, 20, 21, 22);
		if (set == AffectedTilesSet::HELA) ChangeAffectedTiles(affectedTiles, 23, 24, 25, 23, 24);
		if (set == AffectedTilesSet::ADAM) ChangeAffectedTiles(affectedTiles, 26, 27, 26, 27, 26);
		if (set == AffectedTilesSet::HULK) ChangeAffectedTiles(affectedTiles, 28, 29, 30, 31, 28);

	}

	for (int i = 0; i < TOTAL_BLOCKS_SIZE; i++)
	{
		if (m_blocks[i].m_type == 8) //IS WATER
		{
			BlockIterator waterBlock = BlockIterator(this, i);
			BlockIterator below = waterBlock.GetDownNeighbor();

			while (below.GetBlock() && below.GetBlock()->m_type == 0)
			{
				SetBlockType(below.m_blockIndex, 8);
				below = below.GetDownNeighbor();
			}
		}
	}

	UpdateTerrainHeight();
	SetMeshDirty();
}

void Chunk::ChangeAffectedTiles(std::vector<int> listIndex, int rightType, int upType, int leftType, int highVolType, int lowVolType)
{
	for (int index : listIndex)
	{
		BlockIterator iter = BlockIterator(this, index);

		if (iter.GetLeftNeighbor().GetBlock() && !iter.GetLeftNeighbor().GetBlock()->IsBlockVisible())
		{
			SetBlockType(index, leftType); // LEFT FACE EXPLOSION
			continue;
		}
		if (iter.GetRightNeighbor().GetBlock() && !iter.GetRightNeighbor().GetBlock()->IsBlockVisible())
		{
			SetBlockType(index, rightType); // RIGHT FACE EXPLOSION
			continue;
		}
		if (iter.GetUpNeighbor().GetBlock() && !iter.GetUpNeighbor().GetBlock()->IsBlockVisible())
		{
			SetBlockType(index, upType); // UP FACE EXPLOSION
			continue;
		}

		int numberOfHitBlocksAround;

		iter.GetNumberOfBlockAround(BLOCK_BIT_IS_HIT, numberOfHitBlocksAround);

		if (numberOfHitBlocksAround >= 2)
		{
			SetBlockType(index, highVolType); // High EXPLOSION
			continue;
		}
		if (numberOfHitBlocksAround == 1)
		{
			SetBlockType(index, lowVolType); // Low EXPLOSION
			continue;
		}
	}
}

void Chunk::UpdateTerrainHeight()
{
	for (int x = 0; x < CHUNK_SIZE_X; x++)
	{
		int topLineIndex = GetBlockIndex(IntVec2(x, CHUNK_SIZE_Y - 1));
		BlockIterator iter = BlockIterator(this, topLineIndex);

		for (;;)
		{
			iter = iter.GetDownNeighbor();

			if (iter.GetBlock() && iter.GetBlock()->IsBlockSolid())
			{
				IntVec2 blockCoord = GetBlockCoord(iter.m_blockIndex);
				m_terrainHeightList[blockCoord.x] = blockCoord.y;
				break;
			}
			else
			{
				m_terrainHeightList[x] = 0;
				break;
			}
		}
	}
}

void Chunk::SetMeshDirty()
{
	for (auto& dirtyMeshChunk : m_map->m_dirtyMeshChunks)
	{
		if (dirtyMeshChunk == this)
		{
			m_isDirty = true;
			return;
		}
	}

	m_map->m_dirtyMeshChunks.push_back(this);
	m_isDirty = true;
}

void Chunk::GenerateData()
{
	int waterlevel = CHUNK_SIZE_Y / 6;
	m_terrainHeightList.reserve(CHUNK_SIZE_X);
	m_hillinessList.reserve(CHUNK_SIZE_X);
	std::vector<int> randomDirtBelowList;

	for (int x = 0; x < CHUNK_SIZE_X; x++)
	{
		int globalX = m_coord * CHUNK_SIZE_X + x;
		float fglobalX = (float)globalX;

		float terrainHeightNoise = 0.5f + 0.5f * Compute1dPerlinNoise(fglobalX, 700.f, 9, 0.5f, 2.0f, false, m_map->m_worldSeed);
		float hillness = 0.5f + 0.5f * Compute1dPerlinNoise(fglobalX, 120.f, 15, 0.5f, 2.0f, false, m_map->m_worldSeed + 1);
		float oceanness = 0.5f + 0.5f * Compute1dPerlinNoise(fglobalX, 2000.f, 1, 0.5f, 2.0f, false, m_map->m_worldSeed + 2);
		float humidity = 0.5f + 0.5f * Compute1dPerlinNoise(fglobalX, 500.f, 6, 0.5f, 2.0f, false, m_map->m_worldSeed + 3);
		float temperature = 0.5f + 0.5f * Compute1dPerlinNoise(fglobalX, 900.f, 7, 0.5f, 2.0f, false, m_map->m_worldSeed + 4);
		float forestness = 0.5f + 0.5f * Compute1dPerlinNoise(fglobalX, 100.f, 7, 0.5f, 5.0f, false, m_map->m_worldSeed + 5);
		float lavaness = 0.5f + 0.5f * Compute1dPerlinNoise(fglobalX, 500.f, 1, 1.5f, 1.0f, false, m_map->m_worldSeed + 6);


		hillness = SmoothStep3(SmoothStep3(hillness));
		//lavaness = SmoothStep3(lavaness);
		oceanness = SmoothStep3(SmoothStep3(oceanness));
		//forestness = SmoothStart6(forestness);
		humidity = 1.f - (1.f - humidity) * (1.f - oceanness);

		float theoreticalTerrainHeightAboveRiver = g_gameConfigBlackboard.GetValue("baseHeight", 20) * terrainHeightNoise;

		if (RoundDownToInt(theoreticalTerrainHeightAboveRiver) >= g_gameConfigBlackboard.GetValue("minHeightHill", 5))
		{
			theoreticalTerrainHeightAboveRiver *= hillness;
		}

		int actualTerrainHeight = g_gameConfigBlackboard.GetValue("guaranteeHeight", 20) + RoundDownToInt(theoreticalTerrainHeightAboveRiver);

		//if (oceanness >= 0.7f)
		//{
		//	if (oceanness >= 0.95f)
		//	{
		//		actualTerrainHeight -= g_gameConfigBlackboard.GetValue("maxOceanDepth", 39);
		//	}
		//	else
		//	{
		//		float oceanLoweringStrength = RangeMapClamped(oceanness, 0.5f, 0.75f, 0.f, 1.f);
		//		int oceanDepth = int((float)g_gameConfigBlackboard.GetValue("maxOceanDepth", 40) * oceanLoweringStrength);
		//		actualTerrainHeight -= oceanDepth;
		//	}
		//}

		float radomDirtNum = Compute1dPerlinNoise(fglobalX, 200.f, 4, 0.5f, 2.0f, false, m_map->m_worldSeed);
		randomDirtBelowList.push_back((int)RangeMapClamped(radomDirtNum, -1.f, 1.f, 3.f, 4.f));

		m_terrainHeightList.push_back(actualTerrainHeight);
		m_hillinessList.push_back(hillness);
		m_oceannessList.push_back(oceanness);
		m_humidityList.push_back(humidity);
		m_temperatureList.push_back(temperature);
		m_forestnessList.push_back(forestness);
		m_lavanessList.push_back(lavaness);
	}

	for (int y = 0; y < CHUNK_SIZE_Y; y++)
	{
		for (int x = 0; x < CHUNK_SIZE_X; x++)
		{
			int noiseIndex = x;
			int blockIndex = GetBlockIndex(IntVec2(x, y));

			if (y > m_terrainHeightList[noiseIndex])
			{
				if (m_terrainHeightList[noiseIndex] <= waterlevel && y <= waterlevel)
				{
					if (m_temperatureList[noiseIndex] < 0.4f)
					{
						if (m_oceannessList[noiseIndex] > 0.5f)
						{
							SetBlockType(blockIndex, 12); // GLACIER
						}
						else
						{
							SetBlockType(blockIndex, 11); // ICE
						}
					}
					else
					{
						SetBlockType(blockIndex, 8); // WATER
					}
				}
				else
				{
					SetBlockType(blockIndex, 0); // AIR
				}

			}
			else
			{

				if (y == m_terrainHeightList[noiseIndex])
				{
					if (m_humidityList[noiseIndex] < 0.6f && y == waterlevel)
					{
						SetBlockType(blockIndex, 10); // SAND
					}
					else
					{
						if (m_humidityList[noiseIndex] < 0.4f)
						{
							SetBlockType(blockIndex, 10); // SAND
						}
						else
						{
							if (m_temperatureList[noiseIndex] < 0.4f)
							{
								SetBlockType(blockIndex, 9); // SNOW GRASS
							}
							else
							{
								SetBlockType(blockIndex, 1); // GRASS
							}
						}
					}
				}
				else
				{
					if (y >= m_terrainHeightList[noiseIndex] - randomDirtBelowList[noiseIndex])
					{
						if (m_humidityList[noiseIndex] < 0.4f)
						{
							SetBlockType(blockIndex, 10); // SAND
						}
						else
						{
							SetBlockType(blockIndex, 2); // DIRT
						}
					}
					else
					{
						if (m_humidityList[noiseIndex] < 0.4f)
						{
							SetBlockType(blockIndex, 10); // SAND
						}
						else
						{
							if (g_theRNG->RollRandomChance(0.05f))
							{
								SetBlockType(blockIndex, 4); // COAL
							}
							else
							{
								if (g_theRNG->RollRandomChance(0.02f))
								{
									SetBlockType(blockIndex, 5); // IRON
								}
								else
								{
									if (g_theRNG->RollRandomChance(0.005f) && y < 30)
									{
										SetBlockType(blockIndex, 6); // GOLD
									}
									else
									{
										if (g_theRNG->RollRandomChance(0.001f) && y < 15)
										{
											SetBlockType(blockIndex, 7); // DIAMOND
										}
										else
										{
											SetBlockType(blockIndex, 3); // STONE
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}


	for (int x = 0; x < CHUNK_SIZE_X; x++)
	{
		int bottomLineIndex = GetBlockIndex(IntVec2(x, 0));
		SetBlockType(bottomLineIndex, 3); // STONE

		int topLineIndex = GetBlockIndex(IntVec2(x, CHUNK_SIZE_Y - 1));
		BlockIterator iter = BlockIterator(this, topLineIndex);

		for (;;)
		{
			iter.GetBlock()->SetIsSky();
			iter = iter.GetDownNeighbor();

			if (iter.GetBlock()->IsBlockSolid())
			{
				IntVec2 blockCoord = GetBlockCoord(iter.m_blockIndex);
				m_terrainHeightList[blockCoord.x] = blockCoord.y;
				break;
			}
		}
	}
}

void Chunk::CreateBlockMesh(BlockIterator blockIter)
{
	Block* block = blockIter.GetBlock();

	if (!block->IsBlockVisible())
	{
		return;
	}

	AABB2 uv = m_map->m_worldSpriteSheet->GetSpriteUVs(m_map->m_worldSpriteSheet->GetSpriteIndex(BlockDefinition::s_blockDefList[block->m_type]->m_spriteCoord));

	Vec2 worldCoordPos = blockIter.GetWorldCenter() - Vec2(0.5f, 0.5f);

	AddVertsForAABB2D(m_cpuMeshVerts, AABB2(worldCoordPos, worldCoordPos + Vec2(1.f, 1.f)), Rgba8::COLOR_WHITE, uv.m_mins, uv.m_maxs);
}

bool Chunk::BuildCPUMesh()
{
	if (m_gpuMeshVBO)
	{
		delete m_gpuMeshVBO;
		m_gpuMeshVBO = nullptr;
	}

	m_cpuMeshVerts.clear();

	for (int i = 0; i < TOTAL_BLOCKS_SIZE; i++)
	{
		CreateBlockMesh(BlockIterator(this, i));
	}
	if (!m_cpuMeshVerts.empty())
	{
		m_gpuMeshVBO = g_theRenderer->CreateVertexBuffer(sizeof(Vertex_PCU) * (int)m_cpuMeshVerts.size());
		g_theRenderer->CopyCPUToGPU(m_cpuMeshVerts.data(), (int)(m_cpuMeshVerts.size() * sizeof(Vertex_PCU)), m_gpuMeshVBO);

		m_isDirty = false;

		return true;
	}
	else
	{
		if (m_gpuMeshVBO)
		{
			delete m_gpuMeshVBO;
			m_gpuMeshVBO = nullptr;
		}
	}

	return false;
}

Vec2 Chunk::GetWorldCenter() const
{
	return Vec2((float)(CHUNK_SIZE_X * m_coord) + (float)CHUNK_SIZE_X * 0.5f, (float)CHUNK_SIZE_Y * 0.5f);
}

BlockIterator::BlockIterator(Chunk* chunk, int blockIndex)
	:m_chunk(chunk), m_blockIndex(blockIndex)
{

}

Block* BlockIterator::GetBlock() const
{
	if (!m_chunk)
	{
		return nullptr;
	}
	if (m_blockIndex < 0 || m_blockIndex > TOTAL_BLOCKS_SIZE - 1) return nullptr;
	return m_chunk->GetBlock(m_blockIndex);
}

Vec2 BlockIterator::GetWorldCenter() const
{
	IntVec2 blockCoord = GetBlockCoord(m_blockIndex);
	return Vec2((float)(m_chunk->m_coord * CHUNK_SIZE_X + blockCoord.x) + 0.5f, (float)blockCoord.y + 0.5f);
}

BlockIterator BlockIterator::GetUpNeighbor() const
{
	if (m_chunk == nullptr || (m_blockIndex + CHUNK_SIZE_X) >> X_BITS >= CHUNK_SIZE_Y)
	{
		return BlockIterator(nullptr, -1);
	}

	return BlockIterator(m_chunk, m_blockIndex + CHUNK_SIZE_X);
}

BlockIterator BlockIterator::GetDownNeighbor() const
{
	if (m_chunk == nullptr || (m_blockIndex - CHUNK_SIZE_X) >> X_BITS < 0)
	{
		return BlockIterator(nullptr, -1);
	}

	return BlockIterator(m_chunk, m_blockIndex - CHUNK_SIZE_X);
}

BlockIterator BlockIterator::GetLeftNeighbor() const
{
	if (m_chunk == nullptr)
	{
		return BlockIterator(nullptr, -1);
	}

	int localX = m_blockIndex & X_MASK;
	if (localX == 0)
	{
		return BlockIterator(m_chunk->m_leftNeighbor, m_blockIndex | X_MASK);
	}
	else
	{
		return BlockIterator(m_chunk, m_blockIndex - 1);
	}
}

BlockIterator BlockIterator::GetRightNeighbor() const
{
	if (m_chunk == nullptr)
	{
		return BlockIterator(nullptr, -1);
	}

	int localX = m_blockIndex & X_MASK;
	if (localX == X_MASK)
	{
		return BlockIterator(m_chunk->m_rightNeighbor, m_blockIndex & ~X_MASK);
	}
	else
	{
		return BlockIterator(m_chunk, m_blockIndex + 1);
	}
}

void BlockIterator::GetNumberOfBlockAround(int bitCondition, int& out_num) const
{
	out_num = 0;

	if (GetUpNeighbor().GetBlock() && ((GetUpNeighbor().GetBlock()->m_bitFlags >> bitCondition) & 1))
	{
		out_num++;
	}
	if (GetDownNeighbor().GetBlock() && ((GetDownNeighbor().GetBlock()->m_bitFlags >> bitCondition) & 1))
	{
		out_num++;
	}
	if (GetLeftNeighbor().GetBlock() && ((GetLeftNeighbor().GetBlock()->m_bitFlags >> bitCondition) & 1))
	{
		out_num++;
	}
	if (GetRightNeighbor().GetBlock() && ((GetRightNeighbor().GetBlock()->m_bitFlags >> bitCondition) & 1))
	{
		out_num++;
	}
}


Map::Map(Game* game)
	:m_game(game)
{
	m_worldTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Map.png");
	m_worldSpriteSheet = new SpriteSheet(*m_worldTexture, IntVec2(8, 8));
	//m_worldShader = g_theRenderer->CreateShader("Data/Shaders/World");
	//m_gameCBO = g_theRenderer->CreateConstantBuffer(sizeof(GameConstants));

	//SEED
	m_worldSeed = (unsigned int)g_gameConfigBlackboard.GetValue("worldSeed", 0);
	if (m_worldSeed == 0)
	{
		m_worldSeed = g_theRNG->RollRandomUnsignedIntInRange(0, 0xFFFFFFFE);
	}
}

Map::~Map()
{
	FlushEntireWorld();

	if (m_gameCBO)
	{
		delete m_gameCBO;
	}
	m_gameCBO = nullptr;

	for (auto& character : m_characters)
	{
		character->Die();
		character->CleanUp();
		delete character;
		character = nullptr;
	}
	m_characters.clear();
	m_deadCharacters.clear();

	for (auto& proj : m_flyingProjectiles)
	{
		if (proj)
		{
			proj->Die();
			delete proj;
		}
		proj = nullptr;
	}
	m_flyingProjectiles.clear();

	for (auto& proj : m_livingProjectiles)
	{
		if (proj)
		{
			proj->Die();
			delete proj;
		}
		proj = nullptr;
	}
	m_livingProjectiles.clear();

}

void Map::StartUp()
{
	m_dirtyMeshChunks.reserve(50);
}

void Map::Update(float deltaSeconds)
{
	DebugUpdate(deltaSeconds);
	SkyUpdate(deltaSeconds);

	for (const std::pair<int, Chunk*>& chunk : m_activeChunks)
	{
		chunk.second->Update(deltaSeconds);
	}

	BuildChunkDirtyMeshes();

	for (auto& character : m_characters)
	{
		character->Update(deltaSeconds);
	}

	CurrentProjectilesUpdate(deltaSeconds);

	HandleDeadCharacters();
}

void Map::DebugUpdate(float deltaSeconds)
{
	UNUSED(deltaSeconds);

	if (g_theInput->WasKeyJustPressed(KEYCODE_RIGHTBRACKET))
	{
		int index = m_blockDebugMode + 1;
		if (index == BlockDataDebugDrawMode_NUM_MODE)
		{
			index = 0;
		}
		m_blockDebugMode = (BlockDataDebugDrawMode)(index);

	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_LEFTBRACKET))
	{
		int index = m_blockDebugMode - 1;
		if (index == -1)
		{
			index = BlockDataDebugDrawMode_NUM_MODE - 1;
		}
		m_blockDebugMode = (BlockDataDebugDrawMode)(index);
	}
}

void Map::Render() const
{
	//Vec4 worldDayColor = Vec4(Vec3(NormalizeByte((unsigned char)m_skyColor.x), NormalizeByte((unsigned char)m_skyColor.y), NormalizeByte((unsigned char)m_skyColor.z)));
	//float indoorNoise = Compute1dPerlinNoise(m_worldDay, 0.4f, 9);
	//float indoorStrength = RangeMapClamped(indoorNoise, -1.f, 1.f, 0.8f, 1.f);

	//GameConstants gameConstant;
	//gameConstant.b_camWorldPos = Vec4(m_player->m_position);
	//gameConstant.b_skyColor = worldDayColor;
	//gameConstant.b_indoorLightColor = Vec4(1.f, 0.9f, 0.8f, 1.f) * indoorStrength;
	//gameConstant.b_outdoorLightColor = worldDayColor;
	//gameConstant.b_fogStartDist = (m_chunkDeactivationRange - 16.f) * 0.5f;
	//gameConstant.b_fogEndDist = (m_chunkDeactivationRange - 16.f);
	//gameConstant.b_fogMaxAlpha = 1.0f;
	//gameConstant.b_time = m_worldDay;

	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetDepthStencilMode(DepthMode::ENABLED);
	g_theRenderer->SetSamplerMode(SampleMode::POINT_CLAMP);
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture(&m_worldSpriteSheet->GetTexture());
	g_theRenderer->BindShader(m_worldShader);

	//g_theRenderer->CopyCPUToGPU(&gameConstant, sizeof(GameConstants), m_gameCBO);
	//g_theRenderer->BindConstantBuffer(8, m_gameCBO);

	for (const std::pair<int, Chunk*>& chunk : m_activeChunks)
	{
		if (chunk.second && DoAABBsOverlap2D(m_game->m_cameraView, chunk.second->m_bound))
		{
			chunk.second->Render();
		}
	}

	if (g_debugDraw)
	{
		std::vector<Vertex_PCU> idVerts;
		std::vector<Vertex_PCU> colorDebugVerts;

		Rgba8 noColor = Rgba8(255, 0, 0, 150);
		Rgba8 isSkyColor = Rgba8(200, 200, 200, 200);
		Rgba8 nonSkyColor = Rgba8(0, 0, 0, 220);
		Rgba8 destrutibleColor = Rgba8(255, 255, 0, 150);
		Rgba8 solidColor = Rgba8(0, 255, 0, 150);


		for (const std::pair<int, Chunk*>& chunk : m_activeChunks)
		{
			// Chunk Data
			if (chunk.second && DoAABBsOverlap2D(m_game->m_cameraView, chunk.second->m_bound))
			{
				g_theRenderer->BindTexture(nullptr);
				g_theRenderer->BindShader(nullptr);
				g_theRenderer->DrawVertexArray(chunk.second->m_debugVerts.size(), chunk.second->m_debugVerts.data());
				float size = 3.f;
				g_font->AddVertsForText2D(idVerts, chunk.second->m_bound.GetCenter() - Vec2(size * 0.5f, 0.f), size, Stringf("%i", chunk.second->m_coord), Rgba8::COLOR_VIOLET);

				// Block Data

				for (int i = 0; i < TOTAL_BLOCKS_SIZE; i++)
				{
					BlockIterator iter = BlockIterator(chunk.second, i);
					Vec2 worldCoordPos = iter.GetWorldCenter() - Vec2(0.5f, 0.5f);

					switch (m_blockDebugMode)
					{
					case IS_SKY:
						if (iter.GetBlock()->CanSeeSky())
						{
							AddVertsForAABB2D(colorDebugVerts, AABB2(worldCoordPos, worldCoordPos + Vec2(1.f, 1.f)), isSkyColor);
						}
						else
						{
							AddVertsForAABB2D(colorDebugVerts, AABB2(worldCoordPos, worldCoordPos + Vec2(1.f, 1.f)), nonSkyColor);
						}
						break;
					case IS_DESTRUCTIBLE:
						if (iter.GetBlock()->IsBlockDestructible())
						{
							AddVertsForAABB2D(colorDebugVerts, AABB2(worldCoordPos, worldCoordPos + Vec2(1.f, 1.f)), destrutibleColor);
						}
						else
						{
							AddVertsForAABB2D(colorDebugVerts, AABB2(worldCoordPos, worldCoordPos + Vec2(1.f, 1.f)), noColor);
						}
						break;
					case IS_SOLID:
						if (iter.GetBlock()->IsBlockSolid())
						{
							AddVertsForAABB2D(colorDebugVerts, AABB2(worldCoordPos, worldCoordPos + Vec2(1.f, 1.f)), solidColor);
						}
						else
						{
							AddVertsForAABB2D(colorDebugVerts, AABB2(worldCoordPos, worldCoordPos + Vec2(1.f, 1.f)), noColor);
						}
						break;
					}
				}
				//for (int i = 0; i < CHUNK_SIZE_Y; i++)
				//{
				//	g_worldText->AddVertsForText2D(idVerts, Vec2(chunk.second->m_coord * (float)CHUNK_SIZE_X, (float)i), size - 2.f, Stringf("%i", i), Rgba8::COLOR_PINK);
				//}

				//for (int x = 0; x < CHUNK_SIZE_X; x++)
				//{
				//	AddVertsForAABB2D(colorDebugVerts, AABB2(Vec2((float)(x + chunkCoord * CHUNK_SIZE_X), 0), Vec2((float)(x + 1 + chunkCoord * CHUNK_SIZE_X), (float)chunk.second->m_terrainHeightList[x])), Rgba8(0, 255, 255, 50));
				//}
			}
		}

		g_theRenderer->BindTexture(nullptr);
		g_theRenderer->DrawVertexArray(colorDebugVerts.size(), colorDebugVerts.data());

		g_theRenderer->BindTexture(&g_font->GetTexture());
		g_theRenderer->DrawVertexArray(idVerts.size(), idVerts.data());
	}

	for (auto& character : m_characters)
	{
		character->Render();
	}
	if (m_game->m_playerIDTurn != -1)
	{
		m_characters[m_game->m_playerIDTurn]->Render();
	}

	for (auto& proj : m_flyingProjectiles)
	{
		if (proj)proj->Render();
	}
	for (auto& proj : m_livingProjectiles)
	{
		if (proj)proj->Render();
	}
}

void Map::SkyUpdate(float deltaSeconds)
{
	float deltaDay = (deltaSeconds * m_worldTimeScale) / (60.f * 60.f * 24.f);
	m_worldDay += deltaDay;

	if (g_gameConfigBlackboard.GetValue("debugAlwaysDay", false))
	{
		m_skyColor = Vec3(200, 230, 255);
	}
	else
	{
		float timeFraction = 0.f;
		if (IsNightTime())
		{
			m_skyColor = Vec3(20, 20, 40);
		}
		else
		{
			if (GetCurrentTimeOfDay() < 0.5f)
			{
				timeFraction = RangeMap(GetCurrentTimeOfDay(), 0.25f, 0.5f, 0.f, 1.f);
			}
			else
			{
				timeFraction = RangeMap(GetCurrentTimeOfDay(), 0.75f, 0.5f, 0.f, 1.f);
			}
		}
		m_skyColor = Interpolate(Vec3(20, 20, 40), Vec3(200, 230, 255), timeFraction);
	}


	float lightingNoise = Compute1dPerlinNoise(m_worldDay, 1.f, 9);
	float lightingStrength = RangeMapClamped(lightingNoise, 0.6f, 0.9f, 0.f, 1.f);
	m_skyColor = Interpolate(m_skyColor, Vec3(255.f, 255.f, 255.f), lightingStrength);

	if (g_theInput->IsKeyDown('Y'))
	{
		m_worldTimeScale = 10000.f;
	}
	else
	{
		m_worldTimeScale = 200.f;
	}
}

Chunk* Map::GetChunkAtCoord(int chunkCoord) const
{
	auto found = m_activeChunks.find(chunkCoord);
	if (found != m_activeChunks.end())
	{
		return found->second;
	}

	return nullptr;
}

Chunk* Map::GetChunkAtPosition(Vec2 position) const
{
	int chunkCoord = RoundDownToInt(position.x / (float)CHUNK_SIZE_X);
	return GetChunkAtCoord(chunkCoord);
}

IntVec2 Map::GetBlockLocalCoord(IntVec2 global) const
{
	IntVec2 chunkCoord = IntVec2(RoundDownToInt((float)global.x / (float)CHUNK_SIZE_X), global.y);

	return global - IntVec2(chunkCoord.x * CHUNK_SIZE_X, 0);
}

Chunk* Map::GetChunkAndBlockIndexFromGlobalPosition(int& out_blockIndex, Vec2 position)
{
	int chunkCoord = RoundDownToInt(position.x / (float)CHUNK_SIZE_X);

	Chunk* chunk = m_activeChunks[chunkCoord];
	out_blockIndex = (chunk) ? chunk->GetBlockIndexFromGlobalPosition(position) : -1;

	return chunk;
}

void Map::ActivateChunk(Chunk* newChunk)
{
	int chunkCoords = newChunk->m_coord;

	newChunk->m_leftNeighbor = GetChunkAtCoord(chunkCoords - 1);
	newChunk->m_rightNeighbor = GetChunkAtCoord(chunkCoords + 1);

	if (newChunk->m_leftNeighbor)	newChunk->m_leftNeighbor->m_rightNeighbor = newChunk;
	if (newChunk->m_rightNeighbor)	newChunk->m_rightNeighbor->m_leftNeighbor = newChunk;

	newChunk->m_isActive = true;

	m_activeChunks[chunkCoords] = newChunk;

	newChunk->SetMeshDirty();
}

void Map::DeactivateChunk(Chunk* chunk)
{
	int chunkCoords = chunk->m_coord;

	delete m_activeChunks[chunkCoords];
	m_activeChunks[chunkCoords] = nullptr;
	m_activeChunks.erase(chunkCoords);
}

void Map::BuildChunkDirtyMeshes()
{
	for (auto& dirtyMeshChunk : m_dirtyMeshChunks)
	{
		dirtyMeshChunk->BuildCPUMesh();
	}

	m_dirtyMeshChunks.clear();
}

int Map::GetNumChunks()
{
	return (int)m_activeChunks.size();
}

void Map::FlushEntireWorld()
{
	std::vector<int> chunkNeedToBeDeletedCoord;

	for (const std::pair<int, Chunk*>& chunk : m_activeChunks)
	{
		if (chunk.second) chunkNeedToBeDeletedCoord.emplace_back(chunk.second->m_coord);
	}

	for (int i : chunkNeedToBeDeletedCoord)
	{
		Chunk* chunk = GetChunkAtCoord(i);
		DeactivateChunk(chunk);
	}

	m_activeChunks.clear();
	m_dirtyMeshChunks.clear();
}

bool Map::IsNightTime()
{
	return GetCurrentTimeOfDay() < 0.25f || GetCurrentTimeOfDay() > 0.75f;
}

float Map::GetCurrentTimeOfDay()
{
	return m_worldDay - (float)RoundDownToInt(m_worldDay);
}

void Map::ExplodeAtPosition(Vec2 pos, float radius, bool changeTerrainType, AffectedTilesSet set, bool shakeScreen)
{
	int numExtend = RoundDownToInt(radius / CHUNK_SIZE_X) + 1;
	std::vector<Chunk*> chunkLists;

	Chunk* currentChunk = GetChunkAtPosition(pos);
	if (!currentChunk) return;

	Chunk* currentLeftChunk = currentChunk->m_leftNeighbor;
	Chunk* currentRightChunk = currentChunk->m_rightNeighbor;

	chunkLists.push_back(currentChunk);

	for (size_t i = 0; i < numExtend; i++)
	{	
		if (currentLeftChunk)
		{
			chunkLists.push_back(currentLeftChunk);
			currentLeftChunk = currentLeftChunk->m_leftNeighbor;
		}
		if (currentRightChunk)
		{
			chunkLists.push_back(currentRightChunk);
			currentRightChunk = currentRightChunk->m_rightNeighbor;
		}
	}

	for (auto & chunkList : chunkLists)
	{
		chunkList->DestructBlocksInDisc(pos, radius, changeTerrainType, set);
	}

	m_game->SetCooldownCamera(2.f);

	if (shakeScreen)
	{
		float shakeValue = RangeMapClamped(radius, 0.f, 8.f, 0.f, 2.f);
		m_game->m_screenShakeAmount = shakeValue;
	}
}

void Map::CurrentProjectilesUpdate(float deltaSeconds)
{
	for (auto& livingProjectile : m_livingProjectiles)
	{
		if (livingProjectile)
		{
			livingProjectile->Update(deltaSeconds);
		}
	}
	for (size_t i = 0; i < m_flyingProjectiles.size(); i++)
	{
		Projectile* proj = m_flyingProjectiles[i];
		if (proj)
		{
			proj->Update(deltaSeconds);

			if (proj->m_position.x > m_bound.m_maxs.x
				|| proj->m_position.x < m_bound.m_mins.x
				|| proj->m_position.y < m_bound.m_mins.y
				|| (proj->m_isHit && proj->m_turnAliveLeft <= 0)
				|| proj->m_isDead
				)
			{
				proj->Die();
				delete proj;
				m_flyingProjectiles.erase(m_flyingProjectiles.begin() + i);
				i--;
			}
		}
	}
	if (!m_flyingProjectiles.empty() && m_flyingProjectiles[0] && !m_flyingProjectiles[0]->m_hasTouchedGround)
	{
		m_game->GameCameraFollow(m_flyingProjectiles[0]->m_position, deltaSeconds * 3.f);
	}
}

GameRaycastResult2D Map::RaycastWorld(Vec2 startPos, Vec2 fwdNormal, float maxDist) const
{
	GameRaycastResult2D result;
	result.m_rayStartPos = startPos;
	result.m_rayFwdNormal = fwdNormal;
	result.m_rayMaxLength = maxDist;

	IntVec2 currentBlockGlobalCoord = IntVec2(RoundDownToInt(startPos.x), RoundDownToInt(Clamp(startPos.y, 0.f, (float)CHUNK_SIZE_Y - 1.f)));
	BlockIterator currentBlockIterator;
	currentBlockIterator.m_chunk = GetChunkAtPosition(startPos);
	currentBlockIterator.m_blockIndex = GetBlockIndex(GetBlockLocalCoord(currentBlockGlobalCoord));

	// X
	float fwdDistPerXCrossing = 1.f / fabsf(fwdNormal.x);
	int tileStepDirectionX = (fwdNormal.x > 0) ? 1 : -1;
	float xAtFirstXCrossing = currentBlockGlobalCoord.x + (tileStepDirectionX + 1) / 2.f;
	float xDistToFirstXCrossing = xAtFirstXCrossing - startPos.x;
	float fwdDistAtNextXCrossing = fabsf(xDistToFirstXCrossing) * fwdDistPerXCrossing;

	// Y
	float fwdDistPerYCrossing = 1.f / fabsf(fwdNormal.y);
	int tileStepDirectionY = (fwdNormal.y > 0) ? 1 : -1;
	float yAtFirstYCrossing = currentBlockGlobalCoord.y + (tileStepDirectionY + 1) / 2.f;
	float yDistToFirstYCrossing = yAtFirstYCrossing - startPos.y;
	float fwdDistAtNextYCrossing = fabsf(yDistToFirstYCrossing) * fwdDistPerYCrossing;

	for (;;)
	{
		if (fwdDistAtNextXCrossing < fwdDistAtNextYCrossing)
		{
			if (fwdDistAtNextXCrossing > maxDist)
			{
				result.m_didImpact = false;
				return result;
			}
			if (tileStepDirectionX > 0)
			{
				currentBlockIterator = currentBlockIterator.GetRightNeighbor();
			}
			else
			{
				currentBlockIterator = currentBlockIterator.GetLeftNeighbor();
			}
			if (!currentBlockIterator.m_chunk)
			{
				result.m_didImpact = false;
				return result;
			}
			if (currentBlockIterator.GetBlock()->IsBlockSolid())
			{
				result.m_didImpact = true;
				result.m_blockIterator = currentBlockIterator;
				result.m_impactDist = fwdDistAtNextXCrossing;
				result.m_impactPos = startPos + fwdNormal * result.m_impactDist;
				result.m_impactNormal = Vec2((float)-tileStepDirectionX, 0.f);
				return result;
			}
			else
			{
				fwdDistAtNextXCrossing += fwdDistPerXCrossing;
			}
		}
		else
		{
			if (fwdDistAtNextYCrossing > maxDist)
			{
				result.m_didImpact = false;
				return result;
			}
			if (tileStepDirectionY > 0)
			{
				currentBlockIterator = currentBlockIterator.GetUpNeighbor();
			}
			else
			{
				currentBlockIterator = currentBlockIterator.GetDownNeighbor();
			}
			if (!currentBlockIterator.m_chunk)
			{
				result.m_didImpact = false;
				return result;
			}
			if (currentBlockIterator.GetBlock()->IsBlockSolid())
			{
				result.m_didImpact = true;
				result.m_blockIterator = currentBlockIterator;
				result.m_impactDist = fwdDistAtNextYCrossing;
				result.m_impactPos = startPos + fwdNormal * result.m_impactDist;
				result.m_impactNormal = Vec2(0.f, (float)-tileStepDirectionY);
				return result;
			}
			else
			{
				fwdDistAtNextYCrossing += fwdDistPerYCrossing;
			}
		}

	}

	return result;
}


Character* Map::SpawnCharacter(CharacterDefinition* characterDef, Vec2 position)
{
	Character* characterPtr = nullptr;
	std::string name = characterDef->m_name;

	if (name == "Hela")
	{
		characterPtr = new Hela_Character(this);
	}
	if (name == "Iron")
	{
		characterPtr = new Iron_Character(this);
	}
	if (name == "Hulk")
	{
		characterPtr = new Hulk_Character(this);
	}
	if (name == "Thor")
	{
		characterPtr = new Thor_Character(this);
	}
	if (name == "IW")
	{
		characterPtr = new IW_Character(this);
	}
	if (name == "Adam")
	{
		characterPtr = new Adam_Character(this);
	}

	if (characterPtr)
	{
		m_characters.push_back(characterPtr);
		characterPtr->m_position = position;
		int chunkID = GetChunkAtPosition(position)->m_coord;
		m_spawnedCharacterChunkID.push_back(chunkID);
	}
	else
	{
		ERROR_AND_DIE("Character Class doesn't exist");
	}


	return characterPtr;
}

void Map::SpawnGrave(Vec2 position, Controller* controller, CharacterDefinition* characterDef)
{
	Grave* grave = new Grave(this);
	grave->m_controller = controller;
	grave->m_position = position;
	grave->m_previousCharacterDefinition = characterDef;
	m_characters.push_back(grave);
}

void Map::HandleDeadCharacters()
{
	for (auto& character : m_deadCharacters)
	{
		if (character->m_health != -1)
		{
			SpawnGrave(character->m_position, character->m_controller, character->m_characterDef);
		}

		for (auto & flyingProjectile : m_flyingProjectiles)
		{
			if (flyingProjectile && flyingProjectile->m_owner == character)
			{
				flyingProjectile->m_owner = nullptr;
			}
		}

		for (auto& livingProjectile : m_livingProjectiles)
		{
			if (livingProjectile && livingProjectile->m_owner == character)
			{
				livingProjectile->m_owner = nullptr;
			}
		}

		character->CleanUp();

		auto findCharacter = std::find(m_characters.begin(), m_characters.end(), character);
		if (findCharacter != m_characters.end())m_characters.erase(findCharacter);
		delete character;
		character = nullptr;
	}


	m_deadCharacters.clear();
}

bool Map::CanThisChunkSpawnCharacter(int chunkID)
{
	for (size_t i = 0; i < m_spawnedCharacterChunkID.size(); i++)
	{
		if (m_spawnedCharacterChunkID[i] == chunkID) return false;
		if (i > 0 && m_spawnedCharacterChunkID[i - 1] == chunkID) return false;
		if (i < m_spawnedCharacterChunkID.size() - 1 && m_spawnedCharacterChunkID[i + 1] == chunkID) return false;
	}

	return true;
}

Map* Map::GenerateRandomMap(Game* game, int totalNumChunk, int seed)
{
	Map* result = new Map(game);
	if (seed == 0)
	{
		result->m_worldSeed = g_theRNG->RollRandomUnsignedIntInRange(0, 0xFFFFFFFE);
	}
	else
	{
		result->m_worldSeed = seed;
	}

	for (int i = 0; i < totalNumChunk; i++)
	{
		Chunk* newChunk = new Chunk(result, i);
		newChunk->GenerateData();
		result->ActivateChunk(newChunk);
	}

	result->m_bound = AABB2(0, 0, (float)(totalNumChunk * CHUNK_SIZE_X), (float)CHUNK_SIZE_Y);

	return result;
}
