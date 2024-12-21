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

// Constants
static const int width = 800;
static const int height = 600;
static const char* title = "Model Viewer";
static const glm::vec4 background(0.1f, 0.2f, 0.3f, 1.0f);

// Scene Objects
static Cube cube;
static Obj teapot;
static Obj sphere;
static Obj bunny;
static Camera camera;
static bool bWireframe = false;
static bool objectLoaded = false;

// Models in the scene
Geometry* models[] = { &cube, &teapot, &bunny, &sphere };
const char* modelNames[] = { "Cube", "Teapot", "Bunny", "Sphere" }; // Names for UI
int selectedModelIndex = -1; // No model selected by default

//shader
struct NormalShader : Shader {
    glm::mat4 modelview = glm::mat4(1.0f);
    GLuint modelview_loc = 0;
    glm::mat4 projection = glm::mat4(1.0f);
    GLuint projection_loc = 0;
    GLuint isHighlighted_loc = 0;

    NormalShader() {}

    void initUniforms() {
        modelview_loc = glGetUniformLocation(program, "modelview");
        projection_loc = glGetUniformLocation(program, "projection");
        isHighlighted_loc = glGetUniformLocation(program, "isHighlighted");
    }

    void setUniforms(bool isHighlighted = false) {
        glUniformMatrix4fv(modelview_loc, 1, GL_FALSE, &modelview[0][0]);
        glUniformMatrix4fv(projection_loc, 1, GL_FALSE, &projection[0][0]);
        glUniform1i(isHighlighted_loc, static_cast<GLint>(isHighlighted));
    }
};
static NormalShader shader;

// Initialize Models
void initializeModels() {
    try {
        cube.init();
        teapot.init("models/teapot.obj");
        bunny.init("models/bunny.obj");
        sphere.init("models/sphere.obj");

        //cube.model = glm::translate(glm::mat4(1.0f), glm::vec3(-1.5f, 0.0f, 0.0f));
        //teapot.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
       // bunny.model = glm::translate(glm::mat4(1.0f), glm::vec3(1.5f, 0.0f, 0.0f));
        //sphere.model = glm::translate(glm::mat4(1.0f), glm::vec3(3.0f, 0.0f, 0.0f));
    }
    catch (const std::exception& e) {
        std::cerr << "Error initializing models: " << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }
}

// Function to generate random positions
glm::vec3 generateRandomPosition(float rangeMin, float rangeMax) {
    float x = rangeMin + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (rangeMax - rangeMin)));
    float y = rangeMin + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (rangeMax - rangeMin)));
    float z = rangeMin + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (rangeMax - rangeMin)));
    return glm::vec3(x, y, z);
}

