//
// Created by lukas on 11.07.24.
//
#include "DepthTexture3D.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <algorithm>

#include "vendor/stb_image_write.h"

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


// Function to save a layer of GL_TEXTURE_2D_ARRAY to an image file
bool DepthTexture3D::saveTextureArrayLayerToFile(int width, int height, int layer, const char* filename) {
    // Create a framebuffer object
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Attach the specified layer of the texture array to the framebuffer
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, id, 0, layer);

    // Check if the framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Failed to create framebuffer" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteFramebuffers(1, &fbo);
        return false;
    }

    // Allocate memory to store the depth data
    std::vector<float> depthData(width * height);

    // Read the depth values from the framebuffer
    glReadPixels(0, 0, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, depthData.data());

    // Unbind the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &fbo);

    // Find the minimum and maximum depth values
    auto minMax = std::minmax_element(depthData.begin(), depthData.end());
    float minDepth = *minMax.first;
    float maxDepth = *minMax.second;

    // Avoid division by zero in case all depth values are the same
    if (maxDepth - minDepth < 1e-6f) {
        maxDepth = minDepth + 1.0f;
    }

    // Convert depth values to 8-bit grayscale
    std::vector<unsigned char> imageData(width * height);
    for (int i = 0; i < width * height; ++i) {
        //float depthValue = depthData[i];
        float normalizedDepth = (depthData[i] - minDepth) / (maxDepth - minDepth);
        imageData[i] = static_cast<unsigned char>(normalizedDepth * 255.0f);
    }

    // Write the image to a file using stb_image_write
    if (!stbi_write_png(filename, width, height, 1, imageData.data(), width)) {
        std::cerr << "Failed to write image file: " << filename << std::endl;
        return false;
    }

    return true;
}