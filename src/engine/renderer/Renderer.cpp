#include "Renderer.h"

#include <cmath>

#include "TrackballCamera.h"

#define NEAR_PLANE 0.1
#define FAR_PLANE 500

#define SHADOW_MAP_WIDTH 2048
#define SHADOW_MAP_HEIGHT 2048

Renderer::Renderer(unsigned int _viewportWidth, unsigned int _viewportHeight)
    : camera(nullptr), 
    hasCamera(false),
    cameraNearPlane(NEAR_PLANE),
    cameraFarPlane(FAR_PLANE),
    hasLight(false), 
    nLights(0),
    projection(glm::mat4(1.f)), 
    view(glm::mat4(1.f)),
    viewportWidth(_viewportWidth), 
    viewportHeight(_viewportHeight),
    shadowLightPos(0, 0, 0), 
    shadowMapping(false),
    exposure(1.0f),
    hdr(false), 
    gammaCorrection(false), 
    pbr(false), 
    backgroundColor(0.1f) 
{
    loadFunctionsGL();
    initShaders();
    enableBlending();
    enableAntialiasing();
    initShadowMapping();
    initHDR();
    initTextureQuad();

    frameCapturer = FrameCapturer::New(viewportWidth, viewportHeight);
}

Renderer::Renderer() 
    : Renderer(0, 0) {
}

void Renderer::loadFunctionsGL() {
    if (glewInit() != GLEW_OK) {
        std::cout << "Couldn't initialize GLEW" << std::endl;
        return;
    }
}

void Renderer::initShaders() {

    // Default shader program
    Shader vertexShader = Shader::fromFile("glsl/Default.vert", Shader::ShaderType::Vertex);
    Shader fragmentShader = Shader::fromFile("glsl/Default.frag", Shader::ShaderType::Fragment);
    shaderProgram = ShaderProgram::New(vertexShader, fragmentShader);

    // Lighting shader program
    Shader vertexLightingShader = Shader::fromFile("glsl/SimpleLighting.vert", Shader::ShaderType::Vertex);
    Shader fragmentLightingShader = Shader::fromFile("glsl/SimpleLighting.frag", Shader::ShaderType::Fragment);
    shaderProgramLighting = ShaderProgram::New(vertexLightingShader, fragmentLightingShader);

    // PBR shader program
    Shader vertexPBRShader = Shader::fromFile("glsl/PBR.vert", Shader::ShaderType::Vertex);
    Shader fragmentPBRShader = Shader::fromFile("glsl/PBR.frag", Shader::ShaderType::Fragment);
    shaderProgramPBR = ShaderProgram::New(vertexPBRShader, fragmentPBRShader);

    // Depth Map shader program
    Shader vertexDepthMapShader = Shader::fromFile("glsl/SimpleDepth.vert", Shader::ShaderType::Vertex);
    Shader fragmentDepthMapShader = Shader::fromFile("glsl/SimpleDepth.frag", Shader::ShaderType::Fragment);
    shaderProgramDepthMap = ShaderProgram::New(vertexDepthMapShader, fragmentDepthMapShader);

    /*Shader vertexDepthMapShaderCSM = Shader::fromFile("glsl/CSMDepth.vert", Shader::ShaderType::Vertex);
    Shader fragmentDepthMapShaderCSM = Shader::fromFile("glsl/CSMDepth.frag", Shader::ShaderType::Fragment);
    shaderProgramDepthMapCSM = ShaderProgram::New(vertexDepthMapShaderCSM, fragmentDepthMapShaderCSM);*/

    // HDR shader program
    Shader vertexHDRShader = Shader::fromFile("glsl/HDR.vert", Shader::ShaderType::Vertex);
    Shader fragmentHDRShader = Shader::fromFile("glsl/HDR.frag", Shader::ShaderType::Fragment);
    shaderProgramHDR = ShaderProgram::New(vertexHDRShader, fragmentHDRShader);

    // SkyBox shader program
    Shader vertexSkyBoxShader = Shader::fromFile("glsl/SkyBox.vert", Shader::ShaderType::Vertex);
    Shader fragmentSkyBoxShader = Shader::fromFile("glsl/SkyBox.frag", Shader::ShaderType::Fragment);
    shaderProgramSkyBox = ShaderProgram::New(vertexSkyBoxShader, fragmentSkyBoxShader);

    // Selection shader program
    Shader vertexSelectionShader = Shader::fromFile("glsl/Selection.vert", Shader::ShaderType::Vertex);
    Shader fragmentSelectionShader = Shader::fromFile("glsl/Selection.frag", Shader::ShaderType::Fragment);
    shaderProgramSelection = ShaderProgram::New(vertexSelectionShader, fragmentSelectionShader);

    // Textured quad shader program
    Shader vertexTexturedQuadShader = Shader::fromFile("glsl/TexturedQuad.vert", Shader::ShaderType::Vertex);
    Shader fragmentTexturedQuadShader = Shader::fromFile("glsl/TexturedQuad.frag", Shader::ShaderType::Fragment);
    shaderProgramTexturedQuad = ShaderProgram::New(vertexTexturedQuadShader, fragmentTexturedQuadShader);
}

