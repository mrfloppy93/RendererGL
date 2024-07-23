//
// Created by lukas on 11.07.24.
//
#define STB_IMAGE_WRITE

#include "DepthTexture3D.h"
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
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, id, 0, layer);

    // Check if the framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Failed to create framebuffer" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteFramebuffers(1, &fbo);
        return false;
    }

    // Allocate memory to store the texture data
    GLubyte* pixels = new GLubyte[4 * width * height];

    // Read the pixels from the framebuffer
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    // Unbind the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &fbo);

    // Write the image to a file using stb_image_write
    if (!stbi_write_png(filename, width, height, 4, pixels, width * 4)) {
        std::cerr << "Failed to write image file: " << filename << std::endl;
        delete[] pixels;
        return false;
    }

    // Free the allocated memory
    delete[] pixels;
    return true;
}