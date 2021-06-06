#include "camera.h"

void Camera::lookAt(const Point & center,const float size,const Point & orientation)
{
    m_center= center;
    m_position= vec2(0, 0);
    m_rotation= vec2(orientation.y,orientation.z);
    m_size= size;
    m_radius= size*50;
}
