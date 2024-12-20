#include <cstdlib>
#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include "Screenshot.h"
#include "Shader.h"
#include "Cube.h"
#include "Obj.h"
#include "Camera.h"
#include "imgui.h"
#include "imgui_impl_glut.h"
#include "imgui_impl_opengl3.h"
#include <NormalShader.h>
#include "MouseController.h"
#include <Utils.h>

static const int width = 800;
static const int height = 600;
static const char* title = "Model Viewer";
static const glm::vec4 background(0.1f, 0.2f, 0.3f, 1.0f);

static Obj teapot;
static Obj sphere;
static Obj bunny;
static Camera camera;
static bool bWireframe = false;
static bool objectLoaded = false;

std::vector<Geometry*> models;
const char* modelNames[] = {"Teapot", "Bunny", "Sphere" }; // Names for UI
int selectedModelIndex = -1; // No model selected by default
static NormalShader shader;

void initializeModels() {
    try {
        teapot.init("models/teapot.obj");
        bunny.init("models/bunny.obj");
        sphere.init("models/sphere.obj");

        teapot.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
        bunny.model = glm::translate(glm::mat4(1.0f), glm::vec3(1.5f, 0.0f, 0.0f));
        sphere.model = glm::translate(glm::mat4(1.0f), glm::vec3(3.0f, 0.0f, 0.0f));
    }
    catch (const std::exception& e) {
        std::cerr << "Error initializing models: " << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }
}

void initialize() {
    glClearColor(background[0], background[1], background[2], background[3]);
    glViewport(0, 0, width, height);

    // Don't load models here anymore
    initializeModels();  // Remove this line

    camera.eye_default = glm::vec3(0.0f, 1.0f, 15.0f);
    camera.target_default = glm::vec3(0.0f, 0.0f, 0.0f);
    camera.up_default = glm::vec3(0.0f, 1.0f, 0.0f);
    camera.reset();

    shader.read_source("shaders/projective.vert", "shaders/normal.frag");
    shader.compile();
    glUseProgram(shader.program);
    shader.initUniforms();

    glEnable(GL_DEPTH_TEST);

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    if (!ImGui_ImplGLUT_Init()) {
        std::cerr << "Failed to initialize ImGui GLUT backend!" << std::endl;
        exit(EXIT_FAILURE);
    }
    if (!ImGui_ImplOpenGL3_Init("#version 330")) {
        std::cerr << "Failed to initialize ImGui OpenGL3 backend!" << std::endl;
        exit(EXIT_FAILURE);
    }
    ImGui::StyleColorsDark();

    // Set display size explicitly
    ImGui::GetIO().DisplaySize = ImVec2(width, height); // Ensure DisplaySize is set at initialization
}

void loadNewModel() {
    try {
        // List of available model files
        std::vector<std::string> modelFiles = {
            "models/teapot.obj",
            "models/bunny.obj",
            "models/sphere.obj"
        };

        // Attempt to load the new model
        Obj* newModel = new Obj();

        // Load the model from the randomly selected file (use c_str() to convert std::string to const char)
        newModel->init(modelFiles[0].c_str());

        // Apply a random transformation to the new model (e.g., random position)
        glm::vec3 randomPosition(
            (rand() % 100 - 50) * 0.1f,  // Random x between -5.0 and 5.0
            (rand() % 100 - 50) * 0.1f,  // Random y between -5.0 and 5.0
            (rand() % 100 - 50) * 0.1f   // Random z between -5.0 and 5.0
        );
        newModel->model = glm::translate(glm::mat4(1.0f), randomPosition);

        // Add the new model to the scene
        models.push_back(newModel);

        // Update the display to render the new model
        glutPostRedisplay();
    }
    catch (const std::exception& e) {
        std::cerr << "Error loading new model: " << e.what() << std::endl;
    }
}

void reshape(int w, int h) {
    // Update the OpenGL viewport to match the new window size
    glViewport(0, 0, w, h);

    // Update the camera projection matrix to maintain the aspect ratio
    float aspect = static_cast<float>(w) / static_cast<float>(h);
    camera.proj = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 1000.0f);

    // Update ImGui display size
    ImGui::GetIO().DisplaySize = ImVec2(w, h); // Update ImGui size
}

void renderUI() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGLUT_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x * 0.3f, ImGui::GetIO().DisplaySize.y));
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));

    ImGui::Begin("Model Selection");
    ImGui::Text("Select a model:");

    // Check if the model selection has changed
    bool modelChanged = false;
    if (ImGui::Combo("Models", &selectedModelIndex, modelNames, IM_ARRAYSIZE(modelNames))) {
        std::cout << "Selected Model: " << modelNames[selectedModelIndex] << std::endl;
        modelChanged = true; // Model selection has changed
    }

    if (ImGui::Button("Deselect")) {
        selectedModelIndex = -1;
        objectLoaded = false;
        std::cout << "Model deselected." << std::endl;
    }

    ImGui::Checkbox("Wireframe Mode", &bWireframe);

    // Load model only if it has changed or hasn't been loaded yet
    if (modelChanged && selectedModelIndex != -1 && !objectLoaded) {
        std::cout << "Loading model: " << modelNames[selectedModelIndex] << std::endl;

        try {
            // Load the selected model
            if (selectedModelIndex == 0) {
                teapot.init("models/teapot.obj");
                models[selectedModelIndex] = &teapot;
            }
            else if (selectedModelIndex == 1) {
                bunny.init("models/bunny.obj");
                models[selectedModelIndex] = &bunny;
            }
            else if (selectedModelIndex == 2) {
                sphere.init("models/sphere.obj");
                models[selectedModelIndex] = &sphere;
            }

            objectLoaded = true; // Set the flag to indicate an object is loaded
        }
        catch (const std::exception& e) {
            std::cerr << "Error loading model: " << e.what() << std::endl;
        }
    }

    // Add Model Button
    if (ImGui::Button("Add Model")) {
        if (selectedModelIndex != -1) {
            glm::vec3 position = Utils::generateRandomPosition(-5.0f, 5.0f); // Generate a random position for the model
            Geometry* selectedModel = models[selectedModelIndex];
            selectedModel->model = glm::translate(glm::mat4(1.0f), position); // Set the model's position
            std::cout << modelNames[selectedModelIndex] << " added at position: " << position.x << ", " << position.y << ", " << position.z << std::endl;
        }
        else {
            std::cout << "No model selected to add." << std::endl;
        }
    }

    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void renderModels() 
{
    glUseProgram(shader.program);
    camera.computeMatrices();
    shader.projection = camera.proj;

    for (size_t i = 0; i < models.size(); i++) {
        shader.modelview = camera.view * models[i]->model;  // Assuming `model` is the transformation matrix for the model
        shader.setUniforms();
        models[i]->draw();  // Assuming the Geometry class has a draw function
    }
}

void display() {
    if (bWireframe)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    camera.computeMatrices();
    shader.projection = camera.proj; // Update projection matrix

    renderModels();  // Render 3D models
    renderUI();      // Render ImGui UI

    glutSwapBuffers();
}

void cleanup() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGLUT_Shutdown();
    ImGui::DestroyContext();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(width, height);
    glutCreateWindow(title);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return EXIT_FAILURE;
    }

    initialize();

    glutReshapeFunc(reshape);
    glutDisplayFunc(display);
    glutMouseFunc(MouseController::mouseFunc);
    glutIdleFunc(glutPostRedisplay);

    atexit(cleanup);
    glutMainLoop();
    cleanup();

    return 0;
}
