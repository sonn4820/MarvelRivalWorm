#pragma once
#include <vector>

struct FloatRange;

class RandomNumberGenerator
{
public:
	RandomNumberGenerator(unsigned int seed = 0) { m_seed = seed; m_position = 0; }
	int RollRandomIntLessThan(int maxNotInclusive);
	int RollRandomIntInRange(int minInclusive, int maxInclusive);
	unsigned int RollRandomUnsignedIntInRange(unsigned int minInclusive, unsigned int maxInclusive);
	float RollRandomFloatZeroToOne();
	float RollRandomFloatMinusOneToOne();
	float RollRandomFloatInRange(float minInclusive, float maxInclusive);
	float RollRandomFloatInRange(FloatRange range);
	bool RollRandomChance(float rate);
	int RollRandomSign();
	template<typename T>
	T RollRandomFromArray(std::vector<T> arr);

	void SetSeed(int seed);

public:
	unsigned int m_seed = 0;
	int m_position = 0;
};

template<typename T>
T RandomNumberGenerator::RollRandomFromArray(std::vector<T> arr)
{
	int index = RollRandomIntInRange(0, (int)arr.size() - 1);
	return arr[index];
}

