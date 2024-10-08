//
// Created by lukas on 11.07.24.
//

#ifndef TEXTURE3D_H
#define TEXTURE3D_H

#include "Texture.h"
#include <vector>

class DepthTexture3D: public Texture {
    GENERATE_PTR(DepthTexture3D)
public:
    DepthTexture3D(unsigned int _width, unsigned int _height, const std::vector<float> &_shadowCascadeLevels);
    DepthTexture3D() = default;
    ~DepthTexture3D() = default;
    void bind() override;
    void unbind() override;
    bool saveTextureArrayLayerToFile(int width, int height, int layer, const char* filename);
private:
    std::vector<float> shadowCascadeLevels;

    void generateTexture() override;
};



#endif //TEXTURE3D_H
