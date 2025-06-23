#pragma once
#include "Game/GameCommon.hpp"

class Map;
class Chunk;
class Entity
{
public:
	Entity(Map* owner);
	virtual ~Entity();

	virtual void Update(float deltaSeconds) = 0;
	virtual void Render() const = 0;
	virtual void Die() = 0;

	Mat44 GetModelMatrix() const;
public:
	Map* m_map = nullptr;
	Vec2 m_position;
	Vec2 m_velocity;
	float m_orientationDegrees = 0.f;
	AABB2 m_visualBound;
	bool m_isDead = false;

	Chunk* m_currentChunk = nullptr;

	std::vector<Vertex_PCU> m_verts;
	VertexBuffer* m_vbuffer = nullptr;

	Texture* m_sprite = nullptr;

	Rgba8 m_tintColor = Rgba8::COLOR_WHITE;
};
