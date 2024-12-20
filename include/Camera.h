#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>  // Include this header for perspective matrix

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef __CAMERA_H__
#define __CAMERA_H__

class Camera {
public:
    glm::vec3 eye;       // Position of the eye
    glm::vec3 target;    // Look at target
    glm::vec3 up;        // Up vector
    float fovy;          // Field of view in degrees
    float aspect;        // Aspect ratio
    float nearDist;      // Near clipping distance
    float farDist;       // Far clipping distance

    // Default values for reset
    glm::vec3 eye_default = glm::vec3(5.0f, 0.0f, 0.0f);
    glm::vec3 target_default = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 up_default = glm::vec3(0.0f, 1.0f, 0.0f);
    float fovy_default = 30.0f;
    float aspect_default = 4.0f / 3.0f;
    float near_default = 0.01f;
    float far_default = 100.0f;

    glm::mat4 view = glm::mat4(1.0f);  // View matrix
    glm::mat4 proj = glm::mat4(1.0f);  // Projection matrix

    // Method to update the aspect ratio
    void updateAspectRatio(float newWidth, float newHeight) {
        aspect = newWidth / newHeight;  // Calculate new aspect ratio
        proj = glm::perspective(glm::radians(fovy), aspect, nearDist, farDist);  // Update projection matrix
    }

    void rotateRight(const float degrees);
    void rotateUp(const float degrees);
    void computeMatrices(void);
    void reset(void);
};

#endif
