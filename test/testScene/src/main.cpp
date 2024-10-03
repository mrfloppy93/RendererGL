//
// Created by lukas on 24.06.24.
//
#include <iostream>
#include <vector>
#include <chrono>
#include <fstream>
#include <iostream>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <numeric>

#include <engine/renderer/Renderer.h>
#include <engine/renderer/TrackballCamera.h>

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

void logAverageFrameTime(const std::string& inputLogFile) {
    const std::string outputLogFile = "average_frame_time_log.csv";
    // Read frame times from the input logfile
    std::ifstream logFile(inputLogFile);
    if (!logFile.is_open()) {
        std::cerr << "Failed to open input log file." << std::endl;
        return;
    }

    std::vector<double> frameTimes;
    std::vector<double> drawTimes;
    double frameTime;
    double drawTime;
    GLuint primitivesGenerated;

    // Skip the first two lines of the log file
    for(int i = 0; i < 10; i++) {
        logFile >> frameTime >> drawTime >> primitivesGenerated;
    }

    while (logFile >> frameTime >> drawTime >> primitivesGenerated) {
        frameTimes.push_back(frameTime);
        drawTimes.push_back(drawTime);
    }
    logFile.close();

    if (frameTimes.empty()) {
        std::cerr << "No frame times found in the log file." << std::endl;
        return;
    }

    if (drawTimes.empty()) {
        std::cerr << "No draw times found in the log file." << std::endl;
        return;
    }

    // Calculate the average frame time
    double sum = std::accumulate(frameTimes.begin(), frameTimes.end(), 0.0);
    double averageFrameTime = sum / frameTimes.size();

    double sumDraw = std::accumulate(drawTimes.begin(), drawTimes.end(), 0.0);
    double averageDrawTime = sumDraw / drawTimes.size();

    // Append the average frame time, number of data points, and date/time to the output logfile
    std::ofstream outputLog(outputLogFile, std::ios_base::app);
    if (!outputLog.is_open()) {
        std::cerr << "Failed to open output log file." << std::endl;
        return;
    }
    if (outputLog.tellp() == 0) {
        outputLog << "Average Frame Time (ms),Average Draw Time (ms),Primitives Generated" << std::endl;;
    }

    outputLog << averageFrameTime << ", " << averageDrawTime << ", " << primitivesGenerated << std::endl;

    outputLog.close();
}

int main(int argc, char** argv) {

    if(argc < 3) {
        std::cout << "Please provide the number of rows and columns for the grid." << std::endl;
        return -1;
    }

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
    camera->zoom(-180.0);
    camera->rotate(0.0, 1.3);

    renderer->setCamera(std::dynamic_pointer_cast<Camera>(camera));

    // Scene
    // Terrain
    const Model::Ptr ground = Model::New("obj/platform.obj");
    const Polytope::Ptr groundPoly = ground->getPolytopes()[0];
    groundPoly->scale(glm::vec3(500));
    groundPoly->setFaceCulling(Polytope::FaceCulling::BACK);
    Group::Ptr terrain = Group::New();
    terrain->setShadowCaster(false);
    terrain->add(groundPoly);

    // Loop for creating lots of dogs to increase load
    const int max_rows = std::stoi(argv[1]);
    const int max_cols = std::stoi(argv[2]);

    std::vector<Polytope::Ptr> objects;
    Group::Ptr objectGroup = Group::New();
    for(int row = 0; row < max_rows; row++) {
        for(int col = 0; col < max_cols; col++) {
            const auto obj = Model::New("obj/dog.obj");
            objects.emplace_back(obj->getPolytopes()[0]);
            int index = col + row * max_cols;
            objects[index]->setFaceCulling(Polytope::FaceCulling::BACK);
            //objects[index]->rotate(180.0, glm::vec3(0.0,1.0,0.0));
            objects[index]->scale(glm::vec3(.4));
            float translateX = 0;
            float translateZ = -250 + col * 80;
            float multX = 15 + col * 5;
            if(row != 0) translateX = row%2==0 ? -row * multX : (row+1) * multX;
            translateX -= 15;
            objects[index]->translate(glm::vec3(translateX, 0, translateZ));

            objectGroup->add(objects[index]);
        }
    }

    Scene::Ptr scene = Scene::New();
    scene->addGroup(terrain);
    scene->addGroup(objectGroup);


    renderer->addScene(scene);

    // Performance Logging
    const std::string logFileString = "render_log.txt";
    std::ofstream logFile(logFileString);
    if (!logFile.is_open()) {
        std::cerr << "Failed to open log file." << std::endl;
    }

    using clock = std::chrono::high_resolution_clock;
    unsigned int samples = 510;
    bool snap = true;

    // Main loop
    while (!glfwWindowShouldClose(window)) {

        renderer->setBackgroundColor(0.f, 0.f, 0.f);

        GLuint query;
        glGenQueries(1, &query);
        glBeginQuery(GL_PRIMITIVES_GENERATED, query);

        // Render your scene here
        // Draw
        renderer->clear();
        glFinish();
        auto frameStart = clock::now();
        auto drawStart = clock::now();
        renderer->draw();
        auto drawEnd = clock::now();
        glFinish();
        auto frameEnd = clock::now();

        glEndQuery(GL_PRIMITIVES_GENERATED);

        GLuint primitivesGenerated;
        glGetQueryObjectuiv(query, GL_QUERY_RESULT, &primitivesGenerated);


        glDeleteQueries(1, &query);

        // Update window
        glfwSwapBuffers(window);
        glfwPollEvents();

        if(!snap) {
            renderer->takeSnapshot();
            std::cout << "Number of primitives generated: " << primitivesGenerated << std::endl;
            snap = !snap;
        }

        // Calculate the time taken to render this frame in milliseconds
        std::chrono::duration<double, std::milli> frameDuration = frameEnd - frameStart;
        double frameTimeMs = frameDuration.count();
        // Calculate the time taken to run the draw funtions this frame in milliseconds
        std::chrono::duration<double, std::milli> drawDuration = drawEnd - drawStart;
        double drawTimeMs = drawDuration.count();

        logFile << frameTimeMs << " " << drawTimeMs << " " << primitivesGenerated << std::endl;
        //std::cout << "Frame Time: " << frameTimeMs << " ms" << std::endl;
        if(--samples == 0) break;
    }
    // Destroy window
    glfwTerminate();
    logFile.close();

    logAverageFrameTime(logFileString);
        return 0;
}