void Renderer::initTextureQuad() {
    std::vector<Vec3f> quadVertices = {
        Vec3f(-1.0f,  1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f),
        Vec3f(-1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f),
        Vec3f( 1.0f,  1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f),
        Vec3f( 1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f)
    };
    quadVAO = VertexArray::New();
    quadVBO = VertexBuffer::New(quadVertices);
    quadVAO->unbind();
}

void Renderer::removeScene(Scene::Ptr& scene) {
    unsigned int index = 0;
    for(auto& s : scenes) {
        if(s.get() == scene.get()) {
            removeScene(index);
            break;
        }
        index ++;
    }
}

void Renderer::removeLight(Light& light) {
    unsigned int index = 0;
    for(Light* l : lights) {
        if(l == &light) {
            removeLight(index);
            break;
        }
        index ++;
    }
}

void Renderer::textureUniformDefault(ShaderProgram::Ptr& shaderProgram, std::shared_ptr<Polytope>& polytope) {
    unsigned int index = 0;
    std::vector<Texture::Ptr> textures = polytope->getTextures();
    if(!textures.empty()) {
        for(auto& texture : textures) {
            if(texture->getType() == Texture::Type::TextureDiffuse) {
                texture->bind();
                shaderProgram->uniformInt("tex", texture->getID() - 1);
                shaderProgram->uniformInt("hasTexture", true);
                index ++;
            }
        }
    }else shaderProgram->uniformInt("hasTexture", false);
}

void Renderer::textureUniformLighting(ShaderProgram::Ptr& shaderProgram, std::shared_ptr<Polytope>& polytope) {

    unsigned int nDiffuseMaps = 0, nSpecularMaps = 0, nEmissionMap = 0, nNormalMaps = 0, nDepthMaps = 0;

    for(auto& texture : polytope->getTextures()) {

        texture->bind();

        switch(texture->getType()) {

            case Texture::Type::TextureDiffuse:
                shaderProgram->uniformInt("materialMaps.diffuseMap", texture->getID() - 1);
                nDiffuseMaps ++;
            break;

            case Texture::Type::TextureSpecular:
                shaderProgram->uniformInt("materialMaps.specularMap", texture->getID() - 1);
                nSpecularMaps ++;
            break;

            case Texture::Type::TextureEmission:
                shaderProgram->uniformInt("materialMaps.emissionMap", texture->getID() - 1);
                nEmissionMap ++;
            break;

            case Texture::Type::TextureNormal:
                shaderProgram->uniformInt("materialMaps.normalMap", texture->getID() - 1);
                nNormalMaps ++;
            break;

            case Texture::Type::TextureHeight:
                shaderProgram->uniformInt("materialMaps.depthMap", texture->getID() - 1);
                nDepthMaps ++;
            break;
        }
    }

    shaderProgram->uniformInt("hasDiffuse", nDiffuseMaps > 0);
    shaderProgram->uniformInt("hasSpecular", nSpecularMaps > 0);
    shaderProgram->uniformInt("hasNormalMap", nNormalMaps > 0);
    shaderProgram->uniformInt("hasDepthMap", nDepthMaps > 0);
    shaderProgram->uniformInt("hasEmission", nEmissionMap > 0);

    const float heightScale = 0.1f;
    shaderProgram->uniformFloat("heightScale", heightScale);
}

