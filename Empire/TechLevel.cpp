#include "TechLevel.h"

Tech::Tech(int ID, std::string name, int MinPts) :
    m_id(ID),
    m_min_pts(MinPts),
    m_name(name)
{
}

int Tech::GetID() const
{
    return m_id;
}

int Tech::GetMinPts() const
{
    return m_min_pts;
}

const std::string& Tech::GetName() const
{
    return m_name;
}
