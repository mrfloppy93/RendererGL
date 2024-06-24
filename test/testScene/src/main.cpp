//
// Created by lukas on 24.06.24.
//
#include <iostream>
#include <vector>

#include <engine/renderer/Renderer.h>
#include <engine/renderer/TrackballCamera.h>
#include <engine/shapes/Cube.h>

#include <GLFW/glfw3.h>

const int WIDTH = 1280;
const int HEIGHT = 900;
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

    /*PointLight light1(glm::vec3(2, 0, 2));
    light1.setColor(glm::vec3(1, 0, 0));
    renderer->addLight(light1);*/

    DirectionalLight light2(glm::vec3(1));
    light2.setColor(glm::vec3(1));
    renderer->addLight(light2);

    // Camera
    double aspectRatio = static_cast<double>(WIDTH) / HEIGHT;
    TrackballCamera::Ptr camera = TrackballCamera::perspectiveCamera(glm::radians(45.0f), aspectRatio, 0.1, 1000);
    //camera->rotate(0.5,1);
    camera->setPhi(M_PI / 3 - 0.1);
    camera->setTheta(M_PI / 4 - 0.1);
    camera->zoom(-20);

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


    Group::Ptr group = Group::New();
    group->add(teamugPoly);
    group->add(dogPoly);

    Scene::Ptr scene = Scene::New();
    scene->addGroup(group);

    renderer->addScene(scene);

    // Main loop
    while (!glfwWindowShouldClose(window)) {

        // Update scene

        // Draw scene
        renderer->clear();
        renderer->draw();

        // Update window
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Destroy window
    glfwTerminate();

    return 0;
}