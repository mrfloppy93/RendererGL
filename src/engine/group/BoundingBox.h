//
// Created by lukas on 05.08.24.
//
#pragma once

#ifndef BOUNDINGBOX_H
#define BOUNDINGBOX_H
#include <vector>
#include <glm/vec3.hpp>

#include "engine/ptr.h"
#include "engine/Vec3.h"


class BoundingBox {
    GENERATE_PTR(BoundingBox)
public:
    std::vector<glm::vec3> m_points;
    glm::vec3 m_vMin, m_vMax;
public:
    explicit BoundingBox();
    BoundingBox(BoundingBox &bb);
    // create from set of points
    explicit BoundingBox(std::vector<glm::vec3> const& points);
    explicit BoundingBox(std::vector<Vec3f> const& points);
    //create from maximum and minimum vectors
    BoundingBox(const glm::vec3 &vMin, const glm::vec3 &vMax);
public:
    // set from set of points
    // points vector needs to include exactly eight vec3
    bool set(std::vector<glm::vec3> const& points);
    //set from maximum and minimum vectors
    void set(const glm::vec3 &vMin, const glm::vec3 &vMax);
    // set from vertices
    void set(std::vector<Vec3f> const& vertices);

    std::vector<Vec3f> getPointsVec3f();
    void print();

    void operator<<(BoundingBox const& bb1);
};

inline BoundingBox operator+(BoundingBox const& bb1, BoundingBox const& bb2) {
    glm::vec3 vMin,vMax;
    vMin.x = std::min(bb1.m_vMin.x, bb2.m_vMin.x);
    vMin.y = std::min(bb1.m_vMin.y, bb2.m_vMin.y);
    vMin.z = std::min(bb1.m_vMin.z, bb2.m_vMin.z);
    vMax.x = std::max(bb1.m_vMax.x, bb2.m_vMax.x);
    vMax.y = std::max(bb1.m_vMax.y, bb2.m_vMax.y);
    vMax.z = std::max(bb1.m_vMax.z, bb2.m_vMax.z);
    return BoundingBox(vMin,vMax);
}

#endif //BOUNDINGBOX_H
