#include "Game/Entity.hpp"

Entity::Entity(Map* owner)
	:m_map(owner)
{

}

Entity::~Entity()
{
	delete m_vbuffer;
}

Mat44 Entity::GetModelMatrix() const
{
	Mat44 result = Mat44::CreateTranslation2D(m_position);
	result.AppendZRotation(m_orientationDegrees);
	return	result;
}
