//
// Created by lukas on 22.08.24.
//

#include "BBVisuals.h"

#include "../shapes/Shape.h"

std::vector<unsigned int> indices = {
    // Bottom face (vMin.y)
    0, 1, 2,   1, 3, 2,

    // Top face (vMax.y)
    4, 5, 6,   5, 7, 6,

    // Front face (vMin.z)
    0, 1, 4,   1, 5, 4,

    // Back face (vMax.z)
    2, 3, 6,   3, 7, 6,

    // Left face (vMin.x)
    0, 2, 4,   2, 6, 4,

    // Right face (vMax.x)
    1, 3, 5,   3, 7, 5
};

BBVisuals::BBVisuals(const Polytope::Ptr& polytope) {
    std::vector<unsigned int> indices = {
        // Bottom face (vMin.y)
        0, 1, 2,   1, 3, 2,

        // Top face (vMax.y)
        4, 5, 6,   5, 7, 6,

        // Front face (vMin.z)
        0, 1, 4,   1, 5, 4,

        // Back face (vMax.z)
        2, 3, 6,   3, 7, 6,

        // Left face (vMin.x)
        0, 2, 4,   2, 6, 4,

        // Right face (vMax.x)
        1, 3, 5,   3, 7, 5
    };

    std::vector<Vec3f> bbPoints = polytope->getBoundingBox()->getPointsVec3f();
    const Shape::Ptr polyBB = Shape::New(bbPoints, indices);
    polyBB->setFaceCulling(Polytope::FaceCulling::NONE);
    add(polyBB);
    setShowWire(true);
    setShadowCaster(false);
    setModelMatrix(polytope->getModelMatrix());
}
