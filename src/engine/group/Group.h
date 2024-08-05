#pragma once

#include <iostream>
#include <vector>
#include <memory>

#include "BoundingBox.h"
#include "Polytope.h"
#include "DynamicPolytope.h"

#include "engine/ptr.h"

#define POINT_SIZE 1.0
#define LINE_WIDTH 1.0
#define OUTLINING_WIDTH 3.0

class Group {
    GENERATE_PTR(Group)
private:
    std::vector<Polytope::Ptr> polytopes;
    unsigned int primitive;
    float pointSize, lineWidth, outliningWidth;
    bool showWire, visible;
    glm::mat4 modelMatrix;
    BoundingBox boundingBox;

    static unsigned long groupCount;
    unsigned long id;
public:
    Group(unsigned int _primitive, bool _showWire = false);
    Group();
    ~Group() = default;
public:
    void removePolytope(Polytope::Ptr& polytope);
    bool isSelected();
    void setSelected(bool selected);
public:
    inline void translate(const glm::vec3& v) { modelMatrix = glm::translate(modelMatrix, v); }
    inline void rotate(float degrees, const glm::vec3& axis) { modelMatrix = glm::rotate(modelMatrix, glm::radians(degrees), axis); }
    inline void scale(const glm::vec3& s) { modelMatrix = glm::scale(modelMatrix, s); }

    inline void add(const Polytope::Ptr& polytope) { polytopes.push_back(polytope); }
    inline std::vector<Polytope::Ptr>& getPolytopes() { return polytopes; }

    inline void removePolytope(int index) { polytopes.erase(polytopes.begin() + index); }

    inline void setVisible(bool visible) { this->visible = visible; }
    inline bool isVisible() const { return visible; }

    inline void setShowWire(bool showWire) { this->showWire = showWire; }
    inline bool isShowWire() const { return showWire; }

    inline void setPrimitive(unsigned int primitive) { this->primitive = primitive; }
    inline unsigned int getPrimitive() const { return primitive; }

    inline void setModelMatrix(const glm::mat4& modelMatrix) { this->modelMatrix = modelMatrix; }
    inline glm::mat4& getModelMatrix() { return modelMatrix; }

    inline float getPointSize() const { return pointSize; }
    inline float getLineWidth() const { return lineWidth; }
    inline float getOutliningWidth() const { return outliningWidth; }

    inline void setPointSize(float pointSize) { this->pointSize = pointSize; }
    inline void setLineWidth(float lineWidth) { this->lineWidth = lineWidth; }
    inline void setOutliningWidth(float outliningWidth) { this->outliningWidth = outliningWidth; }

    inline unsigned long getID() const { return id; }
};