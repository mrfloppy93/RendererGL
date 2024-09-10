//
// Created by lukas on 05.08.24.
//

#include "BoundingBox.h"

#include <algorithm>
#include <cfloat>
#include <glm/ext/matrix_transform.hpp>

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

// create from set of points
BoundingBox::BoundingBox(std::vector<Vec3f> const& points) {
    set(points);
}

//create from maximum and minimum vectors
BoundingBox::BoundingBox(const glm::vec3 &vMin, const glm::vec3 &vMax) {
    set(vMin, vMax);
}

// set from set of points
void BoundingBox::set(std::vector<glm::vec3> const& points) {

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

std::vector<Vec3f> BoundingBox::getPointsVec3f() {
    std::vector<Vec3f> v3f;
    for(auto p: m_points)
    {
        v3f.emplace_back(p.x,p.y,p.z);
    }
    return v3f;
}

void BoundingBox::print() {
    std::cout << "BoundingBox: " << std::endl << "Vertices: " << std::endl;
    for(auto p: m_points)
    {
        std::cout << p.x << " " << p.y << " " << p.z << std::endl;
    }
    std::cout << std::endl;
    std::cout << "vMin: " << m_vMin.x << " " << m_vMin.y << " " << m_vMin.z << std::endl;
    std::cout << "vMax: " <<  m_vMax.x << " " << m_vMax.y << " " << m_vMax.z << std::endl << std::endl;
}

/**
 * Check if this bounding box contains another bounding box
 * @param bb the bounding box to check
 * @return true if this bounding box contains the other bounding box
 */
bool BoundingBox::contains(const BoundingBox::Ptr& bb) {
    return std::all_of(bb->m_points.begin(), bb->m_points.end(), [&](const auto& p) {
        return      p.x < m_vMax.x && p.x > m_vMin.x
                &&  p.y < m_vMax.y && p.y > m_vMin.y
                &&  p.z < m_vMax.z && p.z > m_vMin.z;
    });
}

/**
 * Check if the 1st bounding box intersects with the 2nd bounding box, both in the same coordinate system.
 * @param bb1 the bounding box to check
 * @param bb2 the bounding box to check
 * @return true if the bounding boxes intersect
 */
bool BoundingBox::intersect(const BoundingBox::Ptr& bb1, const BoundingBox::Ptr& bb2) {
    return std::any_of(bb2->m_points.begin(), bb2->m_points.end(), [&](const auto& p) {
        return      p.x < bb1->m_vMax.x && p.x > bb1->m_vMin.x
                &&  p.y < bb1->m_vMax.y && p.y > bb1->m_vMin.y
                &&  p.z < bb1->m_vMax.z && p.z > bb1->m_vMin.z;
    });
}

/**
 * Transform the bounding box by a matrix
 * @param bb the bounding box to transform
 * @param mat the transformation matrix
 * @return a new bounding box transformed by the matrix
 */
BoundingBox::Ptr BoundingBox::transform(const BoundingBox::Ptr &bb, const glm::mat4 &mat) {
    std::vector<glm::vec3> points;
    for(const auto& point: bb->m_points) {
        auto transformed = mat * glm::vec4(point, 1.0);
        points.emplace_back(transformed/transformed.w);
    }
    return BoundingBox::New(points);
}

/**
 * Merge two bounding boxes within the same coordinate system
 * @param bb1 the first bounding box
 * @param bb2 the second bounding box
 * @return a new bounding box that is the merge of the two bounding boxes
 */
BoundingBox::Ptr BoundingBox::merge(const BoundingBox::Ptr &bb1, const BoundingBox::Ptr &bb2) {
    auto min = glm::vec3(std::numeric_limits<float>::max());
    auto max = glm::vec3(-std::numeric_limits<float>::max());

    min.x = std::min(bb1->m_vMin.x, bb2->m_vMin.x);
    min.y = std::min(bb1->m_vMin.y, bb2->m_vMin.y);
    min.z = std::min(bb1->m_vMin.z, bb2->m_vMin.z);

    max.x = std::max(bb1->m_vMax.x, bb2->m_vMax.x);
    max.y = std::max(bb1->m_vMax.y, bb2->m_vMax.y);
    max.z = std::max(bb1->m_vMax.z, bb2->m_vMax.z);

    return BoundingBox::New(min, max);
}

/**
 * Crop a bounding box within a set of boundaries
 * @param source the bounding box to crop
 * @param boundaries the boundaries to crop the bounding box within
 * @return a new bounding box that is the cropped version of the source bounding box
 */
BoundingBox::Ptr BoundingBox::crop(const BoundingBox::Ptr &source, const BoundingBox::Ptr &boundaries) {
    auto min = glm::vec3(std::numeric_limits<float>::max());
    auto max = glm::vec3(-std::numeric_limits<float>::max());

    min.x = std::max(source->m_vMin.x, boundaries->m_vMin.x);
    min.y = std::max(source->m_vMin.y, boundaries->m_vMin.y);
    min.z = std::max(source->m_vMin.z, boundaries->m_vMin.z);

    max.x = std::min(source->m_vMax.x, boundaries->m_vMax.x);
    max.y = std::min(source->m_vMax.y, boundaries->m_vMax.y);
    max.z = std::min(source->m_vMax.z, boundaries->m_vMax.z);

    return BoundingBox::New(min, max);
}




