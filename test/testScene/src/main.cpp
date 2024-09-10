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
#include "../../../src/engine/group/BBVisuals.h"
#include "engine/shapes/Sphere.h"
#include "imgui/imgui_internal.h"

#define NEAR_PLANE 0.1
#define FAR_PLANE 300.0

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
    DirectionalLight light2(glm::vec3(20, 50, 20));//(glm::vec3(-5, 3, 5.5));
    light2.setColor(glm::vec3(1));
    renderer->addLight(light2);

    // Shadow Mappint
    renderer->setShadowMapping(true);
    renderer->setShadowLightPos(light2.getPosition());

    // Camera
    double aspectRatio = static_cast<double>(WIDTH) / HEIGHT;
    TrackballCamera::Ptr camera = TrackballCamera::perspectiveCamera(glm::radians(45.0f), aspectRatio, NEAR_PLANE, FAR_PLANE);
    camera->zoom(-90);
    camera->rotate(0.0, 5.0);

    renderer->setCamera(std::dynamic_pointer_cast<Camera>(camera));

    // Scene
    // Terrain
    const Model::Ptr ground = Model::New("/home/lukas/CLionProjects/RendererGL/models/OBJ/platform.obj");
    const Polytope::Ptr groundPoly = ground->getPolytopes()[0];
    groundPoly->scale(glm::vec3(50));
    groundPoly->setFaceCulling(Polytope::FaceCulling::BACK);
    Group::Ptr terrain = Group::New();
    terrain->setShadowCaster(false);
    terrain->add(groundPoly);

    // Loop for creating lots of dogs to increase load
    const int max_rows = 2;
    const int max_cols = 5;
    std::vector<Polytope::Ptr> objects;
    Group::Ptr objectGroup = Group::New();
    for(int row = 0; row < max_rows; row++) {
        for(int col = 0; col < max_cols; col++) {
            const auto obj = Model::New("/home/lukas/CLionProjects/RendererGL/models/OBJ/10680_Dog_v2.obj");
            objects.emplace_back(obj->getPolytopes()[0]);
            int index = col + row * max_cols;
            objects[index]->setFaceCulling(Polytope::FaceCulling::BACK);
            objects[index]->scale(glm::vec3(.2));
            objects[index]->rotate(-90, glm::vec3(1,0,0));
            objects[index]->translate(glm::vec3(row * 25, col * 50, 0));

            objectGroup->add(objects[index]);
        }
    }

    Scene::Ptr scene = Scene::New();
    scene->addGroup(terrain);
    scene->addGroup(objectGroup);


    renderer->addScene(scene);

    // Main loop
    while (!glfwWindowShouldClose(window)) {

        // Update scene

        // Draw scene
        renderer->clear();
        renderer->render();

        // ImGUI
        {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            bool p_open = true;
            dockSpace(&p_open);

            // Lighting window
            {
                ImGui::Begin("Lighting");
                ImGui::TextColored(ImColor(200, 150, 255), "Light configuration");
                ImGui::Text("Controls for your custom application");

                static bool enable = true;
                ImGui::Checkbox("Enable", &enable);
                renderer->setLightEnabled(enable);

                /*ImGui::SameLine();

                static bool enablePointLight = false;
                bool previousPointLight = enablePointLight;
                ImGui::Checkbox("Point Light", &enablePointLight);
                if(enablePointLight != previousPointLight) {
                    if(enablePointLight) {
                        renderer->addLight(light1);
                    }
                    else {
                        renderer->removeLight(light1);
                    }
                    //previousPointLight = enablePointLight;
                }*/


                /*static float ambientStrength = light2.getAmbient()[0];
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

                if(color[0] != lightColor.r || color[1] != lightColor.g || color[2] != lightColor.b) {
                    light.setColor(glm::vec3(color[0], color[1], color[2]));
                    for(auto& vec : cubeVertices) {
                        vec.r = color[0];
                        vec.g = color[1];
                        vec.b = color[2];
                    }
                }*/

                // Position of Directional Light
                static float lx = light2.getPosition().x;
                static float ly = light2.getPosition().y;
                static float lz = light2.getPosition().z;

                ImGui::Text("Directional Light position");
                ImGui::SliderFloat("x", &lx, -100.f, 100.f);
                ImGui::SliderFloat("y", &ly, -100.f, 100.f);
                ImGui::SliderFloat("z", &lz, -100.f, 100.f);

                glm::vec3 lightPosition = light2.getPosition();

                if(glm::vec3(lx, ly, lz) != lightPosition) {
                    /*float dx = lx - lightPosition.x;
                    float dy = ly - lightPosition.y;
                    float dz = lz - lightPosition.z;*/
                    light2.setPosition(glm::vec3(lx, ly, lz));
                    renderer->setShadowLightPos(light2.getPosition());
                    //lightPolytope->translate(glm::vec3(dx, dy, dz));
                }

                ImGui::Separator();

                ImGui::TextColored(ImColor(200, 150, 255), "Shadows");

                static bool shadowMapping = true;
                ImGui::Checkbox("Shadow mapping", &shadowMapping);
                renderer->setShadowMapping(shadowMapping);

                static int shadowProcedure = 0;
                static bool previousShadowMapping = false;
                if(previousShadowMapping != shadowMapping) {
                    if(!shadowMapping) {
                        shadowProcedure = 0;
                        //renderer->setShadowMappingProcedure(shadowProcedure);
                    }
                    previousShadowMapping = shadowMapping;
                }

                /*if(ImGui::RadioButton("CSM", shadowProcedure == 1)) {
                    shadowProcedure = 1;
                    // activate CSM, deactivate other procedures
                    renderer->setShadowMappingProcedure(shadowProcedure);
                }
                if(ImGui::RadioButton("PSSM", shadowProcedure == 2)) {
                    shadowProcedure = 2;
                    // activate PSSM, deactivate other procedures
                    renderer->setShadowMappingProcedure(shadowProcedure);
                }
                if(ImGui::RadioButton("TSM", shadowProcedure == 3)) {
                    shadowProcedure = 3;
                    // activate TSM, deactivate other procedures
                    renderer->setShadowMappingProcedure(shadowProcedure);
                }*/
                ImGui::TextColored(ImColor(200, 150, 255), "Snapshot");

                if(ImGui::Button("Take Snapshot")) {
                    renderer->takeSnapshot();
                }

                ImGui::End();
            }

            // Camera Window
            {
                ImGui::Begin("Camera");

                ImGui::TextColored(ImColor(200, 150, 255), "Camera options");
                /*ImGui::Text("Camera sensitivity");

                ImGui::Separator();

                ImGui::SliderFloat("Sensitivity", &sensitivity, 0.01f, 50.f);
                ImGui::SliderFloat("Pan sensitivity", &panSensitivity, 0.01f, 50.f);
                ImGui::SliderFloat("Zoom sensitivity", &zoomSensitivity, 0.01f, 50.f);

                fpsCamera->setSensitivity(sensitivity / 10);*/

                ImGui::Separator();

                ImGui::Text("Camera rotation angles");
                float theta = camera->getTheta(), phi = camera->getPhi();
                ImGui::SliderFloat("Theta", &theta, 0, M_PI);
                ImGui::SliderFloat("Phi", &phi, 0, 2 * M_PI);
                camera->setTheta(theta);
                camera->setPhi(phi);

                ImGui::Separator();

                if (ImGui::Button("Reset camera")) {
                    camera->setTheta(0.0/*M_PI_2*/);
                    camera->setPhi(5.0/*2 * M_PI*/);
                    //camera->zoom(-90.0);
                    //camera->setCenter(glm::vec3(0, 0, 0));
                    //camera->setUp(glm::vec3(0, 1, 0));
                    /*sensitivity = 1.5f;
                    panSensitivity = 1.0f;
                    zoomSensitivity = 1.0f;

                    fpsCamera->setEye(glm::vec3(0, 0, -1));
                    fpsCamera->setUp(glm::vec3(0, -1, 0));
                    fpsCamera->setCenter(glm::vec3(0, 0, 0));*/
                }
                /*ImGui::SameLine();
                if (ImGui::Button("Trackball camera")) {
                    renderer->setCamera(std::dynamic_pointer_cast<Camera>(camera));
                }
                ImGui::SameLine();
                if (ImGui::Button("FPS camera")) {
                    renderer->setCamera(std::dynamic_pointer_cast<Camera>(fpsCamera));
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                }*/

                //ImGui::Separator();

                /*ImGui::TextColored(ImColor(200, 150, 255), "Mouse Ray Casting");

                ImGui::Checkbox("Enable 3D Point", &enablePoint3d);
                ImGui::SameLine();
                ImGui::Checkbox("Enable Drawing Ray", &enableDrawRay);
                ImGui::SliderFloat("Ray long", &rayLong, 0.5f, 1500);
                ImGui::Checkbox("Enable object selecting", &enableObjectSelecting);*/

                ImGui::End();
            }
            // Render window
            static bool windowFocus = false;
            {
                ImGui::Begin("Renderer", &p_open, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
                windowFocus = ImGui::IsWindowFocused() || ImGui::IsWindowHovered();

                // Render graphics as a texture
                ImGui::Image((void*)(intptr_t)renderer->getFrameCapturer()->getTexture()->getID(), ImGui::GetWindowSize());

                // Resize window
                static ImVec2 previousSize(100, 100);
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
                    //*fpsCamera = *FPSCamera::perspectiveCamera(glm::radians(45.0f), currentSize.x  / currentSize.y, 0.1, 1000);
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
                float sensitivity = 5.0;
                if(ImGui::IsMouseDragging(ImGuiMouseButton_Left) && windowFocus) {
                    float dTheta = (mousePositionRelative.x - previous.x) / size.x;
                    float dPhi = (mousePositionRelative.y - previous.y) / size.y;
                    previous = mousePositionRelative;
                    camera->rotate(-dTheta * sensitivity, dPhi * sensitivity);
                }

                // Camera pan
                /*if(ImGui::IsMouseDragging(ImGuiMouseButton_Right) && windowFocus) {
                    float dx = (mousePositionRelative.x - previous.x) / (size.x / 2);
                    float dy = (mousePositionRelative.y - previous.y) / (size.y / 2);
                    previous = mousePositionRelative;
                    camera->pan(dx * panSensitivity, -dy * panSensitivity);
                }*/

                // Camera zoom
                float zoomSensitivity = 5.0;
                if(windowFocus) camera->zoom(ImGui::GetIO().MouseWheel * zoomSensitivity);

                // FPS Camera
                //updateFPSCamera(mousePositionRelative.x, mousePositionRelative.y);

                // Mouse Picking
                /*if(ImGui::IsMouseClicked(ImGuiMouseButton_Left) && (enablePoint3d || enableDrawRay || enableObjectSelecting) && windowFocus) {

                    MouseRayCasting mouseRayCasting(std::dynamic_pointer_cast<Camera>(camera), ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);
                    MouseRayCasting::Ray mouseRay = mouseRayCasting.getRay(mousePositionRelative.x, mousePositionRelative.y);

                    if(enablePoint3d) {
                        // The projection of this point belongs to the Screen Plane (X, Y)
                        glm::vec3 screenProjectedPoint = mouseRay.getScreenProjectedPoint();
                        Vec3f point3D = Vec3f(screenProjectedPoint.x, screenProjectedPoint.y, screenProjectedPoint.z, 1, 0, 0);
                        mousePickingPolytope->addVertex(point3D);
                    }

                    if(enableDrawRay) {
                        // Get the points of the ray from the screen to 'rayLong' distance
                        glm::vec3 begin = mouseRay.getPoint(rayLong);
                        glm::vec3 end = mouseRay.getPoint(1);
                        // Add these two vertices into the GL_LINES dynamic polytope
                        Vec3f vertex1(begin.x, begin.y, begin.z, 0, 1, 0);
                        Vec3f vertex2(end.x, end.y, end.z, 0, 0, 1);
                        raysPolytope->addVertex(vertex1);
                        raysPolytope->addVertex(vertex2);
                    }

                    if(enableObjectSelecting) {

                        // THIS IS A SIMPLE EXAMPLE OF BARYCENTRIC INTERSECTIONS (FOR THIS TEST).
                        // BUILD A BETTER ONE FOR YOUR OWN APPLICATION / GAME / GAMEENGINE

                        struct Plane {

                            double A, B, C, D;

                            Plane(double _A, double _B, double _C, double _D)
                                : A(_A), B(_B), C(_C), D(_D) {
                            }

                            Plane() = default;
                            ~Plane() = default;

                            static Plane plane3points(Vec3f& p1, Vec3f& p2, Vec3f& p3) {
                                Vec3f v1 = p2 - p1;
                                Vec3f v2 = p3 - p1;
                                Vec3f normal = v1 ^ v2;
                                double D = -(p1.x * normal.x + p1.y * normal.y + p1.z * normal.z);
                                return Plane(normal.x, normal.y, normal.z, D);
                            }
                        };

                        auto intersection = [&](MouseRayCasting::Ray& ray, Plane& plane) {
                            double lambda = -( (plane.A * ray.origin.x + plane.B * ray.origin.y + plane.C * ray.origin.z + plane.D)
                                / (plane.A * ray.rayDirection.x + plane.B * ray.rayDirection.y + plane.C * ray.rayDirection.z) );
                            return Vec3f(
                                ray.origin.x + lambda * ray.rayDirection.x,
                                ray.origin.y + lambda * ray.rayDirection.y,
                                ray.origin.z + lambda * ray.rayDirection.z
                            );
                        };

                        struct Vec2f {

                            float x, y;

                            Vec2f(float _x, float _y) : x(_x), y(_y) { }
                            Vec2f() = default;
                            ~Vec2f() = default;

                            // Dot product
                            inline float operator * (const Vec2f& rhs) const {
                                return x * rhs.x + y * rhs.y;
                            }

                            // Cross product
                            inline float operator ^ (const Vec2f& rhs) const {
                                return x * rhs.y - y * rhs.x;
                            }
                        };

                        auto isPointInTriangle = [&](float x, float y, float x0, float y0, float x1, float y1, float x2, float y2)
                        {
                            Vec2f v1(x0, y0);
                            Vec2f v2(x1, y1);
                            Vec2f v3(x2, y2);

                            Vec2f vs1(v2.x - v1.x, v2.y - v1.y);
                            Vec2f vs2(v3.x - v1.x, v3.y - v1.y);

                            Vec2f q(x - v1.x, y - v1.y);

                            float s = static_cast<float>(q ^ vs2) / (vs1 ^ vs2);
                            float t = static_cast<float>(vs1 ^ q) / (vs1 ^ vs2);

                            if((s >= 0) && (t >= 0) && (s + t <= 1)) return true;
                            return false;
                        };

                        auto checkPolytopeSelection = [&](std::vector<Vec3f>& points, Group::Ptr& group, Scene::Ptr& scene, Polytope::Ptr& polytope)
                        {
                            for(int i = 0; i < points.size(); i += 3) {

                                glm::vec4 vertex1(points[i].x, points[i].y, points[i].z, 1);
                                glm::vec4 vertex2(points[i + 1].x, points[i + 1].y, points[i + 1].z, 1);
                                glm::vec4 vertex3(points[i + 2].x, points[i + 2].y, points[i + 2].z, 1);

                                // Apply transforms
                                glm::mat4 model = scene->getModelMatrix() * group->getModelMatrix() * polytope->getModelMatrix();

                                vertex1 = model * vertex1;
                                vertex2 = model * vertex2;
                                vertex3 = model * vertex3;

                                Vec3f v1(vertex1.x, vertex1.y, vertex1.z);
                                Vec3f v2(vertex2.x, vertex2.y, vertex2.z);
                                Vec3f v3(vertex3.x, vertex3.y, vertex3.z);

                                Plane trianglePlane = Plane::plane3points(v1, v2, v3);
                                Vec3f rayIntersection = intersection(mouseRay, trianglePlane);

                                // Check if intersection is inside of the triangle
                                if(isPointInTriangle(
                                    rayIntersection.x, rayIntersection.y,
                                    v1.x, v1.y,
                                    v2.x, v2.y,
                                    v3.x, v3.y
                                ) || isPointInTriangle(
                                    rayIntersection.y, rayIntersection.z,
                                    v1.y, v1.z,
                                    v2.y, v2.z,
                                    v3.y, v3.z
                                ) || isPointInTriangle(
                                    rayIntersection.x, rayIntersection.z,
                                    v1.x, v1.z,
                                    v2.x, v2.z,
                                    v3.x, v3.z
                                ) ) {
                                    polytope->setSelected(true);
                                    if(enablePoint3d) mousePickingPolytope->addVertex(rayIntersection);
                                    break;
                                }
                                polytope->setSelected(false);
                            }
                        };

                        // CubePolytope selection
                        static std::vector<Vec3f> pointsCube = cubePolytope->getVertices();
                        checkPolytopeSelection(pointsCube, group, mainScene, cubePolytope);

                        // CubePolytope2 selection
                        static std::vector<Vec3f> pointsCube2 = cubePolytope2->getVertices();
                        checkPolytopeSelection(pointsCube2, group, mainScene, cubePolytope2);

                        // CubePolytope indices selection
                        static std::vector<Vec3f> pointsIndices;
                        static std::vector<Vec3f> cubeVertices = cubePolytopeIndices->getVertices();
                        static std::vector<unsigned int> cubeIndices = cubePolytopeIndices->getIndices();

                        if(pointsIndices.empty()) {

                            for(int i = 0; i < cubeIndices.size(); i += 3) {

                                Vec3f vertex1 = cubeVertices[cubeIndices[i]];
                                Vec3f vertex2 = cubeVertices[cubeIndices[i + 1]];
                                Vec3f vertex3 = cubeVertices[cubeIndices[i + 2]];

                                pointsIndices.push_back(vertex1);
                                pointsIndices.push_back(vertex2);
                                pointsIndices.push_back(vertex3);
                            }
                        }
                        checkPolytopeSelection(pointsIndices, group, mainScene, cubePolytopeIndices);
                    }
                }*/

                ImGui::End();
            }

            // Rendering
            renderImGui(io);
        }

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
    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

    //ImGui::StyleColorsDark();
    Style();

    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    const char* glsl_version = "#version 330";
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