void Renderer::textureUniformPBR(ShaderProgram::Ptr& shaderProgram, Polytope::Ptr& polytope) {

    unsigned int nAlbedoMaps = 0, nMetallicMaps = 0, nRoughnessMap = 0, nNormalMaps = 0, 
        nAmbientOcclusionMaps = 0, nEmissionMaps = 0, nDepthMaps = 0;

    for(auto& texture : polytope->getTextures()) {

        texture->bind();

        switch(texture->getType()) {

            case Texture::Type::TextureAlbedo:
                shaderProgram->uniformInt("materialMaps.albedo", texture->getID() - 1);
                nAlbedoMaps ++;
            break;

            case Texture::Type::TextureMetallic:
                shaderProgram->uniformInt("materialMaps.metallic", texture->getID() - 1);
                nMetallicMaps ++;
            break;

            case Texture::Type::TextureRoughness:
                shaderProgram->uniformInt("materialMaps.roughness", texture->getID() - 1);
                nRoughnessMap ++;
            break;

            case Texture::Type::TextureNormal:
                shaderProgram->uniformInt("materialMaps.normalMap", texture->getID() - 1);
                nNormalMaps ++;
            break;

            case Texture::Type::TextureHeight:
                shaderProgram->uniformInt("materialMaps.depthMap", texture->getID() - 1);
                nDepthMaps ++;
            break;

            case Texture::Type::TextureAmbientOcclusion:
                shaderProgram->uniformInt("materialMaps.ao", texture->getID() - 1);
                nAmbientOcclusionMaps ++;
            break;

            case Texture::Type::TextureEmission:
                shaderProgram->uniformInt("materialMaps.emission", texture->getID() - 1);
                nEmissionMaps ++;
            break;
        }
    }

    shaderProgram->uniformInt("hasAlbedo", nAlbedoMaps > 0);
    shaderProgram->uniformInt("hasMetallic", nMetallicMaps > 0);
    shaderProgram->uniformInt("hasNormal", nNormalMaps > 0);
    shaderProgram->uniformInt("hasDepthMap", nDepthMaps > 0);
    shaderProgram->uniformInt("hasRoughness", nRoughnessMap > 0);
    shaderProgram->uniformInt("hasAmbientOcclusion", nAmbientOcclusionMaps > 0);
    shaderProgram->uniformInt("hasEmission", nEmissionMaps > 0);

    const float heightScale = 0.5f;
    shaderProgram->uniformFloat("heightScale", heightScale);
}

void Renderer::textureUniform(ShaderProgram::Ptr& shaderProgram, std::shared_ptr<Polytope>& polytope) {
    if(pbr) textureUniformPBR(shaderProgram, polytope);
    else if(hasLight) textureUniformLighting(shaderProgram, polytope);
    else textureUniformDefault(shaderProgram, polytope);
}

void Renderer::primitiveSettings(Group::Ptr& group) {
    glPointSize(group->getPointSize());
    glLineWidth(group->getLineWidth());
}

void Renderer::defaultPrimitiveSettings() {
    glPointSize(1.0f);
    glLineWidth(1.0f);
}

void Renderer::lightShaderUniforms() {

    shaderProgramLighting->uniformInt("nLights", nLights);
    for(int i = 0; i < nLights; i ++) {

        float intensity = hdr ? lights[i]->getIntensity() : 1.0f;

        // Directional Light
        std::string lightUniform = "lights[" + std::to_string(i) + "]";
        shaderProgramLighting->uniformVec3(lightUniform + ".position", lights[i]->getPosition());
        shaderProgramLighting->uniformVec3(lightUniform + ".color", lights[i]->getColor() * intensity);
        shaderProgramLighting->uniformVec3(lightUniform + ".ambient", lights[i]->getAmbient());
        shaderProgramLighting->uniformVec3(lightUniform + ".diffuse", lights[i]->getDiffuse());
        shaderProgramLighting->uniformVec3(lightUniform + ".specular", lights[i]->getSpecular());

        // Point Light
        if(instanceof<PointLight>(lights[i])) {
            PointLight* pointLight = dynamic_cast<PointLight*>(lights[i]);
            shaderProgramLighting->uniformInt(lightUniform + ".pointLight", true);
            shaderProgramLighting->uniformFloat(lightUniform + ".constant", pointLight->getConstant());
            shaderProgramLighting->uniformFloat(lightUniform + ".linear", pointLight->getLinear());
            shaderProgramLighting->uniformFloat(lightUniform + ".quadratic", pointLight->getQuadratic());
        }else shaderProgramLighting->uniformInt(lightUniform + ".pointLight", false);

    }
    
    shaderProgramLighting->uniformInt("blinn", Light::blinn);
    shaderProgramLighting->uniformVec3("viewPos", camera->getEye());
    shaderProgramLighting->uniformInt("shadowMapping", shadowMapping);
}