void initialize() {
    glClearColor(background[0], background[1], background[2], background[3]);
    glViewport(0, 0, width, height);

    // Don't load models here anymore
    // initializeModels();  // Remove this line

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

    if (ImGui::Combo("Models", &selectedModelIndex, modelNames, IM_ARRAYSIZE(modelNames))) {
        std::cout << "Selected Model: " << modelNames[selectedModelIndex] << std::endl;
    }

    if (ImGui::Button("Deselect")) {
        selectedModelIndex = -1;
        objectLoaded = false;
        std::cout << "Model deselected." << std::endl;
    }

    ImGui::Checkbox("Wireframe Mode", &bWireframe);
    // Ensure the model is correctly initialized
    if (selectedModelIndex != -1) {
        std::cout << "Loading model: " << modelNames[selectedModelIndex] << std::endl;

        try {
            // Load the selected model (existing logic)
            if (selectedModelIndex == 0) {
                cube.init();
                models[selectedModelIndex] = &cube;
            }
            else if (selectedModelIndex == 1) {
                teapot.init("models/teapot.obj");
                models[selectedModelIndex] = &teapot;
            }
            else if (selectedModelIndex == 2) {
                bunny.init("models/bunny.obj");
                models[selectedModelIndex] = &bunny;
            }
            else if (selectedModelIndex == 3) {
                sphere.init("models/sphere.obj");
                models[selectedModelIndex] = &sphere;
            }

            // Generate a random position within a defined range
            glm::vec3 randomPos = generateRandomPosition(-5.0f, 5.0f);

            // Set the model's transformation matrix to the random position
            models[selectedModelIndex]->model = glm::translate(glm::mat4(1.0f), randomPos);

            objectLoaded = true; // Set the flag to indicate an object is loaded
            std::cout << "Model loaded at random position: " << randomPos.x << ", " << randomPos.y << ", " << randomPos.z << std::endl;

        }
        catch (const std::exception& e) {
            std::cerr << "Error loading model: " << e.what() << std::endl;
        }
    }

    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


void renderModels() 
{
    glUseProgram(shader.program);
    if (objectLoaded) { // Only render the selected object if it is loaded
        shader.setUniforms(selectedModelIndex == selectedModelIndex); // Highlight the selected model
        shader.modelview = camera.view * models[selectedModelIndex]->model;
        shader.setUniforms(selectedModelIndex == selectedModelIndex); // Highlight the selected model
        models[selectedModelIndex]->draw();
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

// Cleanup Function
void cleanup() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGLUT_Shutdown();
    ImGui::DestroyContext();
}
glm::vec3 screenToWorldRay(int mouseX, int mouseY) {
    float x = (2.0f * mouseX) / width - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / height;
    glm::vec4 rayClip(x, y, -1.0f, 1.0f);

    glm::vec4 rayEye = glm::inverse(camera.proj) * rayClip;
    rayEye.z = -1.0f;
    rayEye.w = 0.0f;

    glm::vec3 rayWorld = glm::vec3(glm::inverse(camera.view) * rayEye);
    return glm::normalize(rayWorld);
}

bool rayIntersectsModel(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, Geometry* model, float radius) {
    glm::vec3 modelPos = glm::vec3(model->model[3]);
    glm::vec3 oc = rayOrigin - modelPos;

    float a = glm::dot(rayDirection, rayDirection);
    float b = 2.0f * glm::dot(oc, rayDirection);
    float c = glm::dot(oc, oc) - radius * radius;

    float discriminant = b * b - 4 * a * c;
    return (discriminant > 0);
}

void mouseCallback(int button, int state, int x, int y) {
    ImGui_ImplGLUT_MouseFunc(button, state, x, y); // Pass mouse events to ImGui

    if (!ImGui::GetIO().WantCaptureMouse) { // Only process if ImGui doesn't capture the mouse
        // Add your custom mouse handling logic here
        if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
            std::cout << "Left mouse button clicked at (" << x << ", " << y << ")" << std::endl;
        }
    }
}


int lastMouseX, lastMouseY;
bool isDragging = false;
bool isMoving = false;  // Track if the user is moving instead of rotating

void mouseDrag(int x, int y) {
    if (selectedModelIndex != -1 && isDragging) {
        Geometry* selectedModel = models[selectedModelIndex];

        int dx = x - lastMouseX;
        int dy = y - lastMouseY;

        if (isMoving) {
            selectedModel->model = glm::translate(selectedModel->model, glm::vec3(dx * 0.01f, -dy * 0.01f, 0.0f));
        }
        else {
            float rotateSpeed = 0.5f;
            selectedModel->model = glm::rotate(selectedModel->model, glm::radians(dx * rotateSpeed), glm::vec3(0, 1, 0));
            selectedModel->model = glm::rotate(selectedModel->model, glm::radians(dy * rotateSpeed), glm::vec3(1, 0, 0));
        }

        lastMouseX = x;
        lastMouseY = y;
    }
}

void mouseFunc(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        isDragging = true;
        lastMouseX = x;
        lastMouseY = y;
    }
    else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
        isDragging = false;
    }
}

void motionFunc(int x, int y) {
    if (isDragging) {
        mouseDrag(x, y);
    }
}

int main(int argc, char** argv) {
    // Initialize GLUT and GLEW
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(width, height);
    glutCreateWindow(title);
    glewInit();

    // Initialize the scene
    initialize();

    // Register GLUT callbacks
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouseFunc);
    glutMotionFunc(motionFunc);
    glutPassiveMotionFunc(motionFunc);
    glutIdleFunc(glutPostRedisplay);

    // ImGui-specific mouse handling
    glutMouseFunc(mouseCallback);

    // Run the GLUT main loop
    glutMainLoop();

    // Cleanup before exiting
    cleanup();

    return 0;
}