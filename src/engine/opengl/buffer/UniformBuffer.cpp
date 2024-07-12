//
// Created by lukas on 12.07.24.
//

#include "UniformBuffer.h"

UniformBuffer::UniformBuffer()
    : Buffer() {
    initBuffer();
}

UniformBuffer::~UniformBuffer() {
    unbind();
    glDeleteFramebuffers(1, &id);
}

// allocates the space of 16 glm::mat4 for the data
void UniformBuffer::initBuffer() {
    glGenBuffers(1, &id);
    glBindBuffer(GL_UNIFORM_BUFFER, id);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4x4) * 16, nullptr, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, id);
}

void UniformBuffer::setSubdataMat4(const glm::mat4& subdata, const int offset) {
    glBufferSubData(GL_UNIFORM_BUFFER, offset * sizeof(glm::mat4x4), sizeof(glm::mat4x4), &subdata);
}

void UniformBuffer::bind() {
    glBindBuffer(GL_UNIFORM_BUFFER, id);
}

void UniformBuffer::unbind() {
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}