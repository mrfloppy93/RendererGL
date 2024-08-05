//
// Created by lukas on 05.08.24.
//

#include "BoundingBox.h"

#include <cfloat>

BoundingBox::BoundingBox() {
    m_vMin = glm::vec3(std::numeric_limits<float>::max());
    m_vMax = glm::vec3(-std::numeric_limits<float>::max());
}

BoundingBox::BoundingBox(BoundingBox &bb) {
    set(bb.m_vMin, bb.m_vMax);
}

// create from set of points
BoundingBox::BoundingBox(std::vector<glm::vec3> const& points) {
    set(points);
}

//create from maximum and minimum vectors
BoundingBox::BoundingBox(const glm::vec3 &vMin, const glm::vec3 &vMax) {
    set(vMin, vMax);
}

// set from set of points
bool BoundingBox::set(std::vector<glm::vec3> const& points) {

    m_vMin = glm::vec3(std::numeric_limits<float>::max());
    m_vMax = glm::vec3(-std::numeric_limits<float>::max());

    // calculate min and max vectors
    for(const auto p: points) {
        if(p.x < m_vMin.x) m_vMin.x = p.x;
        if(p.y < m_vMin.y) m_vMin.y = p.y;
        if(p.z < m_vMin.z) m_vMin.z = p.z;

        if(p.x > m_vMax.x) m_vMax.x = p.x;
        if(p.y > m_vMax.y) m_vMax.y = p.y;
        if(p.z > m_vMax.z) m_vMax.z = p.z;
    }

    set(m_vMin, m_vMax);

}

//set from maximum and minimum vectors
void BoundingBox::set(const glm::vec3 &vMin, const glm::vec3 &vMax) {
    m_points.clear();

    m_points.emplace_back(vMin.x, vMin.y, vMin.z);
    m_points.emplace_back(vMax.x, vMin.y, vMin.z);
    m_points.emplace_back(vMin.x, vMin.y, vMax.z);
    m_points.emplace_back(vMax.x, vMin.y, vMax.z);
    m_points.emplace_back(vMin.x, vMax.y, vMin.z);
    m_points.emplace_back(vMax.x, vMax.y, vMin.z);
    m_points.emplace_back(vMin.x, vMax.y, vMax.z);
    m_points.emplace_back(vMax.x, vMax.y, vMax.z);

    m_vMin = vMin;
    m_vMax = vMax;
}

void BoundingBox::set(std::vector<Vec3f> const& vertices) {

    m_vMin = glm::vec3(std::numeric_limits<float>::max());
    m_vMax = glm::vec3(-std::numeric_limits<float>::max());

    // calculate min and max vectors
    for(const auto p: vertices) {
        if(p.x < m_vMin.x) m_vMin.x = p.x;
        if(p.y < m_vMin.y) m_vMin.y = p.y;
        if(p.z < m_vMin.z) m_vMin.z = p.z;

        if(p.x > m_vMax.x) m_vMax.x = p.x;
        if(p.y > m_vMax.y) m_vMax.y = p.y;
        if(p.z > m_vMax.z) m_vMax.z = p.z;
    }

    set(m_vMin, m_vMax);
}

void BoundingBox::operator<<(BoundingBox const& bb1) {
    *this = *this + bb1;
}