void Renderer::pbrShaderUniforms() {

    shaderProgramPBR->uniformInt("nLights", nLights);

    for(int i = 0; i < nLights; i ++) {

        float intensity = hdr ? lights[i]->getIntensity() : 1.0f;

        // Directional Light
        std::string lightUniform = "lights[" + std::to_string(i) + "]";
        shaderProgramPBR->uniformVec3(lightUniform + ".position", lights[i]->getPosition());
        shaderProgramPBR->uniformVec3(lightUniform + ".color", lights[i]->getColor() * intensity);

        // Point Light
        if(instanceof<PointLight>(lights[i])) {
            PointLight* pointLight = dynamic_cast<PointLight*>(lights[i]);
            shaderProgramPBR->uniformInt(lightUniform + ".pointLight", true);
            shaderProgramPBR->uniformFloat(lightUniform + ".constant", pointLight->getConstant());
            shaderProgramPBR->uniformFloat(lightUniform + ".linear", pointLight->getLinear());
            shaderProgramPBR->uniformFloat(lightUniform + ".quadratic", pointLight->getQuadratic());
        }else shaderProgramPBR->uniformInt(lightUniform + ".pointLight", false);
    }
    
    shaderProgramPBR->uniformVec3("viewPos", camera->getEye());
    //shaderProgramPBR->uniformInt("shadowMapping", shadowMapping);
}

void Renderer::lightMaterialUniforms(const std::shared_ptr<Polytope>& polytope) {

    Material::Ptr material = polytope->getMaterial();

    // Phong materials
    if(material->getMaterialType() == Material::MaterialType::Phong) {

        PhongMaterial* phongMaterial = dynamic_cast<PhongMaterial*>(material.get()); 

        shaderProgramLighting->uniformVec3("material.diffuse", phongMaterial->getDiffuse());
        shaderProgramLighting->uniformVec3("material.specular", phongMaterial->getSpecular());
        shaderProgramLighting->uniformFloat("material.shininess", phongMaterial->getShininess());
    }

    shaderProgramLighting->uniformFloat("emissionStrength", polytope->getEmissionStrength());
}

void Renderer::pbrMaterialUniforms(const std::shared_ptr<Polytope>& polytope) {

    Material::Ptr material = polytope->getMaterial();

    // PBR materials
    if(material->getMaterialType() == Material::MaterialType::PBR) {
        
        PBRMaterial* pbrMaterial = dynamic_cast<PBRMaterial*>(material.get()); 

        shaderProgramPBR->uniformVec3("material.albedo", pbrMaterial->getAlbedo());
        shaderProgramPBR->uniformFloat("material.metallic", pbrMaterial->getMetallic());
        shaderProgramPBR->uniformFloat("material.roughness", pbrMaterial->getRoughness());
        shaderProgramPBR->uniformFloat("material.ao", pbrMaterial->getAmbientOcclusion());
    }
}

void Renderer::mvpUniform(ShaderProgram::Ptr& shaderProgram, const glm::mat4& model) {
    shaderProgram->uniformMat4("model", model);
    shaderProgram->uniformMat4("view", view);
    shaderProgram->uniformMat4("projection", projection);
}

void Renderer::lightMVPuniform(const glm::mat4& model) {
    mvpUniform(shaderProgramLighting, model);
}

void Renderer::pbrMVPuniform(const glm::mat4& model) {
    mvpUniform(shaderProgramPBR, model);
}

