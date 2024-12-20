#pragma once

class Frustum {
public:
    glm::vec4 planes[6]; // The six planes (near, far, left, right, top, bottom)

    void update(const glm::mat4& viewProj) {
        // Calculate the planes from the combined view and projection matrix
        // Extract the planes from the matrix
        planes[0] = glm::vec4(viewProj[0][3] + viewProj[0][0], viewProj[1][3] + viewProj[1][0], viewProj[2][3] + viewProj[2][0], viewProj[3][3] + viewProj[3][0]); // Left
        planes[1] = glm::vec4(viewProj[0][3] - viewProj[0][0], viewProj[1][3] - viewProj[1][0], viewProj[2][3] - viewProj[2][0], viewProj[3][3] - viewProj[3][0]); // Right
        planes[2] = glm::vec4(viewProj[0][3] + viewProj[0][1], viewProj[1][3] + viewProj[1][1], viewProj[2][3] + viewProj[2][1], viewProj[3][3] + viewProj[3][1]); // Bottom
        planes[3] = glm::vec4(viewProj[0][3] - viewProj[0][1], viewProj[1][3] - viewProj[1][1], viewProj[2][3] - viewProj[2][1], viewProj[3][3] - viewProj[3][1]); // Top
        planes[4] = glm::vec4(viewProj[0][3] + viewProj[0][2], viewProj[1][3] + viewProj[1][2], viewProj[2][3] + viewProj[2][2], viewProj[3][3] + viewProj[3][2]); // Near
        planes[5] = glm::vec4(viewProj[0][3] - viewProj[0][2], viewProj[1][3] - viewProj[1][2], viewProj[2][3] - viewProj[2][2], viewProj[3][3] - viewProj[3][2]); // Far

        // Normalize the planes
        for (int i = 0; i < 6; i++) {
            float length = glm::length(glm::vec3(planes[i]));
            planes[i] /= length;
        }
    }

    bool isInFrustum(const glm::vec3& position, float radius) {
        for (int i = 0; i < 6; i++) {
            // Check if the sphere is outside any of the frustum planes
            if (glm::dot(glm::vec3(planes[i]), position) + planes[i].w < -radius) {
                return false; // The sphere is outside the frustum
            }
        }
        return true; // The sphere is inside the frustum
    }
};
