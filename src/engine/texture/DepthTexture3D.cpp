//
// Created by lukas on 11.07.24.
//

#include "DepthTexture3D.h"

DepthTexture3D::DepthTexture3D(unsigned int _width, unsigned int _height, const std::vector<float> &_shadowCascadeLevels)
    : Texture(), shadowCascadeLevels(_shadowCascadeLevels)
{
    width = _width;
    height = _height;
    bpp = 1;
    type = Type::TextureDepth;
    generateTexture();
}

// Like implementation of DepthTexture::generateTexture() but with texture-2d-array.
void DepthTexture3D::generateTexture() {
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D_ARRAY, id);
    glTexImage3D(
    GL_TEXTURE_2D_ARRAY,
    0,
    GL_DEPTH_COMPONENT32F,
    width,
    height,
    int(shadowCascadeLevels.size()) + 1,
    0,
    GL_DEPTH_COMPONENT,
    GL_FLOAT,
    nullptr);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    constexpr float bordercolor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, bordercolor);

    slot = 0x84C0 + count;
    count ++;
}

void DepthTexture3D::bind() {
    glActiveTexture(slot);
    glBindTexture(GL_TEXTURE_2D_ARRAY, id);
}

void DepthTexture3D::unbind() {
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}