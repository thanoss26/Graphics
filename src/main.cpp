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
//Geometry* models[] = { &cube, &teapot, &bunny, &sphere };
std::vector<Geometry*> models;
const char* modelNames[] = { "Cube", "Teapot", "Bunny", "Sphere" }; // Names for UI
int selectedModelIndex = -1; // No model selected by default

// Shader
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

int lastMouseX, lastMouseY;
bool isDragging = false;
bool isMoving = false;  // Track if the user is moving instead of rotating

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

void addModel(Geometry* model) 
{
    models.push_back(model);
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

void loadNewModel() {
    try {
        // List of available model files
        std::vector<std::string> modelFiles = {
            "models/teapot.obj",
            "models/bunny.obj",
            "models/sphere.obj"
        };

        // Seed the random number generator
        srand(static_cast<unsigned int>(time(0)));

        // Pick a random index from the modelFiles vector
        int randomIndex = rand() % modelFiles.size();

        // Attempt to load the new model
        Obj* newModel = new Obj();

        // Load the model from the randomly selected file (use c_str() to convert std::string to const char)
        newModel->init(modelFiles[randomIndex].c_str());

        // Apply a random transformation to the new model (e.g., random position)
        glm::vec3 randomPosition(
            (rand() % 100 - 50) * 0.1f,  // Random x between -5.0 and 5.0
            (rand() % 100 - 50) * 0.1f,  // Random y between -5.0 and 5.0
            (rand() % 100 - 50) * 0.1f   // Random z between -5.0 and 5.0
        );
        newModel->model = glm::translate(glm::mat4(1.0f), randomPosition);

        // Add the new model to the scene
        addModel(newModel);

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

            objectLoaded = true; // Set the flag to indicate an object is loaded
        }
        catch (const std::exception& e) {
            std::cerr << "Error loading model: " << e.what() << std::endl;
        }
    }

    // Add Model Button
    if (ImGui::Button("Add Model")) {
        if (selectedModelIndex != -1) {
            glm::vec3 position = generateRandomPosition(-5.0f, 5.0f); // Generate a random position for the model
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

    float discriminant = b * b - 4.0f * a * c;
    if (discriminant < 0.0f)
        return false;

    float t1 = (-b - sqrt(discriminant)) / (2.0f * a);
    float t2 = (-b + sqrt(discriminant)) / (2.0f * a);

    return t1 > 0.0f || t2 > 0.0f;
}

void mouseFunc(int button, int state, int x, int y) {
    ImGui_ImplGLUT_MouseFunc(button, state, x, y); // Pass mouse events to ImGui

    if (!ImGui::GetIO().WantCaptureMouse) { // Only process if ImGui doesn't capture the mouse
        if (button == GLUT_LEFT_BUTTON) {
            if (state == GLUT_DOWN) {
                std::cout << "Left mouse button clicked at (" << x << ", " << y << ")" << std::endl;

                // Convert screen coordinates to world ray
                glm::vec3 rayOrigin = camera.eye;
                glm::vec3 rayDirection = screenToWorldRay(x, y);

                // Check for intersections with models
                for (int i = 0; i < models.size(); ++i) 
                {
                    if (models[i] != nullptr && rayIntersectsModel(rayOrigin, rayDirection, models[i], 1.0f)) 
                    {
                        std::cout << "Model " << modelNames[i] << " was clicked!" << std::endl;

                        // Highlight the selected model

                        
                        selectedModelIndex = i;
                        objectLoaded = true;
                        break;
                    }
                }
            }
            else if (state == GLUT_UP) {
                isDragging = false;
            }
            loadNewModel();
        }
        
    }
    
}

void mouseDrag(int x, int y) {
    if (selectedModelIndex != -1 && isDragging) {
        Geometry* selectedModel = models[selectedModelIndex];

        // Get the mouse movement in screen space
        int dx = x - lastMouseX;
        int dy = y - lastMouseY;

        // Use raycasting to determine how much to move the model based on the mouse movement
        glm::vec3 rayOrigin = camera.eye;
        glm::vec3 rayDirection = screenToWorldRay(x, y);

        // Calculate a point in world space to move the model to (this can be a simple plane intersection)
        glm::vec3 moveDelta = glm::normalize(rayDirection) * 0.1f; // Adjust scale as needed (this controls the speed)

        // Update model position
        selectedModel->model = glm::translate(selectedModel->model, moveDelta);

        // Store the last mouse position for the next frame
        lastMouseX = x;
        lastMouseY = y;
    }
}

void mouseClick(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            isDragging = true;
            lastMouseX = x;
            lastMouseY = y;

            // Determine whether we are moving or rotating based on Shift key
            if (glutGetModifiers() & GLUT_ACTIVE_SHIFT) {
                isMoving = true;
            }
            else {
                isMoving = false;
            }

            // Model selection logic (ray picking)
            glm::vec3 rayOrigin = camera.eye;
            glm::vec3 rayDirection = screenToWorldRay(x, y);
            selectedModelIndex = 1000;
            float radius = 0.5f;

            for (size_t i = 0; i < models.size(); i++) {
                if (rayIntersectsModel(rayOrigin, rayDirection, models[i], radius)) {
                    selectedModelIndex = i;
                    std::cout << "Model " << i << " selected." << std::endl;
                    break;
                }
            }

            if (selectedModelIndex != 1000) {
                std::cout << "You selected model " << selectedModelIndex << std::endl;
            }
        }
        else if (state == GLUT_UP) {
            isDragging = false;
        }
    }
}

// Mouse motion callback
void motionFunc(int x, int y) {
    if (isDragging) {
        mouseDrag(x, y);  // Ensure dragging is called in motion
    }
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
    glutMouseFunc(mouseFunc);
    glutIdleFunc(glutPostRedisplay);

    atexit(cleanup);
    glutMainLoop();
    cleanup();

    return 0;
}
