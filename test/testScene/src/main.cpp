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

void logAverageFrameTime(const std::string& inputLogFile) {
    const std::string outputLogFile = "average_frame_time_log.txt";
    // Read frame times from the input logfile
    std::ifstream logFile(inputLogFile);
    if (!logFile.is_open()) {
        std::cerr << "Failed to open input log file." << std::endl;
        return;
    }

    std::vector<double> frameTimes;
    double frameTime;
    GLuint primitivesGenerated;
    while (logFile >> frameTime >> primitivesGenerated) {
        frameTimes.push_back(frameTime);;
    }
    logFile.close();

    if (frameTimes.empty()) {
        std::cerr << "No frame times found in the log file." << std::endl;
        return;
    }

    // Calculate the average frame time
    double sum = std::accumulate(frameTimes.begin(), frameTimes.end(), 0.0);
    double averageFrameTime = sum / frameTimes.size();
    //double sumPrimitivesGenerated = std::accumulate(primitivesGeneratedList.begin(), primitivesGeneratedList.end(), 0.0);
    //double averagePrimitivesGenerated = sumPrimitivesGenerated / primitivesGeneratedList.size();
    int dataPoints = frameTimes.size();

    // Get the current date and time
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    // Format the date and time into a string
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    std::string dateTimeStr = oss.str();

    // Append the average frame time, number of data points, and date/time to the output logfile
    std::ofstream outputLog(outputLogFile, std::ios_base::app);
    if (!outputLog.is_open()) {
        std::cerr << "Failed to open output log file." << std::endl;
        return;
    }

    outputLog << "Average Frame Time: " << averageFrameTime << " ms, "
              << "Primitives Generated: " << primitivesGenerated << ", "
              << "Data Points: " << dataPoints << ", "
              << "Date/Time: " << dateTimeStr << std::endl;

    outputLog.close();
}

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
    const int max_rows = 1; //397700 795396
    const int max_cols = 2;
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
    const std::string logFileString = "render_log.txt";
    std::ofstream logFile(logFileString);
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

        GLuint query;
        glGenQueries(1, &query);
        glBeginQuery(GL_PRIMITIVES_GENERATED, query);

        // Render your scene here
        // Draw
        glFinish();
        auto frameStart = clock::now();
        renderer->clear();
        renderer->draw();
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

        logFile << frameTimeMs << " " << primitivesGenerated << std::endl;
        std::cout << "Frame Time: " << frameTimeMs << " ms" << std::endl;

    }
    // Destroy window
    glfwTerminate();
    logFile.close();

    logAverageFrameTime(logFileString);
        return 0;
}

