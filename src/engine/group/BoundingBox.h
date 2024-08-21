//
// Created by lukas on 05.08.24.
//
#pragma once

#ifndef BOUNDINGBOX_H
#define BOUNDINGBOX_H
#include <vector>
#include <glm/vec3.hpp>
#include <glm/ext/matrix_float4x4.hpp>

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
    void set(std::vector<glm::vec3> const& points);
    //set from maximum and minimum vectors
    void set(const glm::vec3 &vMin, const glm::vec3 &vMax);
    // set from vertices
    void set(std::vector<Vec3f> const& vertices);

    std::vector<Vec3f> getPointsVec3f();
    void print();
    bool contains(const BoundingBox::Ptr& bb);

    static bool intersect(const BoundingBox::Ptr& bb1, const BoundingBox::Ptr& bb2);
    static BoundingBox::Ptr transform(const BoundingBox::Ptr &bb, const glm::mat4 &mat);
    static BoundingBox::Ptr merge(const BoundingBox::Ptr &bb1, const BoundingBox::Ptr &bb2);
    static BoundingBox::Ptr crop(const BoundingBox::Ptr &source, const BoundingBox::Ptr &boundaries);
};



#endif //BOUNDINGBOX_H
