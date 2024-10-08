#include "Group.h"

unsigned long Group::groupCount = 0;

Group::Group(unsigned int _primitive, bool _showWire) 
    : primitive(_primitive), showWire(_showWire), modelMatrix(1.f), visible(true), 
    pointSize(POINT_SIZE), lineWidth(LINE_WIDTH), outliningWidth(OUTLINING_WIDTH),
    id(groupCount), boundingBox(BoundingBox::New()), shadowCaster(true) {
    groupCount ++;
}

Group::Group() 
    : primitive(GL_TRIANGLES), showWire(false), modelMatrix(1.f), visible(true), 
    pointSize(POINT_SIZE), lineWidth(LINE_WIDTH), outliningWidth(OUTLINING_WIDTH),
    id(groupCount), shadowCaster(true), boundingBox(BoundingBox::New()) {
    groupCount ++;
}

void Group::updateBoundingBox() {
    // calculate the boundingbox of all  modelBB
    auto vMin = glm::vec3(std::numeric_limits<float>::max());
    auto vMax = glm::vec3(-std::numeric_limits<float>::max());

    for(const auto& polytope: polytopes) {
        const BoundingBox::Ptr aabb = polytope->getBoundingBox();
        const glm::mat4 modelMatrix = polytope->getModelMatrix();

        // transform min and max vectors into group-space
        auto min = glm::vec4(aabb->m_vMin, 1.0) * modelMatrix;
        auto max = glm::vec4(aabb->m_vMax, 1.0) * modelMatrix;

        // get min and max values in group-space
        if(min.x < vMin.x) vMin.x = min.x;
        if(min.y < vMin.y) vMin.y = min.y;
        if(min.z < vMin.z) vMin.z = min.z;

        if(max.x > vMax.x) vMax.x = max.x;
        if(max.y > vMax.y) vMax.y = max.y;
        if(max.z > vMax.z) vMax.z = max.z;
    }

    boundingBox->set(vMin,vMax);
}

void Group::add(const Polytope::Ptr &polytope) {
    polytopes.push_back(polytope);

    //updateBoundingBox();
}


void Group::removePolytope(Polytope::Ptr& polytope) {
    unsigned int index = 0;
    for(auto& p : polytopes) {
        if(p.get() == polytope.get()) {
            removePolytope(index);
            break;
        }
        index ++;
    }
}

bool Group::isSelected() {
    bool selected = true;
    for(auto& p : polytopes) {
        if(!p->isSelected()) selected = false;
    }
    return selected;
}

void Group::setSelected(bool selected) {
    for(auto& p : polytopes) p->setSelected(selected);
}