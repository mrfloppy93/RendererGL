//
// Created by lukas on 24.06.24.
//
#include <iostream>
#include <vector>

#include <engine/renderer/Renderer.h>
#include <engine/renderer/TrackballCamera.h>
#include <engine/shapes/Cube.h>

#include <GLFW/glfw3.h>
#include "ImguiStyles.h"

const int WIDTH = 1280;
const int HEIGHT = 900;

// ImGui functions
void initImGui(ImGuiIO& io);
void dockSpace(bool* p_open);
void renderImGui(ImGuiIO& io);

GLFWwindow* window;

Renderer::Ptr renderer;

int main() {

    // Create window
    if (!glfwInit()) {
        std::cout << "Couldn't initialize window" << std::endl;
        return -1;
    }

    window = glfwCreateWindow(WIDTH, HEIGHT, "Test Scene", NULL, NULL);

    if (!window) glfwTerminate();

    glfwMakeContextCurrent(window);

    // Init ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    initImGui(io);

    // Renderer
    renderer = Renderer::New(WIDTH, HEIGHT);

    // Lighting
    renderer->enableLight();

    // Point lighting
    /*PointLight light1(glm::vec3(2, 10, 2));
    light1.setColor(glm::vec3(.8, .5, .2));
    renderer->addLight(light1);*/

    // Directional Lighting
    DirectionalLight light2(glm::vec3(-4, 7, 5.5));
    light2.setColor(glm::vec3(1));
    renderer->addLight(light2);

    // Shadow Mappint
    renderer->setShadowMapping(true);
    renderer->setShadowLightPos(light2.getPosition());

    // Camera
    double aspectRatio = static_cast<double>(WIDTH) / HEIGHT;
    TrackballCamera::Ptr camera = TrackballCamera::perspectiveCamera(glm::radians(45.0f), aspectRatio, 0.1, 1000);
    camera->setPhi(M_PI / 3 - 0.1);
    camera->setTheta(M_PI / 4 - 0.1);
    camera->zoom(-30);

    renderer->setCamera(std::dynamic_pointer_cast<Camera>(camera));

    // Scene
    const Model::Ptr teamug = Model::New("/home/lukas/CLionProjects/RendererGL/models/OBJ/teamugblend.obj");
    const Polytope::Ptr teamugPoly = teamug->getPolytopes()[0];
    teamugPoly->setFaceCulling(Polytope::FaceCulling::BACK);
    teamugPoly->translate(glm::vec3(5,5,0));


    const Model::Ptr dog = Model::New("/home/lukas/CLionProjects/RendererGL/models/OBJ/10680_Dog_v2.obj");
    const Polytope::Ptr dogPoly = dog->getPolytopes()[0];
    dogPoly->setFaceCulling(Polytope::FaceCulling::BACK);
    dogPoly->rotate(-90, glm::vec3(1,0,0));
    dogPoly->scale(glm::vec3(.2));


    const Model::Ptr ground = Model::New("/home/lukas/CLionProjects/RendererGL/models/OBJ/platform.obj");
    const Polytope::Ptr groundPoly = ground->getPolytopes()[0];
    groundPoly->setFaceCulling(Polytope::FaceCulling::BACK);

    /*std::vector<Vec3f> groundVertices {
        Vec3f(-0.5, 0.0,  -0.5,  1.0f, 1.0f, 1.0f),
        Vec3f( 0.5, 0.0,  -0.5,  1.0f, 1.0f, 1.0f),
        Vec3f( -0.5,  0.0,  0.5,  1.0f, 1.0f, 1.0f),
        Vec3f(0.5,  0.0,  0.5,  1.0f, 1.0f, 1.0f),
    };

    std::vector<unsigned int> groundIndices {
        2,1,0,  3,1,2
    };

    Polytope::Ptr groundPoly = Polytope::New(groundVertices,groundIndices);
    groundPoly->scale(glm::vec3(20));
    //groundPoly->rotate(180, glm::vec3(1,0,0));
    groundPoly->setFaceCulling(Polytope::FaceCulling::BACK);*/

    Group::Ptr group = Group::New();
    group->add(teamugPoly);
    group->add(dogPoly);
    group->add(groundPoly);

    Scene::Ptr scene = Scene::New();
    scene->addGroup(group);

    renderer->addScene(scene);

    // Main loop
    while (!glfwWindowShouldClose(window)) {

        // Update scene

        // Draw scene
        renderer->clear();
        renderer->draw();

        // ImGUI
        /*{
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            bool p_open = true;
            dockSpace(&p_open);

            // Camera Window
            {
                ImGui::Begin("Camera");

                ImGui::TextColored(ImColor(200, 150, 255), "Camera options");

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

                    camera->setEye(glm::vec3(0, 0, -1));
                    camera->setUp(glm::vec3(0, -1, 0));
                    camera->setCenter(glm::vec3(0, 0, 0));
                }
                ImGui::SameLine();
                if (ImGui::Button("Trackball camera")) {
                    renderer->setCamera(std::dynamic_pointer_cast<Camera>(camera));
                }

                ImGui::End();
            }
            renderImGui(io);
        }*/

        // Update window
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

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