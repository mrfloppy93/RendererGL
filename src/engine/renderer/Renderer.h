#pragma once

#include <iostream>
#include <vector>

#define GLEW_STATIC
#include <GL/glew.h>

#include "engine/group/Scene.h"
#include "engine/opengl/shader/Shader.h"

#include "Camera.h"

#include "engine/lighting/Light.h"
#include "engine/lighting/DirectionalLight.h"
#include "engine/lighting/PointLight.h"

#include "engine/lighting/PBRMaterial.h"

#include "engine/texture/DepthTexture.h"
#include "engine/texture/ColorBufferTexture.h"

#include "engine/opengl/buffer/FrameBuffer.h"
#include "engine/opengl/buffer/RenderBuffer.h"

#include "SkyBox.h"

#include "FrameCapturer.h"
#include "TrackballCamera.h"

class Renderer {
    GENERATE_PTR(Renderer)

private:
    // Shaders
    ShaderProgram::Ptr shaderProgram;
    ShaderProgram::Ptr shaderProgramLighting;
    ShaderProgram::Ptr shaderProgramPBR;
    ShaderProgram::Ptr shaderProgramDepthMap;
    ShaderProgram::Ptr shaderProgramHDR;
    ShaderProgram::Ptr shaderProgramSkyBox;
    ShaderProgram::Ptr shaderProgramSelection;
    ShaderProgram::Ptr shaderProgramTexturedQuad;

    // Scenes visualization
    glm::mat4 projection;
    glm::mat4 view;

    std::vector<Scene::Ptr> scenes;

    // Camera
    Camera::Ptr camera;
    bool hasCamera;
    float cameraNearPlane, cameraFarPlane;

    // Lighting
    std::vector<Light*> lights;
    unsigned int nLights;
    bool hasLight;

    bool pbr;

    // Previous FBO
    int previousFBO;

    // Shadow Mapping
    std::vector<FrameBuffer::Ptr> depthMapFBOVector;
    std::vector<DepthTexture::Ptr> depthMapVector;
    std::vector<float> shadowCascadeLevels;
    int num_cascades;

    glm::mat4 lightSpaceMatrix;
    std::vector<glm::mat4> lightSpaceMatrices;
    glm::vec3 shadowLightPos;
    bool shadowMapping;

    // HDR
    FrameBuffer::Ptr hdrFBO;
    ColorBufferTexture::Ptr colorBufferTexture;
    RenderBuffer::Ptr rboDepth;

    bool hdr;
    float exposure;
    bool gammaCorrection;

    // Texture Quad
    VertexArray::Ptr quadVAO;
    VertexBuffer::Ptr quadVBO;

    // Background and skybox
    glm::vec3 backgroundColor;
    SkyBox::Ptr skyBox;

    unsigned int viewportWidth, viewportHeight;

    // Frame capturer
    FrameCapturer::Ptr frameCapturer;

public:
    Renderer(unsigned int _viewportWidth, unsigned int _viewportHeight);
    Renderer();
    ~Renderer() = default;
private:
    void loadFunctionsGL();
    void initShaders();
    void initTextureQuad();

    void textureUniformDefault(ShaderProgram::Ptr& shaderProgram, Polytope::Ptr& polytope);
    void textureUniformLighting(ShaderProgram::Ptr& shaderProgram, Polytope::Ptr& polytope);
    void textureUniformPBR(ShaderProgram::Ptr& shaderProgram, Polytope::Ptr& polytope);
    void textureUniform(ShaderProgram::Ptr& shaderProgram, Polytope::Ptr& polytope);

    void initShadowMapping();
    void initHDR();

    void primitiveSettings(Group::Ptr& group);
    void defaultPrimitiveSettings();
    void lightShaderUniforms();

    void pbrShaderUniforms();
    void lightMaterialUniforms(const Polytope::Ptr& polytope);
    void pbrMaterialUniforms(const Polytope::Ptr& polytope);
    void mvpUniform(ShaderProgram::Ptr& shaderProgram, const glm::mat4& model);
    void lightMVPuniform(const glm::mat4& model);
    void pbrMVPuniform(const glm::mat4& model);
    void shadowMappingUniforms();

    void renderScenesToDepthMap(std::vector<Scene::Ptr>& scenes);
    void renderScenes(std::vector<Scene::Ptr>& scenes);
    void renderToDepthMap();
    void renderQuad();
    void drawGroup(Scene::Ptr& scene, Group::Ptr& group);
    void drawSkyBox();