void Renderer::shadowMappingUniforms() {
    if(!shadowMapping) return;

    for(int i = 0; i < num_cascades; ++i) {
        depthMapVector[i]->bind();
        shaderProgramLighting->uniformInt("shadowMap[" + std::to_string(i) + "]", depthMapVector[i]->getID() - 1);

        shaderProgramLighting->uniformMat4("lightSpaceMatrices[" + std::to_string(i) + "]", lightSpaceMatrices[i]);
    }

    for(size_t i = 0; i < shadowCascadeLevels.size(); i++)
    {
        shaderProgramLighting->uniformFloat("cascadePlaneDistances[" + std::to_string(i) + "]", shadowCascadeLevels[i]);
        //std::cout << "cascadePlaneDistances[" + std::to_string(i) + "] : " << shadowCascadeLevels[i] << std::endl;
    }

    //shaderProgramLighting->uniformMat4("lightSpaceMatrix", lightSpaceMatrix);
    shaderProgramLighting->uniformVec3("lightPos", shadowLightPos);
    shaderProgramLighting->uniformFloat("farPlane", cameraFarPlane);
    shaderProgramLighting->uniformInt("cascadeCount", shadowCascadeLevels.size());
}

void Renderer::setFaceCulling(const Polytope::Ptr& polytope) {
    switch(polytope->getFaceCulling()) {
        case Polytope::FaceCulling::FRONT:
            enableFrontFaceCulling();
        break;
        case Polytope::FaceCulling::BACK:
            enableBackFaceCulling();
        break;
        case Polytope::FaceCulling::NONE:
            disableFaceCulling();
        break;
    }
}

void Renderer::setViewport(unsigned int viewportWidth, unsigned int viewportHeight) {
    this->viewportWidth = viewportWidth;
    this->viewportHeight = viewportHeight;
    frameCapturer->updateViewPort(viewportWidth, viewportHeight);
}

void Renderer::initShadowMapping() {
    shaderProgramLighting->useProgram();

    calculateCascadeLevels();

    for(int i = 0; i < num_cascades; ++i) {

        depthMapFBOVector.push_back(FrameBuffer::New());
        depthMapVector.push_back(DepthTexture::New(SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT));

        depthMapVector[i]->bind();
        depthMapFBOVector[i]->bind();

        depthMapFBOVector[i]->toTexture(
            GL_DEPTH_ATTACHMENT,
            GL_TEXTURE_2D,
            depthMapVector[i]->getID()
            );

        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

        depthMapFBOVector[i]->unbind();

        shaderProgramLighting->uniformInt("shadowMap[" + std::to_string(i) + "]", depthMapVector[i]->getID() - 1);
    }
}

void Renderer::initHDR() {
    
    hdrFBO = FrameBuffer::New();

    colorBufferTexture = ColorBufferTexture::New(viewportWidth, viewportHeight);
    colorBufferTexture->bind();

    // create depth buffer (renderbuffer)
    rboDepth = RenderBuffer::New(viewportWidth, viewportHeight);

    // attach buffers
    hdrFBO->bind();
    hdrFBO->toTexture(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBufferTexture->getID());
    hdrFBO->setRenderBuffer(GL_DEPTH_ATTACHMENT, rboDepth->getID());

    if (!hdrFBO->isComplete()) std::cout << "Framebuffer not complete!" << std::endl;

    hdrFBO->unbind();
}

void Renderer::renderScenesToDepthMap(std::vector<Scene::Ptr>& scenes) {

    for(auto& scene : scenes) {

            if(scene->isVisible()) {

                // Render groups
                for(auto& group : scene->getGroups()) {

                    if(!group->isVisible()) continue;

                    for(auto& polytope : group->getPolytopes()) {
                        
                        glm::mat4 model = scene->getModelMatrix() * group->getModelMatrix() * polytope->getModelMatrix();

                        shaderProgramDepthMap->uniformMat4("model", model);

                        glCullFace(GL_BACK);
                        polytope->draw(group->getPrimitive(), group->isShowWire());
                        glCullFace(GL_FRONT);
                    }
                }

                // Render child scenes
                renderScenesToDepthMap(scene->getScenes());
            }
        }
}

