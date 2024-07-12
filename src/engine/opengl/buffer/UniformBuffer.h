//
// Created by lukas on 12.07.24.
//

#ifndef UNIFORMBUFFER_H
#define UNIFORMBUFFER_H
#include <glm/ext/matrix_float4x4.hpp>

#include "Buffer.h"

// implementation of a Uniformbuffer for glm::mat4, at the moment everything is hardcoded
class UniformBuffer: public Buffer {
    GENERATE_PTR(UniformBuffer)
public:
    UniformBuffer();
    ~UniformBuffer() override;
protected:
    void initBuffer() override;
public:
    static void setSubdataMat4(const glm::mat4& subdata, int offset);
    void bind() override;
    void unbind() override;
};



#endif //UNIFORMBUFFER_H
