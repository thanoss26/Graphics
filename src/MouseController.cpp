#include <MouseController.h>
#include <img/imgui_impl_glut.cpp>
#include <iostream>
#include <Obj.h>
#include <glm.hpp>
#include <Camera.h>
#include "main.cpp"

// Mouse control
int lastMouseX, lastMouseY;
bool isDragging = false;
bool isMoving = false;  // Track if the user is moving instead of rotating

void mouseFunc(int button, int state, int x, int y) {
    ImGui_ImplGLUT_MouseFunc(button, state, x, y); // Pass mouse events to ImGui

    if (!ImGui::GetIO().WantCaptureMouse) { // Only process if ImGui doesn't capture the mouse
        if (button == GLUT_LEFT_BUTTON) {
            if (state == GLUT_DOWN) {
                std::cout << "Left mouse button clicked at (" << x << ", " << y << ")" << std::endl;

                // Convert screen coordinates to world ray
                glm::vec3 rayOrigin = camera.eye;
                glm::vec3 rayDirection = Utils::screenToWorldRay(x, y, width, height, camera);

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
        glm::vec3 rayDirection = Utils::screenToWorldRay(x, y, width, height, camera);

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
            glm::vec3 rayDirection = Utils::screenToWorldRay(x, y, width, height, camera);
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