void Renderer::renderScenes(std::vector<Scene::Ptr>& scenes) {
    for(auto& scene : scenes) {
        if(scene->isVisible()) {
            // Draw groups
            for(auto& group : scene->getGroups()) {
                if(group->isVisible()) drawGroup(scene, group);
            }
            // Draw child scenes
            renderScenes(scene->getScenes());
        }
    }
}

void Renderer::renderToDepthMap() {

    if(!hasLight) return;

    loadPreviousFBO();

    shaderProgramDepthMap->useProgram();
    lightSpaceMatrices = getLightSpaceMatrices();

    for(int i = 0; i < num_cascades; ++i) {
        glViewport(0, 0, SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT);
        depthMapFBOVector[i]->bind();
        shaderProgramDepthMap->uniformMat4("lightSpaceMatrix", lightSpaceMatrices[i]);

        // Draw
        glClear(GL_DEPTH_BUFFER_BIT);

        renderScenesToDepthMap(scenes);

        depthMapFBOVector[i]->unbind();
    }

    bindPreviousFBO();
}

void Renderer::drawGroup(Scene::Ptr& scene, Group::Ptr& group) {
    
    if(!group->isVisible()) return;

    glViewport(0, 0, viewportWidth, viewportHeight);

    primitiveSettings(group);

    enableBlending();
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    for(auto& polytope : group->getPolytopes()) {

        // Compute model matrix from polytope, group and scene
        glm::mat4 model = scene->getModelMatrix() * group->getModelMatrix() * polytope->getModelMatrix();
        glm::mat4 mvp = projection * view * model;

        // PBR 
        if(pbr) {
            shaderProgramPBR->useProgram();
            pbrShaderUniforms();
            pbrMaterialUniforms(polytope);
            textureUniform(shaderProgramPBR, polytope);
            pbrMVPuniform(model);
            //shadowMappingUniforms();
        }
            
        // Phong lighting
        else if(hasLight) {
            shaderProgramLighting->useProgram();
            lightShaderUniforms();
            lightMaterialUniforms(polytope);
            textureUniform(shaderProgramLighting, polytope);
            lightMVPuniform(model);
            shadowMappingUniforms();
        }

        // Default  
        else {
            shaderProgram->useProgram();
            shaderProgram->uniformMat4("mvp", mvp);
            textureUniform(shaderProgram, polytope);
        }
        
        // Set face culling
        setFaceCulling(polytope);

        // Draw polytope
        polytope->draw(group->getPrimitive(), group->isShowWire());

        // Draw selected polytope if selected
        if(polytope->isSelected()) {
            shaderProgramSelection->useProgram();
            shaderProgramSelection->uniformMat4("mvp", mvp);

            glDisable(GL_DEPTH_TEST);
            polytope->draw(group->getPrimitive(), group->isShowWire());
            glEnable(GL_DEPTH_TEST);
        }

        // unbind textures
        for(auto& texture : polytope->getTextures()) texture->unbind();
    }

    // Set default primitive settings
    defaultPrimitiveSettings();
}

void Renderer::drawSkyBox() {

    if(skyBox == nullptr) return;

    shaderProgramSkyBox->useProgram();

    // remove translation from the view matrix
    view = glm::mat4(glm::mat3(camera->getViewMatrix()));

    skyBox->getTextureCubeMap()->bind();
    shaderProgramSkyBox->uniformInt("skybox", skyBox->getTextureCubeMap()->getID() - 1);

    shaderProgramSkyBox->uniformMat4("view", view);
    shaderProgramSkyBox->uniformMat4("projection", projection);

    // Draw call
    glDepthRange(0.999,1.0);
    skyBox->bind();
    skyBox->draw();
    glDepthRange(0.0,1.0);

    // set depth function back to default
    glDepthFunc(GL_LESS);
}

void Renderer::renderQuad() {

    shaderProgramHDR->useProgram();

    shaderProgramHDR->uniformInt("hdr", hdr);
    shaderProgramHDR->uniformFloat("exposure", exposure);
    shaderProgramHDR->uniformInt("gammaCorrection", gammaCorrection);

    colorBufferTexture->bind();
    shaderProgramHDR->uniformInt("hdrBuffer", colorBufferTexture->getID() - 1);

    quadVAO->bind();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    quadVAO->unbind();
}

