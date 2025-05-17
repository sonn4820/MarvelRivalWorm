#include "Game/CharacterCommon.hpp"
#include "Game/GameCommon.hpp"
#include "Game/AllCharacters.hpp"

std::vector<ItemDefinition*> ItemDefinition::s_itemDefList;

ItemDefinition::ItemDefinition(XmlElement& element)
	:m_name(ParseXmlAttribute(element, "name", ""))
{

}


void ItemDefinition::SetVisual(XmlElement& element)
{
	m_visualHalfLength = ParseXmlAttribute(element, "halfLength", Vec2());
	m_itemTextureName = ParseXmlAttribute(element, "image", "");
	m_offsetAngle = ParseXmlAttribute(element, "offsetAngle", 0.f);
}

void ItemDefinition::InitializeDefs(char const* filePath)
{
	XmlDocument file;
	XmlError result = file.LoadFile(filePath);
	GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS, "FILE IS NOT LOADED");

	XmlElement* rootElement = file.RootElement();
	GUARANTEE_OR_DIE(rootElement, "Root Element is null");

	XmlElement* itemDefElement = rootElement->FirstChildElement();

	while (itemDefElement)
	{
		std::string name = itemDefElement->Name();
		GUARANTEE_OR_DIE(name == "ItemDefinition", "Root child element is in the wrong format");

		ItemDefinition* newItemDef = new ItemDefinition(*itemDefElement);

		XmlElement* actorChildElement = itemDefElement->FirstChildElement();
		while (actorChildElement)
		{
			std::string sectionName = actorChildElement->Name();
			if (sectionName == "Visuals")
			{
				newItemDef->SetVisual(*actorChildElement);
			}
			actorChildElement = actorChildElement->NextSiblingElement();
		}
		s_itemDefList.push_back(newItemDef);
		itemDefElement = itemDefElement->NextSiblingElement();
	}
}

void ItemDefinition::ClearDefinition()
{
	for (auto& i : s_itemDefList)
	{
		if (i != nullptr)
		{
			delete i;
			i = nullptr;
		}
	}
}

ItemDefinition* ItemDefinition::GetByName(std::string name)
{
	for (auto& i : s_itemDefList)
	{
		if (i->m_name == name)
		{
			return i;
		}
	}
	return nullptr;
}


Item::Item(Map* map, Character* owner, ItemDefinition* def)
	:Entity(map), m_itemDef(def), m_owner(owner)
{
	AABB2 bound = AABB2(Vec2::ZERO, m_itemDef->m_visualHalfLength.y, m_itemDef->m_visualHalfLength.x);
	AddVertsForAABB2D(m_verts, bound, Rgba8::COLOR_WHITE);
	m_vbuffer = g_theRenderer->CreateVertexBuffer(sizeof(Vertex_PCU) * (int)m_verts.size());
	g_theRenderer->CopyCPUToGPU(m_verts.data(), (int)(m_verts.size() * sizeof(Vertex_PCU)), m_vbuffer);

	m_sprite = g_theRenderer->CreateOrGetTextureFromFile(m_itemDef->m_itemTextureName.c_str());

	m_orientationDegrees = m_itemDef->m_offsetAngle;
}

Item::~Item()
{

}

void Item::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);
}

void Item::Render() const
{
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetDepthStencilMode(DepthMode::ENABLED);
	g_theRenderer->SetSamplerMode(SampleMode::POINT_CLAMP);
	g_theRenderer->BindTexture(m_sprite);
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexBuffer(m_vbuffer, m_verts.size());

	if (g_debugDraw)
	{
	
	}
}

void Item::Die()
{
	m_isDead = true;
}