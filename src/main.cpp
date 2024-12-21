#include <cstdlib>
#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include "Screenshot.h"
#include <cstdlib>
#include <ctime> 
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
Geometry* models[] = { &teapot, &bunny, &sphere };
const char* modelNames[] = { "Teapot", "Bunny", "Sphere" }; // Names for UI
int selectedModelIndex = -1; // No model selected by default
std::vector<std::unique_ptr<Obj>> loadedModels;

struct NormalShader : Shader
{
    glm::mat4 modelview = glm::mat4(1.0f);
    GLuint modelview_loc;
    glm::mat4 projection = glm::mat4(1.0f);
    GLuint projection_loc;
    GLuint isHighlighted_loc;
    GLuint highlightColor_loc; // New uniform for highlight color

    void initUniforms() {
        modelview_loc = glGetUniformLocation(program, "modelview");
        projection_loc = glGetUniformLocation(program, "projection");
        isHighlighted_loc = glGetUniformLocation(program, "isHighlighted");
        highlightColor_loc = glGetUniformLocation(program, "highlightColor"); // Initialize highlight color uniform
    }

    void setUniforms(bool isHighlighted = false, glm::vec3 highlightColor = glm::vec3(1.0f, 0.0f, 0.0f)) {
        glUniformMatrix4fv(modelview_loc, 1, GL_FALSE, &modelview[0][0]);
        glUniformMatrix4fv(projection_loc, 1, GL_FALSE, &projection[0][0]);
        glUniform1i(isHighlighted_loc, static_cast<GLint>(isHighlighted));
        glUniform3fv(highlightColor_loc, 1, &highlightColor[0]); // Set highlight color
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

// Define the camera parameters
glm::vec3 cameraPosition(0.0f, 0.0f, 5.0f); // Example camera position
glm::vec3 cameraTarget(0.0f, 0.0f, 0.0f); // Camera target
glm::vec3 cameraUp(0.0f, 1.0f, 0.0f); // Up direction
glm::mat4 viewMatrix = glm::lookAt(cameraPosition, cameraTarget, cameraUp);

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
        std::cout << "Model deselected." << std::endl;
    }

    ImGui::Checkbox("Wireframe Mode", &bWireframe);

    if (ImGui::Button("Add model")) {
        std::cout << "Add model button pressed." << std::endl;

        // Create a new model based on the selected model index
        std::unique_ptr<Obj> newModel = std::make_unique<Obj>();

        if (selectedModelIndex == 0) {
            newModel->init("models/teapot.obj");
        }
        else if (selectedModelIndex == 1) {
            newModel->init("models/bunny.obj");
        }
        else if (selectedModelIndex == 2) {
            newModel->init("models/sphere.obj");
        }

        // Define the distance from the camera
        float distanceFromCamera = 5.0f;

        // Calculate the model's base position in front of the camera
        glm::vec3 cameraForward = glm::normalize(cameraTarget - cameraPosition); // Forward direction
        glm::vec3 basePosition = cameraPosition + cameraForward * distanceFromCamera;

        // Add random offsets within the visible frustum
        float frustumWidth = distanceFromCamera * tan(glm::radians(45.0f)); // Adjust based on your field of view
        float frustumHeight = frustumWidth / (16.0f / 9.0f); // Adjust based on your aspect ratio

        float randomOffsetX = (std::rand() % 200 - 100) / 100.0f * frustumWidth;
        float randomOffsetY = (std::rand() % 200 - 100) / 100.0f * frustumHeight;
        float randomOffsetZ = (std::rand() % 200) / 100.0f; // Random value between 0 and 2.0f

        glm::vec3 randomOffset(randomOffsetX, randomOffsetY, randomOffsetZ);

        // Set the model's position to the base position plus the random offset
        glm::vec3 modelPosition = basePosition + randomOffset;

        // Set the model's transformation matrix
        newModel->model = glm::translate(glm::mat4(1.0f), modelPosition);

        // Add the new model to the list of loaded models
        loadedModels.push_back(std::move(newModel));

        std::cout << "New model added at position: " << modelPosition.x << ", " << modelPosition.y << ", " << modelPosition.z << std::endl;
    }

    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


void renderModels() {
    glUseProgram(shader.program);
    camera.computeMatrices();

    // Loop through static models
    for (size_t i = 0; i < std::size(models); ++i) {
        bool isHighlighted = (i == selectedModelIndex);
        glm::vec4 highlightColor = isHighlighted ? glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) : glm::vec4(0.8f, 0.8f, 0.8f, 0.8f); // Red for highlight, white otherwise

        shader.modelview = camera.view * models[i]->model;
        shader.setUniforms(isHighlighted, glm::vec3(highlightColor)); // Set highlight color
        models[i]->draw();
    }

    // Render dynamically loaded models
    for (const auto& loadedModel : loadedModels) {
        if (!loadedModel) {
            // Skip if the model is null (safety check)
            continue;
        }

        // Check if selectedModelIndex is within the bounds of loadedModels
        bool isHighlighted = false;
        if (selectedModelIndex >= 0 && selectedModelIndex < loadedModels.size()) {
            isHighlighted = (loadedModel.get() == loadedModels[selectedModelIndex].get());
        }

        shader.modelview = camera.view * loadedModel->model;

        // Set highlight color for dynamically loaded models (red if highlighted, white otherwise)
        glm::vec4 highlightColor = isHighlighted ? glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) : glm::vec4(0.8f, 0.8f, 0.8f, 0.8f);

        shader.setUniforms(isHighlighted, glm::vec3(highlightColor)); // Set highlight color
        loadedModel->draw();
    }
}
void display() 
{
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

glm::vec3 screenToWorldRay(int mouseX, int mouseY, int screenWidth, int screenHeight, const glm::mat4& projection, const glm::mat4& view) {
    // Normalize screen coordinates to [-1, 1]
    float x = (2.0f * mouseX) / screenWidth - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / screenHeight;
    glm::vec4 ray_clip(x, y, -1.0f, 1.0f);

    // Transform from clip space to camera space
    glm::vec4 ray_eye = glm::inverse(projection) * ray_clip;
    ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0f, 0.0f); // Set w = 0 for directions

    // Transform from camera space to world space
    glm::vec3 ray_world = glm::vec3(glm::inverse(view) * ray_eye);
    ray_world = glm::normalize(ray_world); // Normalize to get direction

    return ray_world;
}
void onMouseClick(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        glm::vec3 rayDirection = screenToWorldRay(x, y, width, height, camera.proj, camera.view);

        // Perform ray-object intersection here
        std::cout << "Ray Direction: " << rayDirection.x << ", " << rayDirection.y << ", " << rayDirection.z << std::endl;
    }
}

bool rayIntersectsModel(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, Geometry* model, float radius) {
    glm::vec3 modelPos = glm::vec3(model->model[3]); // Extract position from the model matrix
    glm::vec3 oc = rayOrigin - modelPos;

    float a = glm::dot(rayDirection, rayDirection);
    float b = 2.0f * glm::dot(oc, rayDirection);
    float c = glm::dot(oc, oc) - radius * radius;

    float discriminant = b * b - 4 * a * c;
    return discriminant > 0; // Intersection occurs if the discriminant is positive
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
    glutMouseFunc(onMouseClick);
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