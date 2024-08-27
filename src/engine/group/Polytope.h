#pragma once

#include <iostream>
#include <vector>
#include <memory>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "BoundingBox.h"
#include "engine/opengl/buffer/VertexArray.h"
#include "engine/opengl/buffer/VertexBuffer.h"

#include "engine/lighting/Material.h"
#include "engine/lighting/PhongMaterial.h"

#include "engine/texture/Texture.h"

#include "engine/ptr.h"

#define MATERIAL_DIFFUSE glm::vec3(1.0f)
#define MATERIAL_SPECULAR glm::vec3(1.0f)
#define MATERIAL_SHININESS 64.f

class Polytope {
    GENERATE_PTR(Polytope)
public:
    enum class FaceCulling {
        NONE, FRONT, BACK
    };
protected:
    VertexArray::Ptr vertexArray;
    VertexBuffer::Ptr vertexBuffer;
    std::vector<Texture::Ptr> textures;
    BoundingBox::Ptr boundingBox;
    unsigned int vertexLength, indicesLength;
    Material::Ptr material;
    glm::mat4 modelMatrix;
    bool selected;
    FaceCulling faceCulling;
    float emissionStrength;
    bool tangentAndBitangents;
public:
    Polytope(size_t length);
    Polytope(std::vector<Vec3f>& vertices, bool _tangentAndBitangents = true);
    Polytope(std::vector<Vec3f>& vertices, std::vector<unsigned int>& indices, bool _tangentAndBitangents = true);
    Polytope(const Polytope& polytope);
    Polytope(Polytope&& polytope) noexcept;
    Polytope() = default;
    virtual ~Polytope() = default;
private:
    void initBoundingBox();
protected:
    void setTangentsAndBitangents(Vec3f& vertex0, Vec3f& vertex1, Vec3f& vertex2);
    void calculateTangentsAndBitangents(std::vector<Vec3f>& vertices);
    void calculateTangentsAndBitangents(std::vector<Vec3f>& vertices, std::vector<unsigned int>& indices);
public:
    void initPolytope(size_t length);
    void initPolytope(std::vector<Vec3f>& vertices);
    void initPolytope(std::vector<Vec3f>& vertices, std::vector<unsigned int>& indices);
    void bind();
    void unbind();
    void updateVertices(std::vector<Vec3f>& vertices);
    void updateVertex(int pos, Vec3f newVertex);
    void updateIndices(std::vector<unsigned int>& indices);
    void removeTexture(const Texture::Ptr& texture);
    void draw(unsigned int primitive, bool showWire = false);
    void drawInstanced(unsigned int primitive, unsigned int instances, bool showWire = false);
public:
    inline void translate(const glm::vec3& v) { modelMatrix = glm::translate(modelMatrix, v); }
    inline void rotate(float degrees, const glm::vec3& axis) { modelMatrix = glm::rotate(modelMatrix, glm::radians(degrees), axis); }
    inline void scale(const glm::vec3& s) { modelMatrix = glm::scale(modelMatrix, s); }

    inline VertexArray::Ptr& getVertexArray() { return vertexArray; }
    inline VertexBuffer::Ptr& getVertexBuffer() { return vertexBuffer; }
    inline IndexBuffer::Ptr& getIndexBuffer() { return vertexBuffer->getIndexBuffer(); }

    inline BoundingBox::Ptr& getBoundingBox() { return boundingBox; }

    inline void addTexture(const Texture::Ptr& texture) { textures.push_back(texture); }
    inline void removeTexture(int index) { textures.erase(textures.begin() + index); }
    inline std::vector<Texture::Ptr>& getTextures() { return textures; }

    inline unsigned int getVertexLength() const { return vertexLength; }

    inline void setModelMatrix(const glm::mat4& modelMatrix) { this->modelMatrix = modelMatrix; }
    inline glm::mat4& getModelMatrix() { return modelMatrix; }

    inline void setVetexArray(const VertexArray::Ptr& vertexArray) { this->vertexArray = vertexArray; }
    inline void setVertexBuffer(const VertexBuffer::Ptr& vertexBuffer) { this->vertexBuffer = vertexBuffer; }
    inline void setVertexLength(unsigned int vertexLength) { this->vertexLength = vertexLength; }

    inline void setMaterial(const Material::Ptr& material) { this->material = material; }
    inline Material::Ptr& getMaterial() { return material; }

    inline std::vector<Vec3f> getVertices() { return vertexBuffer->getVertices(); }
    inline std::vector<unsigned int> getIndices() { return vertexBuffer->getIndexBuffer()->getIndices(); }

    inline bool isSelected() const { return selected; }
    inline void setSelected(bool selected) { this->selected = selected; }

    inline void setFaceCulling(const FaceCulling& faceCulling) { this->faceCulling = faceCulling; }
    inline FaceCulling& getFaceCulling() { return faceCulling; }

    inline void setEmissionStrength(float emissionStrength) { this->emissionStrength = emissionStrength; }
    float getEmissionStrength() const { return emissionStrength; }
};