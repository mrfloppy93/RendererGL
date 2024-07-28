//
// Created by lukas on 24.06.24.
//
#include <chrono>
#include <iostream>
#include <vector>

#include <engine/renderer/Renderer.h>
#include <engine/renderer/TrackballCamera.h>
#include <engine/shapes/Cube.h>

#include <GLFW/glfw3.h>
#include "ImguiStyles.h"
#include "engine/renderer/FPSCamera.h"
#include "imgui/imgui_internal.h"

#define NEAR_PLANE 0.1
#define FAR_PLANE 1000

const int WIDTH = 1280;
const int HEIGHT = 900;

// ImGui functions
void initImGui(ImGuiIO& io);
void dockSpace(bool* p_open);
void renderImGui(ImGuiIO& io);

// Window callbacks
static void resizeCallback(GLFWwindow* w, int width, int height);
static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

// Camera functions
void updateFPSCamera(double xpos, double ypos);

GLFWwindow* window;

Renderer::Ptr rendererSimple;
Renderer::Ptr rendererCSM;

// TrackBallCamera
TrackballCamera::Ptr camera;

// FPS camera
FPSCamera::Ptr fpsCamera;
bool movementForward = false, movementBackward = false;
bool movementRight = false, movementLeft = false;

// Mouse Ray casting (gui)
bool enablePoint3d = false, enableDrawRay = false;
bool enableObjectSelecting = false;
float rayLong = 100;

typedef Renderer::ShadowMappingProcedure ShadowProcedure;

