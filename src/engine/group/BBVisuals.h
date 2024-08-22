//
// Created by lukas on 22.08.24.
//

#ifndef BOUNDINGBOXSHAPE_H
#define BOUNDINGBOXSHAPE_H
#include "Group.h"


class BBVisuals : public Group {
    GENERATE_PTR(BBVisuals)
public:
    explicit BBVisuals(const Polytope::Ptr& polytope);
    virtual ~BBVisuals() = default;
};



#endif //BOUNDINGBOXSHAPE_H