void Renderer::render() {

    frameCapturer->startCapturing();

    //enableAntialiasing();
    enableBlending();

    if(hasCamera) {
        projection = camera->getProjectionMatrix();
        view = camera->getViewMatrix();
    }

    if(shadowMapping) renderToDepthMap();

    // FBO HDR
    if(hdr) {
        loadPreviousFBO();
        hdrFBO->bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    // Draw scenes
    renderScenes(scenes);

    // Draw skybox
    drawSkyBox();

    // Draw HDR texture to quad
    if(hdr) {
        bindPreviousFBO();
        renderQuad();
    }

    frameCapturer->finishCapturing();
}

void Renderer::draw() {

    render();

    shaderProgramTexturedQuad->useProgram();

    frameCapturer->getTexture()->bind();
    shaderProgramTexturedQuad->uniformInt("tex", frameCapturer->getTexture()->getID() - 1);

    quadVAO->bind();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    quadVAO->unbind();
}

void Renderer::setBackgroundColor(float r, float g, float b) {

    backgroundColor.r = r;
    backgroundColor.g = g;
    backgroundColor.b = b;

    frameCapturer->setBackgroundColor(r, g, b);
}

void Renderer::setCamera(const Camera::Ptr& camera) {
    hasCamera = true;
    this->camera = camera;
}

void Renderer::addLight(Light& light) {
    hasLight = true;
    lights.push_back(&light);
    nLights ++;
}

void Renderer::clear() {
    glStencilMask(0xFF);
    glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void Renderer::enableBlending() {
    glEnable(GL_BLEND & GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glDepthFunc(GL_LESS);
}

void Renderer::enableAntialiasing() {
    glEnable(GL_MULTISAMPLE);
}

void Renderer::enableBackFaceCulling() {
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW); 
}

void Renderer::enableFrontFaceCulling() {
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CCW); 
}

void Renderer::disableFaceCulling() {
    glDisable(GL_CULL_FACE);
}

void Renderer::loadPreviousFBO() {
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previousFBO);
}

