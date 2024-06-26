#pragma once

#include <iostream>
#include <vector>
#include <memory>

#include "engine/opengl/buffer/VertexArray.h"
#include "engine/opengl/buffer/VertexBuffer.h"
#include "engine/opengl/shader/Shader.h"
#include "engine/texture/CubeMapTexture.h"

class SkyBox : public Buffer {
    GENERATE_PTR(SkyBox)
private:
    VertexArray::Ptr vertexArray;
    VertexBuffer::Ptr vertexBuffer;
    std::vector<std::string> faces;
    CubeMapTexture::Ptr cubeMap;
public:
    SkyBox() = default;
    /**
     Loads a cubemap texture from 6 individual texture faces
     order:
     +X (right)
     -X (left)
     +Y (top)
     -Y (bottom)
     +Z (front) 
     -Z (back)
    */
    SkyBox(const std::vector<std::string>& _faces);
    ~SkyBox();
private:
    void initBuffer() override;
public:
    void bind() override;
    void unbind() override;
    void draw();
public:
    inline std::vector<std::string>& getFaces() { return faces; }
    inline VertexArray::Ptr& getVertexArray() { return vertexArray; }
    inline CubeMapTexture::Ptr& getTextureCubeMap() { return cubeMap; }
};