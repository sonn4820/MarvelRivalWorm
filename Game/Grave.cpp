#include "Game/Grave.hpp"

Grave::Grave(Map* map)
	:Character(map)
{
	m_characterDef = CharacterDefinition::GetByName("Grave");
	Initialize();
}

Grave::~Grave()
{

}

void Grave::BeginTurn()
{

}

void Grave::EndTurn()
{

}

void Grave::Passive()
{

}

void Grave::Skill_1()
{

}

void Grave::Skill_2()
{

}

void Grave::Ultimate()
{

}