    void loadPreviousFBO();
    void bindPreviousFBO();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Functions for Cascaded Shadow Mapping
    static std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4& proj, const glm::mat4& view);
    glm::mat4 getLightSpaceMatrix(float nearPlane, float farPlane);
    std::vector<glm::mat4> getLightSpaceMatrices();
    void calculateCascadeLevels();
    static BoundingBox::Ptr createSceneDependentBB(const std::vector<Scene::Ptr>& scenes, const BoundingBox::Ptr& splitFrustumLightViewSpace, const glm::mat4& lightView, const glm::mat4& lightProj);
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
    void removeScene(Scene::Ptr& scene);
    void removeLight(Light& light);

    /**
     * Renders the whole scene into the frame capturer texture
     * 
     * You can get it and draw it in ImGui... etc
    */
    void render();

    /**
     * Renders the whole scene into the frame capturer texture
     * and then it draws the texture in a quad
    */
    void draw();

    void setBackgroundColor(float r, float g, float b);
    void setCamera(const Camera::Ptr& camera);

    void addLight(Light& light);

    void clear();

    void enableBlending();
    void enableAntialiasing();
    void enableBackFaceCulling();   // Counter-clockwise order
    void enableFrontFaceCulling();  // Counter-clockwise order
    void disableFaceCulling();

    void setFaceCulling(const Polytope::Ptr& polytope);
    void setViewport(unsigned int viewportWidth, unsigned int viewportHeight);

    void takeSnapshot();
public:
    inline void addScene(Scene::Ptr& scene) { scenes.push_back(scene); }
    inline void removeScene(int index) { scenes.erase(scenes.begin() + index); }
    inline Scene::Ptr getScene(int index) { return scenes[index]; }
    inline std::vector<Scene::Ptr>& getScenes() { return scenes; }

    inline Camera::Ptr getCamera() { return camera; }
    inline float getCameraNearPlane() const { return cameraNearPlane; }
    inline void setCameraNearPlane(float _nearPlane) { this->cameraNearPlane = _nearPlane; }
    inline float getCameraFarPlane() const { return cameraNearPlane; }
    inline void setCameraFarPlane(float _farPlane) { this->cameraFarPlane = _farPlane; }

    inline void enableLight() { hasLight = true; }
    inline void disableLight() { hasLight = false; } 
    inline void setLightEnabled(bool enable) { hasLight = enable; }

    inline void removeLight(int index) { lights.erase(lights.begin() + index); nLights --; }
    inline Light* getLight(int index) { return lights[index]; }

    inline std::vector<Light*>& getLights() { return lights; }

    inline void enablePBR() { pbr = true; }
    inline void disablePBR() { pbr = false; }
    inline void setPBREnabled(bool enable) { pbr = enable; }

    inline void setShadowLightPos(const glm::vec3& shadowLightPos) { this->shadowLightPos = shadowLightPos; }
    inline glm::vec3& getShadowLightPos() { return shadowLightPos; }

    inline void setShadowMapping(bool shadowMapping) { this->shadowMapping = shadowMapping; }
    inline bool isShadowMapping() const { return shadowMapping; }

    inline ShaderProgram::Ptr& getShaderProgram() { return shaderProgram; }

    inline glm::vec3& getBackgroundColor() { return backgroundColor; }

    inline void setSkyBox(const SkyBox::Ptr& skyBox) { this->skyBox = skyBox; }

    inline void setViewportWidth(unsigned int viewportWidth) { this->viewportWidth = viewportWidth; }
    inline unsigned int getViewportWidth() const { return viewportWidth; }

    inline void setViewportHeight(unsigned int viewportHeight) { this->viewportHeight = viewportHeight; }
    inline unsigned int getViewportHeight() const { return viewportHeight; }

    inline std::vector<DepthTexture::Ptr>& getDepthMaps() { return depthMapVector; }

    inline void setHDR(bool hdr) { this->hdr = hdr; }
    inline bool isHDR() const { return hdr; }

    inline void setExposure(float exposure) { this->exposure = exposure; }
    inline float getExposure() const { return exposure; }

    inline void setGammaCorrection(bool gammaCorrection) { this->gammaCorrection = gammaCorrection; }
    inline bool isGammaCorrection() const { return gammaCorrection; }

    inline FrameCapturer::Ptr getFrameCapturer() { return frameCapturer; }
};
