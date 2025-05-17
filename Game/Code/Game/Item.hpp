#pragma once
#include "Game/GameCommon.hpp"
#include "Game/Entity.hpp"

class Game;
class Character;

struct ItemDefinition
{
	std::string m_name = "";
	Vec2 m_visualHalfLength = Vec2();
	std::string	m_itemTextureName = "";
	float m_offsetAngle = 0.f;

	ItemDefinition(XmlElement& element);
	void SetVisual(XmlElement& element);

	static void InitializeDefs(char const* filePath);
	static void ClearDefinition();
	static ItemDefinition* GetByName(std::string name);
	static std::vector<ItemDefinition*> s_itemDefList;
};

class Item : public Entity
{
public:
	Item(Map* map, Character* owner, ItemDefinition* def);
	virtual ~Item();

	void Update(float deltaSeconds) override;
	void Render() const override;
	void Die() override;

public:
	ItemDefinition* m_itemDef = nullptr;
	Character* m_owner = nullptr;
	Mat44 m_matrixToRender;
};

