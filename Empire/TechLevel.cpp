
#include "TechLevel.h"

TechLevel::TechLevel(int ID, std::string& name, int MinPts) :
    m_id(ID),
    m_name(name),
    m_min_pts(MinPts)
{
    // no other stuff to do
}


int TechLevel::GetID() const
{
    return m_id;
}

int TechLevel::GetMinPts() const
{
    return m_min_pts;
}

const std::string& TechLevel::GetName() const
{
    return m_name;
}