void Renderer::bindPreviousFBO() {
    glBindFramebuffer(GL_FRAMEBUFFER, previousFBO);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Functions for cascaded shadow mapping
BoundingBox::Ptr Renderer::getFrustumCornersWorldSpace(const glm::mat4& proj, const glm::mat4& view) {
    const auto inv = glm::inverse(proj * view);

    std::vector<glm::vec3> frustumCorners;
    for(int x = 0; x < 2; ++x) {
        for(int y = 0; y < 2; ++y) {
            for(int z = 0; z < 2; ++z) {
                const glm::vec4 pt =
                    inv * glm::vec4(
                    2.0f * x - 1.0f,
                    2.0f * y - 1.0f,
                    2.0f * z - 1.0f,
                    1.0f);
                    frustumCorners.push_back(glm::vec3(pt/pt.w));
            }
        }
    }
    return BoundingBox::New(frustumCorners);
}

// get frustum corners from camera-frustum
BoundingBox::Ptr Renderer::getFrustumCornersWorldSpace() {
    return getFrustumCornersWorldSpace(camera->getProjectionMatrix(), camera->getViewMatrix());
}

glm::mat4 Renderer::getLightSpaceMatrix(const float nearPlane, const float farPlane) {
    //std::cout << "Calclate LightSpaceMatrix with nearPlane: " << nearPlane << " and farPlane: " << farPlane << std::endl;


    // Calculate field of view from camera-perspective-matrix
    auto cameraFovy = 2.0f * std::atan(1.0f/camera->getProjectionMatrix()[1][1]);
    const auto proj = glm::perspective(
            cameraFovy,
            (float) getViewportWidth()/(float) getViewportHeight(),
            nearPlane,
            farPlane);
    BoundingBox::Ptr frustumBBWorld = getFrustumCornersWorldSpace(proj, camera->getViewMatrix());

    //calculating lightView-Matrix
    glm::vec3 center = glm::vec3(0,0,0);

    for(const auto& v : frustumBBWorld->m_points) {
        center += v;
    }
    center /= frustumBBWorld->m_points.size();

    const auto lightView = glm::lookAt(center + glm::normalize(shadowLightPos), center, glm::vec3(0.0,1.0,0.0));

    //calculating lightProjection-Matrix
    auto min = glm::vec3(std::numeric_limits<float>::max());
    auto max = glm::vec3(std::numeric_limits<float>::lowest());

    for(const auto& v: frustumBBWorld->m_points) {
        const auto trf = lightView * glm::vec4(v, 1.0);
        min.x = std::min(min.x, trf.x);
        max.x = std::max(max.x, trf.x);
        min.y = std::min(min.y, trf.y);
        max.y = std::max(max.y, trf.y);
        min.z = std::min(min.z, trf.z);
        max.z = std::max(max.z, trf.z);
    }

    constexpr float zMult = 30.0f;
    if(min.z < 0) min.z *= zMult;
    else min.z /= zMult;
    if(max.z < 0) max.z /= zMult;
    else max.z *= zMult;

    const glm::mat4 lightProjection = glm::ortho(min.x,max.x,min.y,max.y,min.z,max.z);

    //const glm::mat4 lightViewProj = lightView * lightProjection;

    /*BoundingBox::Ptr cropBB = BoundingBox::transform(frustumBBWorld, lightViewProj);

    //cropBB->m_vMin.z = 0.0f;
    // Create the crop matrix
    glm::vec3 scale, offset;

    scale.x = 2.0f / (cropBB->m_vMax.x - cropBB->m_vMin.x);
    scale.y = 2.0f / (cropBB->m_vMax.y - cropBB->m_vMin.y);
    offset.x = -0.5f / (cropBB->m_vMax.x + cropBB->m_vMin.x) * scale.x;
    offset.y = -0.5f / (cropBB->m_vMax.y + cropBB->m_vMin.y) * scale.y;
    scale.z = 1.0f / (cropBB->m_vMax.z - cropBB->m_vMin.z);
    offset.z = -cropBB->m_vMin.z * scale.z;*/

    //std::cout << "minX: " << minX << "\tmaxX: " << maxX << std::endl;
    //std::cout << "minY: " << minY << "\tmaxY: " << maxY << std::endl;
    //std::cout << "minZ: " << minZ << "\tmaxZ: " << maxZ << std::endl;

    return lightView * lightProjection;

}



std::vector<glm::mat4> Renderer::getLightSpaceMatrices() {
    std::vector<glm::mat4> ret;
    if(shadowCascadeLevels.size() == 0) {
        ret.push_back(getLightSpaceMatrix(cameraNearPlane, cameraFarPlane));
        return ret;
    }
    for (size_t i = 0; i <= shadowCascadeLevels.size(); ++i)
    {
        if (i == 0)
        {
            ret.push_back(getLightSpaceMatrix(cameraNearPlane, shadowCascadeLevels[i]));
        }
        else if (i < shadowCascadeLevels.size())
        {
            ret.push_back(getLightSpaceMatrix(shadowCascadeLevels[i - 1], shadowCascadeLevels[i]));
        }
        else
        {
            ret.push_back(getLightSpaceMatrix(shadowCascadeLevels[i - 1], cameraFarPlane));
        }
    }
    return ret;
}

// applying the practical split scheme
void Renderer::calculateCascadeLevels() {

    float lambda = 0.5; // Lambda should be between 0 and 1

    for(int i = 1; i < 3; ++i) {
        float splitPosUni = cameraNearPlane + (cameraFarPlane - cameraNearPlane) * static_cast<float>(i)/3.0;
        float splitPosLog = cameraNearPlane * std::powf((cameraFarPlane/cameraNearPlane), static_cast<float>(i)/3.0);
        float splitPos = lambda * splitPosUni + (1.0 - lambda) * splitPosLog;
        shadowCascadeLevels.push_back(splitPos);
    }
    num_cascades = shadowCascadeLevels.size() + 1;
}

void Renderer::takeSnapshot() {
    for(int i = 0; i < num_cascades; ++i) {
        // Save the texture to an image file
        std::string filename = "depth_map_" + std::to_string(i) + ".png";
        if (depthMapVector[i]->saveDepthTextureToImage(SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT, filename.c_str())) {
            std::cout << "Image saved successfully!" << std::endl;
        } else {
            std::cerr << "Failed to save image." << std::endl;
        }
    }
}