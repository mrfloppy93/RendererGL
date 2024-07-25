#include "DepthTexture.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <algorithm>

#include "vendor/stb_image_write.h"

DepthTexture::DepthTexture(int _width, int _height) 
    : Texture() {
    width = _width;
    height = _height;
    bpp = 1;
    type = Type::TextureDepth;
    generateTexture();
}

void DepthTexture::generateTexture() {
    glGenTextures(1, &id);
    
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    
    slot = 0x84C0 + count;
    count ++;
}

// Function to save GL_TEXTURE_2D to an image file
// Gernated with ChatGPT 4.0 in July 2024
bool DepthTexture::saveDepthTextureToImage(int width, int height, const char* filename) {
    // Bind the texture
    glBindTexture(GL_TEXTURE_2D, id);

    // Allocate memory to read the depth data
    float* depthData = new float[width * height];

    // Read the depth data from the texture
    glGetTexImage(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GL_FLOAT, depthData);

    // Find min and max depth values
    float minDepth = *std::min_element(depthData, depthData + width * height);
    float maxDepth = *std::max_element(depthData, depthData + width * height);

    // Normalize the depth data to [0, 255] for saving as an image
    unsigned char* image = new unsigned char[width * height];
    for (int i = 0; i < width * height; ++i) {
        //image[i] = static_cast<unsigned char>((depthData[i] - minDepth) / (maxDepth - minDepth) * 255.0f);
        image[i] = static_cast<unsigned char>(depthData[i] * 255.0f);
    }

    // Write the image using stb_image_write
    if (!stbi_write_png(filename, width, height, 1, image, width)) {
        std::cerr << "Failed to write image file " << filename << std::endl;
        delete[] depthData;
        delete[] image;
        return false;
    }

    // Cleanup
    delete[] depthData;
    delete[] image;
    return true;
}