static class Utils {
public:
    static glm::vec3 generateRandomPosition(float rangeMin, float rangeMax) {
        float x = rangeMin + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (rangeMax - rangeMin)));
        float y = rangeMin + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (rangeMax - rangeMin)));
        float z = rangeMin + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (rangeMax - rangeMin)));
        return glm::vec3(x, y, z);
    }

    static glm::vec3 screenToWorldRay(int mouseX, int mouseY, int width, int height, Camera camera) {
        float x = (2.0f * mouseX) / width - 1.0f;
        float y = 1.0f - (2.0f * mouseY) / height;
        glm::vec4 rayClip(x, y, -1.0f, 1.0f);

        glm::vec4 rayEye = glm::inverse(camera.proj) * rayClip;
        rayEye.z = -1.0f;
        rayEye.w = 0.0f;

        glm::vec3 rayWorld = glm::vec3(glm::inverse(camera.view) * rayEye);
        return glm::normalize(rayWorld);
    }
};