int main() {

    // Create window
    if (!glfwInit()) {
        std::cout << "Couldn't initialize window" << std::endl;
        return -1;
    }

    window = glfwCreateWindow(WIDTH, HEIGHT, "Cascaded Shadow Mapping", NULL, NULL);
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    if (!window) glfwTerminate();

    glfwMakeContextCurrent(window);

    // Init ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    initImGui(io);

    // Renderer
    rendererSimple = Renderer::New(WIDTH, HEIGHT, ShadowProcedure::Simple);
    rendererCSM = Renderer::New(WIDTH, HEIGHT, ShadowProcedure::CSM);

    // Camera
    double aspectRatio = static_cast<double>(WIDTH) / HEIGHT;
    float sensitivity = 1.5f, panSensitivity = 1.0f, zoomSensitivity = 1.0f;
    camera = TrackballCamera::perspectiveCamera(glm::radians(45.0f), aspectRatio, NEAR_PLANE, FAR_PLANE);
    camera->zoom(-60);
    camera->rotate(2.5, 5.0);

    // First Person Shooter Camera
    fpsCamera = FPSCamera::perspectiveCamera(glm::radians(45.0f), WIDTH / HEIGHT, NEAR_PLANE, FAR_PLANE);
    fpsCamera->setSensitivity(sensitivity / 10);

    rendererSimple->setCamera(std::dynamic_pointer_cast<Camera>(camera));
    rendererCSM->setCamera(std::dynamic_pointer_cast<Camera>(camera));

    // Lighting
    rendererSimple->enableLight();
    rendererCSM->enableLight();

    // Point lighting
    /*PointLight light1(glm::vec3(2, 10, 2));
    light1.setColor(glm::vec3(.8, .5, .2));
    renderer->addLight(light1);*/

    // Directional Lighting
    DirectionalLight light2(glm::vec3(20, 50, 20));//(glm::vec3(-5, 3, 5.5));
    light2.setColor(glm::vec3(1));
    rendererSimple->addLight(light2);
    rendererCSM->addLight(light2);

    // Shadow Mapping
    rendererSimple->setShadowMapping(true);
    rendererSimple->setShadowLightPos(light2.getPosition());
    rendererCSM->setShadowMapping(true);
    rendererCSM->setShadowLightPos(light2.getPosition());



    // Scene
    const Model::Ptr dog = Model::New("/home/lukas/CLionProjects/RendererGL/models/OBJ/10680_Dog_v2.obj");
    //Texture::Ptr dogTexture = Texture::New("/home/lukas/CLionProjects/RendererGL/models/OBJ/10680_Dog_v2.mtl");
    const Polytope::Ptr dogPoly = dog->getPolytopes()[0];
    //dogPoly->addTexture(dogTexture);
    dogPoly->setFaceCulling(Polytope::FaceCulling::BACK);
    dogPoly->rotate(-90, glm::vec3(1,0,0));
    dogPoly->scale(glm::vec3(.2));

    const Model::Ptr dog2 = Model::New("/home/lukas/CLionProjects/RendererGL/models/OBJ/10680_Dog_v2.obj");
    const Polytope::Ptr dog2Poly = dog2->getPolytopes()[0];
    //dogPoly->addTexture(dogTexture);
    dog2Poly->setFaceCulling(Polytope::FaceCulling::BACK);
    dog2Poly->translate(glm::vec3(glm::vec3(5,0,10)));
    dog2Poly->rotate(-30, glm::vec3(0,1,0));
    dog2Poly->rotate(-90, glm::vec3(1,0,0));
    dog2Poly->scale(glm::vec3(.3));

    Cube::Ptr cube = Cube::New();
    cube->translate(glm::vec3(-10,5,-10));
    cube->scale(glm::vec3(10));

    const Model::Ptr ground = Model::New("/home/lukas/CLionProjects/RendererGL/models/OBJ/platform.obj");
    const Polytope::Ptr groundPoly = ground->getPolytopes()[0];
    groundPoly->scale(glm::vec3(10));
    groundPoly->setFaceCulling(Polytope::FaceCulling::BACK);

    const Model::Ptr wizard = Model::New("/home/lukas/CLionProjects/RendererGL/models/OBJ/wizard.obj");
    const Polytope::Ptr wizardPoly = wizard->getPolytopes()[0];
    wizardPoly->setFaceCulling(Polytope::FaceCulling::NONE);
    wizardPoly->translate(glm::vec3(0.0,3.5,0.0));
    wizardPoly->scale(glm::vec3(1.5));

    Group::Ptr group = Group::New();
    group->add(dogPoly);
    group->add(groundPoly);
    group->add(dog2Poly);
    group->add(cube);
    group->add(wizardPoly);

    Scene::Ptr scene = Scene::New();
    scene->addGroup(group);
    //scene->addModel(canyon);

    rendererSimple->addScene(scene);
    rendererCSM->addScene(scene);

    // Main loop
    while (!glfwWindowShouldClose(window)) {

        // Draw scene
        rendererSimple->clear();
        auto start_simple = std::chrono::high_resolution_clock::now();
        rendererSimple->render();
        auto stop_simple = std::chrono::high_resolution_clock::now();
        rendererCSM->clear();
        auto start_csm = std::chrono::high_resolution_clock::now();
        rendererCSM->render();
        auto stop_csm = std::chrono::high_resolution_clock::now();

        // ImGUI
        {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            bool p_open = true;
            dockSpace(&p_open);

            // Window
            {
                static float f = 0.0f;
                static int counter = 0;
                ImGui::Begin("Main Window");
                ImGui::TextColored(ImColor(200, 150, 255), "Main window controls");

                ImGui::Separator();

                static bool groupVisible = group->isVisible();
                ImGui::Checkbox("Show group", &groupVisible);
                group->setVisible(groupVisible);

                ImGui::SameLine();

                static bool showWire = group->isShowWire();
                ImGui::Checkbox("Show wire", &showWire);
                group->setShowWire(showWire);
                model->setShowWire(showWire);

                ImGui::SameLine();

                static bool showGrid = groupGrid->isVisible();
                ImGui::Checkbox("Show grid", &showGrid);
                groupGrid->setVisible(showGrid);

                ImGui::SameLine();

                static bool showSkyBox = true;
                ImGui::Checkbox("Skybox", &showSkyBox);
                if(!showSkyBox) renderer->setSkyBox(nullptr);
                else renderer->setSkyBox(skyBox);

                static bool showLights = lightsGroup->isVisible();
                ImGui::Checkbox("Show lights", &showLights);
                lightsGroup->setVisible(showLights);

                ImGui::SameLine();

                static bool emission = true;
                bool tempEmission = emission;
                ImGui::Checkbox("Emission", &emission);
                if(tempEmission != emission) {
                    if(!emission) {
                        textureEmission->setType(Texture::Type::None);
                        textureEmissionRed->setType(Texture::Type::None);
                    }
                    else {
                        textureEmission->setType(Texture::Type::TextureEmission);
                        textureEmissionRed->setType(Texture::Type::TextureEmission);
                    }
                }

                ImGui::SameLine();

                if(ImGui::Button("Red emission")) {
                    cubePolytope2->removeTexture(textureEmission);
                    cubePolytope2->addTexture(textureEmissionRed);
                }

                ImGui::SameLine();

                if(ImGui::Button("White emission")) {
                    cubePolytope2->removeTexture(textureEmissionRed);
                    cubePolytope2->addTexture(textureEmission);
                }

                ImGui::SameLine();

                if(ImGui::Button("Swap")) {
                    cubePolytope2->removeTexture(textureEmission);
                    cubePolytope2->removeTexture(textureEmissionRed);

                    Texture::Ptr temp = textureEmission;
                    textureEmission = textureEmissionRed;
                    textureEmissionRed = temp;

                    // IMPORTANT: set FreeGPU to true if the textures were copies
                    textureEmission->setFreeGPU(true);
                    textureEmissionRed->setFreeGPU(true);

                    cubePolytope2->addTexture(textureEmission);
                    cubePolytope2->addTexture(textureEmissionRed);
                }

                glm::vec3 backgroundColor = renderer->getBackgroundColor();
                static float color[3] = {backgroundColor.r, backgroundColor.g, backgroundColor.b};
                ImGui::ColorEdit3("Background color", color, 0);
                renderer->setBackgroundColor(color[0], color[1], color[2]);

                ImGui::Separator();

                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
                ImGui::End();
            }

            // Lighting window
            {
                ImGui::Begin("Lighting");
                ImGui::TextColored(ImColor(200, 150, 255), "Light configuration");
                ImGui::Text("Controls for your custom application");

                static bool enable = false;
                ImGui::Checkbox("Enable", &enable);
                rendererSimple->setLightEnabled(enable);

                ImGui::SameLine();

                static bool enableBlinn = true;
                ImGui::Checkbox("Blinn", &enableBlinn);
                Light::blinn = enableBlinn;

                ImGui::SameLine();

                static bool enableDirectionalLight = false;
                bool previousDirectionalLight = enableDirectionalLight;
                ImGui::Checkbox("Directional", &enableDirectionalLight);
                if(enableDirectionalLight != previousDirectionalLight) {
                    if(enableDirectionalLight) {
                        renderer->addLight(light4);
                        lightsGroup->add(directionalPolytope);
                    }
                    else {
                        renderer->removeLight(light4);
                        lightsGroup->removePolytope(directionalPolytope);
                    }
                    previousDirectionalLight = enableDirectionalLight;
                }

                static float ambientStrength = light.getAmbient()[0];
                ImGui::SliderFloat("Ambient strength", &ambientStrength, 0.f, 1.f);
                light.setAmbient(glm::vec3(ambientStrength));
                light2.setAmbient(glm::vec3(ambientStrength));
                light3.setAmbient(glm::vec3(ambientStrength));
                light4.setAmbient(glm::vec3(ambientStrength));
                light5.setAmbient(glm::vec3(ambientStrength));

                static float diffuseStrength = light.getDiffuse()[0];
                ImGui::SliderFloat("Diffuse strength", &diffuseStrength, 0.f, 1.f);
                light.setDiffuse(glm::vec3(diffuseStrength));
                light2.setDiffuse(glm::vec3(diffuseStrength));
                light3.setDiffuse(glm::vec3(diffuseStrength));
                light4.setDiffuse(glm::vec3(diffuseStrength));
                light5.setDiffuse(glm::vec3(diffuseStrength));

                static float specularStrength = light.getSpecular()[0];
                ImGui::SliderFloat("Specular strength", &specularStrength, 0.f, 1.f);
                light.setSpecular(glm::vec3(specularStrength));
                light2.setSpecular(glm::vec3(specularStrength));
                light3.setSpecular(glm::vec3(specularStrength));
                light4.setSpecular(glm::vec3(specularStrength));
                light5.setSpecular(glm::vec3(specularStrength));

                glm::vec3 lightColor = light.getColor();
                static float color[3] = { lightColor[0], lightColor[1], lightColor[2] };
                ImGui::ColorEdit3("Light color", color, 0);

                static Polytope::Ptr lightPolytope = lightsGroup->getPolytopes()[0];

                if(color[0] != lightColor.r || color[1] != lightColor.g || color[2] != lightColor.b) {
                    light.setColor(glm::vec3(color[0], color[1], color[2]));
                    for(auto& vec : cubeVertices) {
                        vec.r = color[0];
                        vec.g = color[1];
                        vec.b = color[2];
                    }
                    lightPolytope->updateVertices(cubeVertices);
                }

                static float lx = light2.getPosition().x;
                static float ly = light2.getPosition().y;
                static float lz = light2.getPosition().z;

                ImGui::Text("Light position");
                ImGui::SliderFloat("x:", &lx, -50.f, 50.f);
                ImGui::SliderFloat("y:", &ly, -50.f, 50.f);
                ImGui::SliderFloat("z:", &lz, -50.f, 50.f);

                glm::vec3 lightPosition = light2.getPosition();

                if(glm::vec3(lx, ly, lz) != lightPosition) {
                    float dx = lx - lightPosition.x;
                    float dy = ly - lightPosition.y;
                    float dz = lz - lightPosition.z;
                    light2.setPosition(glm::vec3(lx, ly, lz));
                    lightPolytope->translate(glm::vec3(dx, dy, dz));
                }

                ImGui::Separator();

                ImGui::TextColored(ImColor(200, 150, 255), "Shadows");

                static bool shadowMapping = false;
                ImGui::Checkbox("Shadow mapping", &shadowMapping);
                rendererSimple->setShadowMapping(shadowMapping);

                ImGui::Separator();

                ImGui::TextColored(ImColor(200, 150, 255), "HDR");

                static bool hdr = rendererSimple->isHDR();
                ImGui::Checkbox("HDR", &hdr);
                rendererSimple->setHDR(hdr);

                ImGui::SameLine();

                static bool gammaCorrection = rendererSimple->isGammaCorrection();
                ImGui::Checkbox("Gamma correction", &gammaCorrection);
                rendererSimple->setGammaCorrection(gammaCorrection);

                ImGui::SameLine();

                static float hdrExposure = 1.0f;
                ImGui::SliderFloat("HDR exposure:", &hdrExposure, 0.f, 5.f);
                rendererSimple->setExposure(hdrExposure);

                ImGui::End();
            }

            // Camera Window
            {
                ImGui::Begin("Camera");

                ImGui::TextColored(ImColor(200, 150, 255), "Camera options");
                ImGui::Text("Camera sensitivity");

                ImGui::Separator();

                ImGui::SliderFloat("Sensitivity", &sensitivity, 0.01f, 50.f);
                ImGui::SliderFloat("Pan sensitivity", &panSensitivity, 0.01f, 50.f);
                ImGui::SliderFloat("Zoom sensitivity", &zoomSensitivity, 0.01f, 50.f);

                fpsCamera->setSensitivity(sensitivity / 10);

                ImGui::Separator();

                ImGui::Text("Camera rotation angles");
                float theta = camera->getTheta(), phi = camera->getPhi();
                ImGui::SliderFloat("Theta", &theta, 0, M_PI);
                ImGui::SliderFloat("Phi", &phi, 0, 2 * M_PI);
                camera->setTheta(theta);
                camera->setPhi(phi);

                ImGui::Separator();

                if (ImGui::Button("Reset camera")) {
                    camera->setTheta(M_PI_2);
                    camera->setPhi(2 * M_PI);
                    camera->setRadius(5.5f);
                    camera->setCenter(glm::vec3(0, 0, 0));
                    camera->setUp(glm::vec3(0, 1, 0));
                    sensitivity = 1.5f;
                    panSensitivity = 1.0f;
                    zoomSensitivity = 1.0f;

                    fpsCamera->setEye(glm::vec3(0, 0, -1));
                    fpsCamera->setUp(glm::vec3(0, -1, 0));
                    fpsCamera->setCenter(glm::vec3(0, 0, 0));
                }
                ImGui::SameLine();
                if (ImGui::Button("Trackball camera")) {
                    rendererSimple->setCamera(std::dynamic_pointer_cast<Camera>(camera));
                }
                ImGui::SameLine();
                if (ImGui::Button("FPS camera")) {
                    rendererSimple->setCamera(std::dynamic_pointer_cast<Camera>(fpsCamera));
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                }

                ImGui::Separator();

                ImGui::TextColored(ImColor(200, 150, 255), "Mouse Ray Casting");

                ImGui::Checkbox("Enable 3D Point", &enablePoint3d);
                ImGui::SameLine();
                ImGui::Checkbox("Enable Drawing Ray", &enableDrawRay);
                ImGui::SliderFloat("Ray long", &rayLong, 0.5f, 1500);
                ImGui::Checkbox("Enable object selecting", &enableObjectSelecting);

                ImGui::End();
            }
            // Render window
            static bool windowFocus = false;
            {
                ImGui::Begin("Renderer", &p_open, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
                windowFocus = ImGui::IsWindowFocused() || ImGui::IsWindowHovered();

                // Render graphics as a texture
                ImGui::Image((void*)(intptr_t)rendererSimple->getFrameCapturer()->getTexture()->getID(), ImGui::GetWindowSize());

                // Resize window
                static ImVec2 previousSize(0, 0);
                ImVec2 currentSize = ImGui::GetWindowSize();

                if(currentSize.x != previousSize.x || currentSize.y != previousSize.y) {

                    // Restart trackball camera
                    float theta = camera->getTheta(), phi = camera->getPhi();
                    glm::vec3 center = camera->getCenter(), up = camera->getUp();
                    float radius = camera->getRadius();

                    // Update camera aspect ratio
                    *camera = *TrackballCamera::perspectiveCamera(glm::radians(45.0f), currentSize.x  / currentSize.y, 0.1, 1000);
                    camera->setTheta(theta);  camera->setPhi(phi);
                    camera->setCenter(center); camera->setUp(up);
                    camera->setRadius(radius);

                    // Restart fps camera
                    *fpsCamera = *FPSCamera::perspectiveCamera(glm::radians(45.0f), currentSize.x  / currentSize.y, 0.1, 1000);
                }
                previousSize = currentSize;

                // Mouse Events
                ImVec2 size = ImGui::GetWindowSize();
                ImVec2 mousePositionAbsolute = ImGui::GetMousePos();
                ImVec2 screenPositionAbsolute = ImGui::GetItemRectMin();
                ImVec2 mousePositionRelative = ImVec2(mousePositionAbsolute.x - screenPositionAbsolute.x, mousePositionAbsolute.y - screenPositionAbsolute.y);

                static bool first = true;
                static ImVec2 previous(0, 0);

                if(ImGui::IsMouseDown(ImGuiMouseButton_Left) || ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
                    if(first) {
                        previous = mousePositionRelative;
                        first = false;
                    }
                }else first = true;

                // Camera rotation
                if(ImGui::IsMouseDragging(ImGuiMouseButton_Left) && windowFocus) {
                    float dTheta = (mousePositionRelative.x - previous.x) / size.x;
                    float dPhi = (mousePositionRelative.y - previous.y) / size.y;
                    previous = mousePositionRelative;
                    camera->rotate(-dTheta * sensitivity, dPhi * sensitivity);
                }

                // Camera pan
                if(ImGui::IsMouseDragging(ImGuiMouseButton_Right) && windowFocus) {
                    float dx = (mousePositionRelative.x - previous.x) / (size.x / 2);
                    float dy = (mousePositionRelative.y - previous.y) / (size.y / 2);
                    previous = mousePositionRelative;
                    camera->pan(dx * panSensitivity, -dy * panSensitivity);
                }

                // Camera zoom
                if(windowFocus) camera->zoom(ImGui::GetIO().MouseWheel * zoomSensitivity);

                // FPS Camera
                updateFPSCamera(mousePositionRelative.x, mousePositionRelative.y);

                ImGui::End();
            }

            // Rendering
            renderImGui(io);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

        // Update window
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Destroy imgui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Destroy window
    glfwTerminate();

    return 0;
}

// ImGui functions

void initImGui(ImGuiIO& io) {
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

    //ImGui::StyleColorsDark();
    Style();

    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    const char* glsl_version = "#version 130";
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
}

void dockSpace(bool* p_open) {

    static bool opt_fullscreen = true;
    static bool opt_padding = false;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    if (opt_fullscreen)
    {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    }
    else dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;


    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
    // and handle the pass-thru hole, so we ask Begin() to not render a background.
    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;

    // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
    // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
    // all active windows docked into it will lose their parent and become undocked.
    // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
    // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
    if (!opt_padding) ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace", p_open, window_flags);
    if (!opt_padding) ImGui::PopStyleVar();

    if (opt_fullscreen) ImGui::PopStyleVar(2);

    // Submit the DockSpace
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Options")) {
            if (ImGui::MenuItem("Close", NULL, false, p_open != NULL)) exit(0);
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    ImGui::End();
}

void renderImGui(ImGuiIO& io) {
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
}

// Window functions
void resizeCallback(GLFWwindow* w, int width, int height) {
    //renderer->setViewport(width, height);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_W && action == GLFW_PRESS)          movementForward = true;
    else if(key == GLFW_KEY_W && action == GLFW_RELEASE)    movementForward = false;
    if (key == GLFW_KEY_S && action == GLFW_PRESS)          movementBackward = true;
    else if(key == GLFW_KEY_S && action == GLFW_RELEASE)    movementBackward = false;
    if (key == GLFW_KEY_A && action == GLFW_PRESS)          movementLeft = true;
    else if(key == GLFW_KEY_A && action == GLFW_RELEASE)    movementLeft = false;
    if (key == GLFW_KEY_D && action == GLFW_PRESS)          movementRight = true;
    else if (key == GLFW_KEY_D && action == GLFW_RELEASE)   movementRight = false;
    if(key == GLFW_KEY_ESCAPE) glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void updateFPSCamera(double xpos, double ypos) {
    // Look around
    fpsCamera->lookAround(xpos, ypos);
    // Movement
    if(movementForward) fpsCamera->move(FPSCamera::Movement::Forward);
    if(movementBackward) fpsCamera->move(FPSCamera::Movement::Backward);
    if(movementRight) fpsCamera->move(FPSCamera::Movement::Right);
    if(movementLeft) fpsCamera->move(FPSCamera::Movement::Left);
}