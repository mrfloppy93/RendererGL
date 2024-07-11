//
// Created by lukas on 11.07.24.
//

#ifndef TEXTURE3D_H
#define TEXTURE3D_H

#include "Texture.h"
#include <vector>

class DepthTexture3D: public Texture {
public:
    DepthTexture3D(int _width, int _height, const std::vector<float> &_shadowCascadeLevels);
    DepthTexture3D() = default;
    ~DepthTexture3D() = default;
    void bind() override;
    void unbind() override;
private:
    std::vector<float> shadowCascadeLevels;

    void generateTexture() override;
};



#endif //TEXTURE3D_H
