//
// Created by lukas on 24.06.24.
//
#include <iostream>
#include <vector>
#include <chrono>
#include <fstream>

#include <engine/renderer/Renderer.h>
#include <engine/renderer/TrackballCamera.h>
#include <engine/shapes/Cube.h>

#include <GLFW/glfw3.h>
#include "ImguiStyles.h"
#include "../../../src/engine/group/BBVisuals.h"
#include "engine/shapes/Sphere.h"
#include "imgui/imgui_internal.h"

#define NEAR_PLANE 0.1
#define FAR_PLANE 500.0

const int WIDTH = 1920;
const int HEIGHT = 1080;

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

    // Renderer
    renderer = Renderer::New(WIDTH, HEIGHT);

    // Lighting
    renderer->enableLight();

    // Point lighting
    /*PointLight light1(glm::vec3(2, 10, 2));
    light1.setColor(glm::vec3(.8, .5, .2));
    renderer->addLight(light1);*/

    // Directional Lighting
    DirectionalLight light2(glm::vec3(200, 80, 100));//(glm::vec3(-5, 3, 5.5));
    light2.setColor(glm::vec3(1));
    renderer->addLight(light2);

    // Shadow Mappint
    renderer->setShadowMapping(true);
    renderer->setShadowLightPos(light2.getPosition());

    // Camera
    double aspectRatio = static_cast<double>(WIDTH) / HEIGHT;
    TrackballCamera::Ptr camera = TrackballCamera::perspectiveCamera(glm::radians(45.0f), aspectRatio, NEAR_PLANE, FAR_PLANE);
    camera->zoom(-90);
    camera->rotate(0.0, 1.0);

    renderer->setCamera(std::dynamic_pointer_cast<Camera>(camera));

    // Scene
    // Terrain
    const Model::Ptr ground = Model::New("/home/lukas/CLionProjects/RendererGL/models/OBJ/platform.obj");
    const Polytope::Ptr groundPoly = ground->getPolytopes()[0];
    groundPoly->scale(glm::vec3(500));
    groundPoly->setFaceCulling(Polytope::FaceCulling::BACK);
    Group::Ptr terrain = Group::New();
    terrain->setShadowCaster(false);
    terrain->add(groundPoly);

    // Loop for creating lots of dogs to increase load
    const int max_rows = 1;
    const int max_cols = 5;
    std::vector<Polytope::Ptr> objects;
    Group::Ptr objectGroup = Group::New();
    for(int row = 0; row < max_rows; row++) {
        for(int col = 0; col < max_cols; col++) {
            const auto obj = Model::New("/home/lukas/CLionProjects/RendererGL/models/OBJ/10680_Dog_v2.obj");
            objects.emplace_back(obj->getPolytopes()[0]);
            int index = col + row * max_cols;
            objects[index]->setFaceCulling(Polytope::FaceCulling::BACK);
            objects[index]->rotate(-90.0, glm::vec3(1.0,0.0,0.0));
            objects[index]->scale(glm::vec3(.4));
            float translateX = 0;
            float translateY = 0;
            if(row != 0) translateX = row%2==0 ? -row * 30 : (row+1) * 30;
            objects[index]->translate(glm::vec3(translateX, -col * 60, 0));

            objectGroup->add(objects[index]);
        }
    }

    Scene::Ptr scene = Scene::New();
    scene->addGroup(terrain);
    scene->addGroup(objectGroup);


    renderer->addScene(scene);

    // Performance Logging
    std::ofstream logFile("render_log.txt");
    if (!logFile.is_open()) {
        std::cerr << "Failed to open log file." << std::endl;
    }

    using clock = std::chrono::high_resolution_clock;
    auto startTime = clock::now();
    bool snap = false;

    // Main loop
    while (!glfwWindowShouldClose(window)) {

        renderer->setBackgroundColor(0.f, 0.f, 0.f);

        // Update scene
        //camera->rotate(.01, .0);

        // Draw
        glFinish();
        auto frameStart = clock::now();
        renderer->clear();
        renderer->draw();
        glFinish();
        auto frameEnd = clock::now();

        // Update window
        glfwSwapBuffers(window);
        glfwPollEvents();

        if(!snap) {
            renderer->takeSnapshot();
            snap = !snap;
        }

        // Calculate the time taken to render this frame in milliseconds
        std::chrono::duration<double, std::milli> frameDuration = frameEnd - frameStart;
        double frameTimeMs = frameDuration.count();

        // Calculate FPS (Frames per second)
        double fps = 1000.0 / frameTimeMs;

        // Log every second
        auto currentTime = clock::now();
        std::chrono::duration<double> elapsedTime = currentTime - startTime;
        if (elapsedTime.count() >= 1.0) {
            logFile << "FPS: " << fps << ", Frame Time: " << frameTimeMs << " ms" << std::endl;
            std::cout << "FPS: " << fps << ", Frame Time: " << frameTimeMs << " ms" << std::endl;
            startTime = currentTime;  // Reset the start time for the next second
        }
    }
        // Destroy window
        glfwTerminate();
        logFile.close();

        return 0;
}
