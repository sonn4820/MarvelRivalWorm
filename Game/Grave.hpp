#pragma once
#include "Game/GameCommon.hpp"
#include "Game/CharacterCommon.hpp"

class Grave : public Character
{
public:
	Grave(Map* map);
	virtual ~Grave();

	void BeginTurn() override;
	void EndTurn() override;
	void Passive()  override;
	void Skill_1()  override;
	void Skill_2()  override;
	void Ultimate() override;

	CharacterDefinition* m_previousCharacterDefinition = nullptr;
};