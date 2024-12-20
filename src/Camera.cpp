#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>

// Rotate the camera around the "right" (horizontal) axis
void Camera::rotateRight(const float degrees) {
    // Compute the right vector as the cross product of (eye - target) and up
    glm::vec3 right = glm::normalize(glm::cross(eye - target, up));
    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(degrees), right);

    // Rotate the eye vector
    glm::vec4 rotated_eye = rotation * glm::vec4(eye - target, 1.0f);
    eye = glm::vec3(rotated_eye) + target;

    // Update the up vector
    up = glm::mat3(rotation) * up;
    up = glm::normalize(up);
}

// Rotate the camera around the "up" (vertical) axis
void Camera::rotateUp(const float degrees) {
    // Create a rotation matrix around the up vector
    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(degrees), up);

    // Rotate the eye vector
    glm::vec4 rotated_eye = rotation * glm::vec4(eye - target, 1.0f);
    eye = glm::vec3(rotated_eye) + target;

    // Ensure up remains orthogonal to the new view direction
    glm::vec3 forward = glm::normalize(target - eye);
    up = glm::normalize(glm::cross(glm::cross(forward, up), forward));
}

//// Implement the trackball rotation controlled by the mouse
//void Camera::rotate(const float deltaX, const float deltaY) {
//    // For simplicity, let's implement the rotation using Euler angles
//    // Rotate based on mouse movement along X and Y axes (deltaX and deltaY)
//
//    // Horizontal rotation (around Y axis)
//    rotateUp(deltaX * 0.1f);  // DeltaX is mapped to rotation around the up vector
//
//    // Vertical rotation (around X axis)
//    rotateRight(deltaY * 0.1f); // DeltaY is mapped to rotation around the right vector
//}

// Compute the view and projection matrices
void Camera::computeMatrices(void) {
    // View matrix using glm::lookAt
    view = glm::lookAt(eye, target, up);

    // Projection matrix using glm::perspective
    proj = glm::perspective(glm::radians(fovy), aspect, nearDist, farDist);

}

// Reset the camera to its default position and parameters
void Camera::reset(void) {
    // Reset all parameters to their default values
    eye = eye_default;
    target = target_default;
    up = up_default;
    fovy = fovy_default;
    aspect = aspect_default;
    nearDist = near_default;
    farDist = far_default;
}